#include "Player.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/tpe.h"
#include "GameManager.h"
#include "game/spells/SourceSinkSpell.h"
#include "game/spells/FlowSpell.h"
#include "game/items/SpellItem.h"
using namespace std;

#define DENSITY 900.f  //1000kg/m^3 ~ density of water
#define WALK_FORCE 1.1f
#define SPRINT_FORCE (1.6 * WALK_FORCE)
#define SPELL_DURATION 300
#define ANIM_TIMER_MAX 3
#define SPRINT_ANIM_TIMER_MAX 1
#define WALK_ANIM_TIMER_MAX 9

DraggableHud *Player::s_pHud = NULL;

enum PlayerAnims {
    PANIM_STANDING = 0,     //1 frame
    PANIM_WALKING = 1,      //4 frames
    PANIM_THROWING = 5,     //3 frames
    PANIM_PUSHING = 8,      //4 frames
    PANIM_CLIMBING = 12,    //8 frames
    NUM_PANIMS
};

Player::Player(uint uiId, const Point &ptPos)
    :   m_uiId(uiId),
        m_uiFlags(0),

        //Movement & direction information
        m_iStrafeSpeed(0),
        m_iForwardSpeed(0),
        m_fDeltaZoom(0.f),
        m_fDeltaPitch(0.f),
        m_iDirection(SOUTH),

        //Animation information
        m_iAnimTimer(0),
        m_iAnimState(0),
        m_uiAnimFrameStart(1),

        //State & action information
        m_bFirst(true),
        m_bMouseDown(false),
        m_bCanClimb(false),
        m_bSprinting(false),
        m_eState(PLAYER_NORMAL),
        //m_eAction(PLAYER_LOOK),
        m_uiHealth(10),
        m_uiMaxHealth(20),

        //Spell info
        m_pCurSpell(NULL),

        //Climbing info
        m_uiClimbObjId(0),
        m_uiClimbObjCmdlId(0),
        m_ptClimbShift(),
        m_ptStartClimbPos()

{

    Image *img = D3RE::get()->getImage("player");
    int iw = img->h / img->m_iNumFramesH;
    float w = WORLD_TILE_SIZE / 2;    //img->w / img->m_iNumFramesW,
    float h = iw / (float)TEXTURE_TILE_SIZE;    //img->h / img->m_iNumFramesH;

    float fBxWidth = 2 * w / 3;
    float fBxHeight = 3 * h / 4;

    Rect rcDrawArea = Rect(-w / 2, 0, w, h);
    Box bxVolume = Box(-fBxWidth / 2, 0, -fBxWidth / 2, fBxWidth, fBxHeight, fBxWidth);
    m_pPhysicsModel = new TimePhysicsModel(this, ptPos, DENSITY);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxVolume));
    m_pPhysicsModel->setListener(this);

    m_pRenderModel  = new D3SpriteRenderModel(m_pPhysicsModel, img->m_uiID, rcDrawArea);


    //TODO: How can we design this better?
    GameManager::get()->registerPlayer(this);
    if(s_pHud == NULL) {
        //The DraggableHud gets deleted by the screen at the end of the game
        s_pHud = new DraggableHud(PWE::get()->genId());
    }
    s_pHud->registerPlayer(this);

    //Flags
    setFlag(GAM_CAN_LINK, true);
}

Player::~Player() {
    if(m_pCurSpell != NULL) {
        delete m_pCurSpell; //TODO: Possible cause of crash on exit (tries to access ev that does not exist)
    }

    PWE::get()->freeId(getId());
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

GameObject*
Player::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    float x = pt.get(keyBase + ".pos.x", 0.f);
    float y = pt.get(keyBase + ".pos.y", 0.f);
    float z = pt.get(keyBase + ".pos.z", 0.f);

    //Put m_iAnimState information here

    return new Player(id, Point(x,y,z));
}

void
Player::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Point ptPos = m_pPhysicsModel->getPosition();
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".pos.x", ptPos.x);
    pt.put(keyBase + ".pos.y", ptPos.y);
    pt.put(keyBase + ".pos.z", ptPos.z);

    //Read m_iAnimState information here
}

bool
Player::update(float fDeltaTime) {
    //Currently needs to occur in any m_iAnimState that isn't "paused"
    updateSpells();
    upateHud();

    switch(m_eState) {
    case PLAYER_CASTING:
        updateCasting(fDeltaTime);
        break;
    case PLAYER_NORMAL:
        updateNormal(fDeltaTime);
        break;
    case PLAYER_CLIMBING_TRANS:
        updateClimbingTrans(fDeltaTime);
        break;
    default:
        //Unhandled m_iAnimState
        break;
    }


    //Point pos = m_pPhysicsModel->getPosition();
    //printf("Player position (line %d): (%f,%f,%f)\n", __LINE__, pos.x, pos.y, pos.z);

    m_bFirst = false;
    return false;
}


int
Player::callBack(uint cID, void *data, uint uiEventId) {
    int status = EVENT_DROPPED;
    switch(uiEventId) {
    case ON_UPDATE_HUD:
        s_pHud->updateItemAnimations();
        break;
    case ON_ITEM_DROPPED: {
        Item *item = ((ItemDropEvent*)data)->item;
        item->moveBy(m_pPhysicsModel->getPosition() - item->getPhysicsModel()->getPosition());
        item->onItemDrop();
        PWE::get()->add(item);
        break;
    }
    case PWE_ON_ADDED_TO_AREA: {
        uint uiAreaId = *((uint*)data);
        PWE::get()->addListener(this, ON_BUTTON_INPUT, uiAreaId);
        //PWE::get()->setCurrentArea(*((uint*)data));
        if(!m_bFirst) {
            GameManager::get()->callBack(getId(), data, ON_AREA_FADE_IN);
        }
        m_iStrafeSpeed = m_iForwardSpeed = 0;
        m_bMouseDown = false;

        ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR);
        D3HudRenderModel *label = panel->get<D3HudRenderModel*>(MGHUD_CUR_AREA);
        label->updateText(PWE::get()->getAreaName(uiAreaId));

        //handleButtonNormal(MGE::get()->getInputState());
        break;
    }
    case PWE_ON_ERASED_FROM_AREA:
    case PWE_ON_REMOVED_FROM_AREA:
        PWE::get()->removeListener(getId(), ON_BUTTON_INPUT, *((uint*)data));
        m_pPhysicsModel->setSurface(NULL);
        //GameManager::get()->callBack(getID(), data, ON_AREA_FADE_OUT);
        break;
    case ON_BUTTON_INPUT: {
        switch(m_eState) {
        case PLAYER_CASTING:
            handleButtonCasting((InputData*)data);
            break;
        case PLAYER_NORMAL:
            handleButtonNormal((InputData*)data);
            break;
        default:
            break;
        }
        break;
      }
    case TPE_ON_COLLISION:
        handleCollision((HandleCollisionData*)data);
        break;
    default:
        status = EVENT_DROPPED;
        break;
    }
    return status;
}


void
Player::updateNormal(float fDeltaTime) {
    Point mov;
    float fSpeed = m_bSprinting ? SPRINT_FORCE : WALK_FORCE;
    float fLookAngle = D3RE::get()->getLookAngle();
    if(m_bMouseDown) {
        Point ptMouseVec = D3RE::get()->getMouseRay();
        Point ptCamPos = D3RE::get()->getCameraPosition();
        float myY = m_pPhysicsModel->getPosition().y;
        float t = (myY - ptCamPos.y) / ptMouseVec.y;
        Point ptMouse =  ptCamPos + ptMouseVec * t;

        //Move according to the mouse
        mov = ptMouse - m_pPhysicsModel->getPosition();
        mov.y = 0.f;
        m_iStrafeSpeed = mov.x;
        m_iForwardSpeed = mov.z;

        //Distance from mouse has some effect on speed
        float scale = mov.magnitude() / 1.f;
        if(scale > 1.f) scale = 1.f;
        mov.normalize();
        mov *= scale * fSpeed;

        //Player faces mouse
        float myAngle = atan2(mov.z, mov.x);

        //Player direction separate from screen-relative direction
        m_iDirection = angle2dir(myAngle - M_PI / 2);
    } else if(m_iStrafeSpeed != 0 || m_iForwardSpeed != 0) {
        //Move according to the movement keys
        mov = Point(
            m_iForwardSpeed * cos(fLookAngle) - m_iStrafeSpeed * sin(-fLookAngle),
            0,
            m_iForwardSpeed * sin(fLookAngle) - m_iStrafeSpeed * cos(-fLookAngle)
        );
        mov.normalize();
        mov *= fSpeed;

        //Player faces direction of arrow keys
        float myAngle = atan2(mov.z, mov.x);

        //Player direction separate from screen-relative direction
        m_iDirection = angle2dir(myAngle - M_PI / 2);
    }

    //Face the appropriate direction in case the camera turns
    //Less efficient when the player is moving, but it does keep them facing
    // the same direction when the camera turns
    int relativeDir = m_iDirection - angle2dir(fLookAngle + M_PI / 2);
    if(relativeDir < 0) {
        relativeDir += NUM_CARDINAL_DIRECTIONS;
    }
    m_pRenderModel->setFrameW(relativeDir);

    //Move the player
    m_pPhysicsModel->applyForce(mov);

    //Camera adjustment
    D3RE::get()->adjustCamAngle(m_fDeltaPitch);
    D3RE::get()->adjustCamDist(m_fDeltaZoom);

    if(m_iStrafeSpeed == 0 && m_iForwardSpeed == 0) {
        m_pRenderModel->setFrameH(PANIM_STANDING);
        m_iAnimTimer = -1;
    } else {
        if(m_iAnimTimer < 0) {
            m_iAnimTimer = BOUND(1, ANIM_TIMER_MAX / mov.magnitude(), WALK_ANIM_TIMER_MAX);
            m_iAnimState = ((m_iAnimState + 1) % 4);
            m_pRenderModel->setFrameH(m_iAnimState + m_uiAnimFrameStart);
#if 1
            if(m_iAnimState == 0 || m_iAnimState == 2) {
                BAE::get()->playSound(AUD_STEP);
            }
#endif
        } else {
            --m_iAnimTimer;
        }
    }
    m_bCanClimb = false;

    //Collisions continuously change the animation frame: make sure it gets reset
    m_uiAnimFrameStart = PANIM_WALKING;
    Point ptPos = m_pPhysicsModel->getPosition();
    D3RE::get()->moveScreenTo(ptPos);
}

void
Player::updateCasting(float fDeltaTime) {
    //Update animation
    if(m_uiAnimFrameStart == PANIM_THROWING) {
        if(m_iAnimTimer < 0) {
            m_iAnimTimer = ANIM_TIMER_MAX;
            m_iAnimState = m_iAnimState + 1;
            m_pRenderModel->setFrameH(m_iAnimState + m_uiAnimFrameStart);
            if(m_iAnimState > 3) {
                //Continue casting
                m_eState = PLAYER_CASTING;
                m_iAnimState = 0;
                m_uiAnimFrameStart = PANIM_STANDING;
                m_pRenderModel->setFrameH(PANIM_STANDING);
            }
        } else {
            --m_iAnimTimer;
        }
    }


    //Move the screen to halfway between the player and the mouse
    Point ptMouse;
    //if(D3RE::get()->getMouseOverObject() == NULL) {
        Point ptMouseVec = D3RE::get()->getMouseRay();
        Point ptCamPos = D3RE::get()->getCameraPosition();
        float myY = m_pPhysicsModel->getPosition().y;
        float t = (myY - ptCamPos.y) / ptMouseVec.y;
        ptMouse =  ptCamPos + ptMouseVec * t;
    //} else {
    //    ptMouse = D3RE::get()->getMousePos();
    //}

    //Calculate player look direction
    Point ptPos = m_pPhysicsModel->getPosition();
    Vec3f v3Diff = ptMouse - ptPos;
    float fLookAngle = D3RE::get()->getLookAngle();
    float myAngle = atan2(v3Diff.z, v3Diff.x);
    float myRelativeAngle = myAngle - fLookAngle;

    //Player direction separate from screen-relative direction
    m_iDirection = angle2dir(myAngle - M_PI / 2);
    m_pRenderModel->setFrameW(angle2dir(myRelativeAngle));

    Point ptScreenPos = (ptPos + ptMouse) / 2.f;
    D3RE::get()->moveScreenTo(ptScreenPos);
}


void
Player::updateClimbingTrans(float fDeltaTime) {
    //My current position: Used in the position update calculations below
    Point ptMyPos = m_pPhysicsModel->getPosition();
    setFlag(TPE_PASSABLE, true);
    //setFlag(TPE_STATIC, true);


    //Update climbing information
    GameObject *obj = PWE::get()->find(m_uiClimbObjId);
    if(obj != NULL) {
        //Position/movement information about the object I'm climbing
        TimePhysicsModel *tpmObj = (TimePhysicsModel*)obj->getPhysicsModel();
        Box bxObjBounds = tpmObj->getCollisionModel(m_uiClimbObjCmdlId)->getBounds();
        bxObjBounds += tpmObj->getPosition();
        float fObjHeight = getObjHeight(tpmObj, m_uiClimbObjCmdlId);

        //Update start climb position: Changes due to object movement
        m_ptStartClimbPos += tpmObj->getLastVelocity();

        //Update destination climb position: Changes if object height is changing and/or object is moving
        float tx, tz, t;
        tx = (m_ptClimbShift.x < 0) ? ((bxObjBounds.x + bxObjBounds.w) - ptMyPos.x) / m_ptClimbShift.x: //west
                                       (bxObjBounds.x - ptMyPos.x) / m_ptClimbShift.x;                  //east
        tz = (m_ptClimbShift.z < 0) ? ((bxObjBounds.z + bxObjBounds.l) - ptMyPos.z) / m_ptClimbShift.z: //south
                                       (bxObjBounds.z - ptMyPos.z) / m_ptClimbShift.z;                  //north
        t = (fabs(tx) < fabs(tz)) ? tx : tz;
        Point ptEndClimbPos = m_ptStartClimbPos + m_ptClimbShift * t;
        ptEndClimbPos.y = fObjHeight;

        //Update intermediate position: Relative to start and end climb positions
        Point ptIntermediateClimbPos;
        ptIntermediateClimbPos.x = m_ptStartClimbPos.x;
        ptIntermediateClimbPos.y = ptEndClimbPos.y;
        ptIntermediateClimbPos.z = m_ptStartClimbPos.z;


        //Calculate new position information
        //Interpolation biases for each of the three points
        float fCurTime = (m_iAnimState * ANIM_TIMER_MAX + ANIM_TIMER_MAX - m_iAnimTimer) / (ANIM_TIMER_MAX * 8.f);
        float fIntermediateBias = 0.5f - fabs(0.5f - fCurTime);
        float fEndBias          = fCurTime         - fIntermediateBias / 2.f;
        float fStartBias        = (1.f - fCurTime) - fIntermediateBias / 2.f;

        //Calculate the new position
        //It's more efficient to calculate each dimension separately
        Point ptNewPos;
        ptNewPos.x = m_ptStartClimbPos.x      * fStartBias +
                     ptIntermediateClimbPos.x * fIntermediateBias +
                     ptEndClimbPos.x          * fEndBias;
        ptNewPos.y = m_ptStartClimbPos.y      * fStartBias +
                     ptIntermediateClimbPos.y * fIntermediateBias +
                     ptEndClimbPos.y          * fEndBias;
        ptNewPos.z = m_ptStartClimbPos.z      * fStartBias +
                     ptIntermediateClimbPos.z * fIntermediateBias +
                     ptEndClimbPos.z          * fEndBias;

        //Move myself and the screen to the new position
        moveBy(ptNewPos - ptMyPos);
        D3RE::get()->moveScreenTo(ptNewPos);
    }

    //Finish climbing?
    if(m_iAnimTimer < 0 || obj == NULL) {
        m_iAnimTimer = ANIM_TIMER_MAX;
        m_iAnimState = m_iAnimState + 1;
        m_pRenderModel->setFrameH(m_iAnimState + m_uiAnimFrameStart);

        if(m_iAnimState > 8) {
            m_eState = PLAYER_NORMAL;
            m_bCanClimb = false;
            if(obj == NULL) {
                m_pPhysicsModel->setSurface(NULL);
            } else {
                m_pPhysicsModel->setSurface((AbstractTimePhysicsModel*)obj->getPhysicsModel());
            }
            m_iAnimState = 0;
            m_uiAnimFrameStart = PANIM_STANDING;
            setFlag(TPE_FLOATING, false);
            setFlag(TPE_STATIC, false);
            setFlag(TPE_PASSABLE, false);

            handleButtonNormal(MGE::get()->getInputState());
        }
    } else {
        --m_iAnimTimer;
    }
}

void
Player::updateSpells() {
    if(m_pCurSpell == NULL) {
        return;
    }

    //Update the spell
    m_pCurSpell->update();

    //If the spell is finished, delete it
    if(m_pCurSpell->getStatus() == SPELL_INVALID) {
        delete m_pCurSpell;
        m_pCurSpell = NULL;
    }
}


void
Player::upateHud() {
    ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR);
    D3HudRenderModel *label = panel->get<D3HudRenderModel*>(MGHUD_CUR_ACTION);
    SpellItem *item = s_pHud->getCurSpell();
    if(m_bCanClimb) {
        label->updateText("Climb");
    } else if(m_iStrafeSpeed != 0.f || m_iForwardSpeed != 0.f || m_bMouseDown) {
        label->updateText("Sprint");
    } else if(item != NULL && m_pCurSpell == NULL) {
        label->updateText("Cast");
    } else if(m_pCurSpell != NULL && m_pCurSpell->getStatus() != SPELL_READY && m_pCurSpell->getStatus() != SPELL_ACTIVE) {
        label->updateText("Cancel");
    } else if(m_pCurSpell != NULL && m_pCurSpell->getStatus() == SPELL_READY) {
        label->updateText("Cast");
    } else if(m_pCurSpell != NULL) {
        label->updateText("Dispell");
    } else {
        label->updateText("Look");
    }
}

void
Player::handleButtonNormal(InputData* data) {
    m_bMouseDown = data->getInputState(IN_SELECT);
    if(!m_bMouseDown) {
        m_iStrafeSpeed = 0;
        m_iForwardSpeed = 0;
    }

    if(!data->getInputState(IN_TOGGLE_DEBUG_MODE) && data->hasChanged(IN_TOGGLE_DEBUG_MODE)) {
        //Debug stuff
        D3RE::get()->setDrawCollisions(!D3RE::get()->getDrawCollisions());
    }

    if(data->getInputState(IN_CTRL)) {
        if(data->getInputState(IN_NORTH)) {
            m_fDeltaPitch = M_PI / 100;
        } else if(data->getInputState(IN_SOUTH)) {
            m_fDeltaPitch = -M_PI / 100;
        } else {
            m_fDeltaPitch = 0.f;
        }
        m_iForwardSpeed = 0;

        m_fDeltaZoom = 0.f;
    } else if(data->getInputState(IN_SHIFT)) {
        if(data->getInputState(IN_NORTH)) {
            m_fDeltaZoom = -0.1f;
        } else if(data->getInputState(IN_SOUTH)) {
            m_fDeltaZoom = 0.1f;
        } else {
            m_fDeltaZoom = 0.f;
        }
        m_iForwardSpeed = 0;

        m_fDeltaPitch = 0.f;
    } else {
        if(data->getInputState(IN_NORTH)) {
            m_iForwardSpeed = -1;
        } else if(data->getInputState(IN_SOUTH)) {
            m_iForwardSpeed = 1;
        } else {
            m_iForwardSpeed = 0;
        }
        m_fDeltaZoom = m_fDeltaPitch = 0.f;
    }

    if(data->getInputState(IN_WEST)) {
        m_iStrafeSpeed = -1;
    } else if(data->getInputState(IN_EAST)) {
        m_iStrafeSpeed = 1;
    } else {
        m_iStrafeSpeed = 0;
    }

    //Start casting if we don't already have a spell running
    if(data->getInputState(IN_CAST) && data->hasChanged(IN_CAST)) {
        //Determine possible courses of action
        if(m_bCanClimb) {
            startClimbing();
        } else if(m_iStrafeSpeed != 0.f || m_iForwardSpeed != 0.f || m_bMouseDown) {
            m_bSprinting = true;
        } else {
            startCasting();
        }
    } else if(!data->getInputState(IN_CAST) && data->hasChanged(IN_CAST)) {
        m_bSprinting = false;
    }

    float fCurLookAngle = D3RE::get()->getLookAngle();
    float fCurDesiredLookAngle = D3RE::get()->getDesiredLookAngle();
    if(data->getInputState(IN_ROTATE_LEFT) && data->hasChanged(IN_ROTATE_LEFT) && fCurDesiredLookAngle <= fCurLookAngle) {
        D3RE::get()->adjustLookAngle(M_PI / 2);
    } else  if(data->getInputState(IN_ROTATE_RIGHT) && data->hasChanged(IN_ROTATE_RIGHT) && fCurDesiredLookAngle >= fCurLookAngle) {
        D3RE::get()->adjustLookAngle(-M_PI / 2);
    }
}

void
Player::handleButtonCasting(InputData* data) {
    //If the spell became invalid, try to create a new one
    if(m_pCurSpell == NULL) {
        SpellItem *item = s_pHud->getCurSpell();
        if(item != NULL) {
            m_pCurSpell = item->createSpell(SPELL_DURATION, 0.8f);
        }
    }

    //Actually cast the spell, if it is valid
    if(!data->getInputState(IN_CAST) && data->hasChanged(IN_CAST)) {
        m_eState = PLAYER_NORMAL;
        m_uiAnimFrameStart = PANIM_WALKING;
        if(m_pCurSpell != NULL) {
            if(m_pCurSpell->getStatus() == SPELL_READY) {
                m_pCurSpell->activate();
            } else {
                m_pCurSpell->deactivate();
            }
        }

        //Stop playing spell sound if it is playing
        BAE::get()->playSound(AUD_NONE, 0, AUD_CHAN_PLAYER_SPELL_BACKGROUND);
        return;
    }

    //Add points to current spells
    if(data->getInputState(IN_SELECT) && data->hasChanged(IN_SELECT)
       && m_pCurSpell != NULL && GameManager::get()->getTopVolume() != NULL) {
        m_pCurSpell->addPoint(GameManager::get()->getTopVolume(), D3RE::get()->getMousePos(), true);
        m_uiAnimFrameStart = PANIM_THROWING;
    }

    if(data->getInputState(IN_RCLICK) && data->hasChanged(IN_RCLICK)
       && m_pCurSpell != NULL && GameManager::get()->getTopVolume() != NULL) {
        m_pCurSpell->addPoint(GameManager::get()->getTopVolume(), D3RE::get()->getMousePos(), false);
        m_uiAnimFrameStart = PANIM_THROWING;
    }

    float fCurLookAngle = D3RE::get()->getLookAngle();
    float fCurDesiredLookAngle = D3RE::get()->getDesiredLookAngle();
    if(data->getInputState(IN_ROTATE_LEFT) && data->hasChanged(IN_ROTATE_LEFT) && fCurDesiredLookAngle <= fCurLookAngle) {
        D3RE::get()->adjustLookAngle(M_PI / 2);
    } else  if(data->getInputState(IN_ROTATE_RIGHT) && data->hasChanged(IN_ROTATE_RIGHT) && fCurDesiredLookAngle >= fCurLookAngle) {
        D3RE::get()->adjustLookAngle(-M_PI / 2);
    }
}

float
Player::getObjHeight(TimePhysicsModel *pmdl, uint uiCmdlId) {
    CollisionModel *cmdl = pmdl->getCollisionModel(uiCmdlId);
    float fHeightOfObj = pmdl->getPosition().y;
    switch(cmdl->getType()) {
    case CM_BOX: {
        BoxCollisionModel *bxmdl = (BoxCollisionModel*)cmdl;
        fHeightOfObj += bxmdl->m_bxBounds.y + bxmdl->m_bxBounds.h;
        break;
    }
    case CM_Y_HEIGHTMAP: {
        PixelMapCollisionModel *pxmdl = (PixelMapCollisionModel*)cmdl;
        fHeightOfObj += pxmdl->getHeightAtPoint(m_pPhysicsModel->getPosition() - pmdl->getPosition());
        break;
    }
    default:
        break;
    }
    return fHeightOfObj;
}

void
Player::handleCollision(HandleCollisionData *data) {
/*
    uint uiMyLeft = (m_iDirection + 1) % NUM_CARDINAL_DIRECTIONS;
    uint uiMyRight = (m_iDirection + NUM_CARDINAL_DIRECTIONS - 1) % NUM_CARDINAL_DIRECTIONS;
    uint uiMyCollisionDirs = BIT(m_iDirection) | BIT(uiMyLeft) | BIT(uiMyRight);
    uint uiEquivCollisionDirs = data->iDirection & uiMyCollisionDirs;
*/
    //GameObject *me = PWE::get()->find(getId());
    //printf("Did it find me? %s (Is it me? %s)\n", me == NULL ? "no" : "yes", me == this ? "yes" : "no");

    if(data->obj->getType() == TYPE_ITEM && data->obj->getFlag(GAM_CAN_PICK_UP)) {
        //Pick up the item
        Item *item = (Item*)data->obj;
        if(s_pHud->addItem(item)) {    //Item successfully added to the index?
            //Remove the item from the world
            PWE::get()->remove(item->getId());
            item->onItemPickup();

            //Play the appropriate sound
            BAE::get()->playSound(AUD_PICKUP);
        }
    } else if(data->iDirection & BIT(m_iDirection) &&
              m_eState == PLAYER_NORMAL &&
              !data->obj->getFlag(TPE_PASSABLE)) {

        //Climb onto the object

        //Extract the height we have to climb: Get the time physics model
        TimePhysicsModel *pmdl = dynamic_cast<TimePhysicsModel*>(data->obj->getPhysicsModel());
        if(pmdl == NULL) {
            return;
        }

        //Get the height according to the collision model type
        float fObjHeight = getObjHeight(pmdl, data->uiCollisionModel);

        Box mbx = m_pPhysicsModel->getCollisionVolume();

        //float shiftMag = data->ptShift.magnitude();
        //float moveMag = m_pPhysicsModel->getLastVelocity().magnitude();

        //Object is low enough to step onto
        if((fObjHeight) - (mbx.y) < 0.25) {   //Too low to climb over, so step over
            //Step up
            moveBy(Point(-data->ptShift.x, (fObjHeight) - (mbx.y), -data->ptShift.z));
            Point ptPos = m_pPhysicsModel->getPosition();
            D3RE::get()->moveScreenTo(ptPos);
            m_pPhysicsModel->setSurface(pmdl);
        } else {
            //We might be able to climb, if we didn't step up.
            if(fObjHeight < mbx.y + mbx.h) {
                //Prep for climbing
                m_ptClimbShift = Point(-data->ptShift.x, 0.f, -data->ptShift.z);
                m_uiClimbObjCmdlId = data->uiCollisionModel;
                m_uiClimbObjId = data->obj->getId();
                m_bCanClimb = true;

                //Position/velocity information needed from the other object
                Point ptMyPos = m_pPhysicsModel->getPosition();
                Box bxObjBounds = pmdl->getCollisionModel(data->uiCollisionModel)->getBounds();
                bxObjBounds += pmdl->getPosition();

                //Start climb position = my current position
                m_ptStartClimbPos = ptMyPos;
            }

            m_uiAnimFrameStart = PANIM_PUSHING;
        }
    }
}

void
Player::startClimbing() {
    //Set the state to prepare for climbing
    // Assumes all climbing prep has been done and the player can climb

    //Play appropriate sound
    BAE::get()->playSound(AUD_LIFT);

    //Climbing animation
    m_eState = PLAYER_CLIMBING_TRANS;
    m_uiAnimFrameStart = PANIM_CLIMBING;
    m_iAnimState = 0;

    //Physics bludgeoning
    setFlag(TPE_FLOATING, true);
    //setFlag(TPE_PASSABLE, true);
    //setFlag(TPE_STATIC, true);
    m_bSprinting = false;
}

void
Player::startCasting() {
    if(m_pCurSpell == NULL) {
        m_eState = PLAYER_CASTING;
        m_pRenderModel->setFrameH(PANIM_STANDING);

        SpellItem *item = s_pHud->getCurSpell();
        if(item != NULL) {
            m_pCurSpell = item->createSpell(SPELL_DURATION, 0.8f);

            //Play appropriate sound
            BAE::get()->playSound(AUD_CASTING, -1, AUD_CHAN_PLAYER_SPELL_BACKGROUND);
        }
    } else {
        m_pCurSpell->deactivate();
    }
    m_bSprinting = false;
}
