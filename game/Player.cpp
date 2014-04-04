#include "Player.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/tpe.h"
#include "GameManager.h"
#include "game/spells/SourceSinkSpell.h"
#include "game/spells/FlowSpell.h"
#include "game/items/Inventory.h"
#include "game/items/SpellItem.h"
using namespace std;

#define DENSITY 900.f  //1000kg/m^3 ~ density of water
#define WALK_FORCE 0.5f
#define SPELL_DURATION 300
#define ANIM_TIMER_MAX 3

enum PlayerAnims {
    PANIM_STANDING = 0,     //1 frame
    PANIM_WALKING = 1,      //4 frames
    PANIM_THROWING = 5,     //3 frames
    PANIM_PUSHING = 8,      //4 frames
    PANIM_CLIMBING = 12,    //8 frames
    NUM_PANIMS
};

Player::Player(uint uiId, const Point &ptPos)
    :   m_hud(PWE::get()->genId())
{
    Image *img = D3RE::get()->getImage("player");
    int iw = img->h / img->m_iNumFramesH;
    float w = WORLD_TILE_SIZE / 2;    //img->w / img->m_iNumFramesW,
    float h = iw / (float)TEXTURE_TILE_SIZE;    //img->h / img->m_iNumFramesH;
    m_uiId = uiId;
    m_uiFlags = 0;

    Rect rcDrawArea = Rect(-w / 2, 0, w, h);
    Box bxVolume = Box(-w / 3, 0, -w / 8, 2 * w / 3, 3 * h / 4, w / 4);
    m_pPhysicsModel = new TimePhysicsModel(ptPos, DENSITY);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxVolume));
    m_pRenderModel  = new D3SpriteRenderModel(this, img->m_uiID, rcDrawArea);

    dx = dy = 0;
    m_fDeltaPitch = m_fDeltaZoom = 0.f;
    m_iAnimState = m_iAnimTimer = 0;
    m_iDirection = SOUTH;
    m_bFirst = true;
    m_uiAnimFrameStart = 1;
    m_pPhysicsModel->setListener(this);
    m_bMouseDown = false;

    m_eState = PLAYER_NORMAL;
    //PWE::get()->addListener(this, ON_BUTTON_INPUT);

    m_pCurSpell = NULL;

    m_uiHealth = 10;
    m_uiMaxHealth = 20;
    setFlag(GAM_CAN_LINK, true);

    //TODO: How can we design this better?
    GameManager::get()->registerPlayer(this);
    m_hud.registerPlayer(this);
    m_inv.setInventoryDisplay(&m_hud);
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

bool Player::update(uint time) {
    //Currently needs to occur in any m_iAnimState that isn't "paused"
    updateSpells();
    upateHud();

    switch(m_eState) {
    case PLAYER_CASTING:
        updateCasting(time);
        break;
    case PLAYER_NORMAL:
        updateNormal(time);
        break;
    case PLAYER_CASTING_TRANS:
        updateCastingTrans(time);
        break;
    case PLAYER_CLIMBING_TRANS:
        updateClimbingTrans(time);
        break;
    default:
        //Unhandled m_iAnimState
        break;
    }

    m_bFirst = false;
    return false;
}


int
Player::callBack(uint cID, void *data, uint uiEventId) {
    int status = EVENT_DROPPED;
    switch(uiEventId) {
    case ON_UPDATE_HUD:
        m_hud.updateItemAnimations();
        break;
    case ON_ITEM_DROPPED: {
printf(__FILE__" %d\n",__LINE__);
        ItemDropEvent *event = (ItemDropEvent*)data;
printf(__FILE__" %d\n",__LINE__);
        //An EVENT_ITEM_CANNOT_DROP flag means that we successfully reacted to the
        //item/spell/element, but its position on the hud should not be changed
        status = EVENT_ITEM_CANNOT_DROP;
printf(__FILE__" %d\n",__LINE__);
        if(event->itemId < ITEM_NUM_ELEMENTS) {
printf(__FILE__" %d\n",__LINE__);
            //Elements cannot be moved, can only be made current
            m_inv.setCurElement(event->itemOldIndex);
        } else if(event->itemId < ITEM_NUM_SPELLS) {
printf(__FILE__" %d\n",__LINE__);
            //Spells cannot be moved, can only be made current
            m_inv.setCurSpell(event->itemOldIndex);
        } else if(event->itemNewIndex == CUR_GENERIC_ITEM_INDEX) {
printf(__FILE__" %d\n",__LINE__);
            //Generic item should be made current
            m_inv.setCurItem(event->itemOldIndex);
        } else if(event->itemNewIndex == DROP_GENERIC_ITEM_INDEX) {
printf(__FILE__" %d\n",__LINE__);
            //Generic item should be dropped on the ground
        } else {
printf(__FILE__" %d\n",__LINE__);
            //Generic item should be moved
            m_inv.moveItem(event->itemOldIndex, event->itemNewIndex);
            status = EVENT_ITEM_CAN_DROP;
        }
printf(__FILE__" %d\n",__LINE__);
        break;
    }
    case PWE_ON_ADDED_TO_AREA: {
        uint uiAreaId = *((uint*)data);
        PWE::get()->addListener(this, ON_BUTTON_INPUT, uiAreaId);
        //PWE::get()->setCurrentArea(*((uint*)data));
        if(!m_bFirst) {
            GameManager::get()->callBack(getId(), data, ON_AREA_FADE_IN);
        }
        dx = dy = 0;
        m_bMouseDown = false;

        ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_TOPBAR);
        D3HudRenderModel *label = panel->get<D3HudRenderModel*>(MGHUD_CUR_AREA);
        label->updateText(PWE::get()->getAreaName(uiAreaId));

        //handleButtonNormal(MGE::get()->getInputState());
        break;
    }
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
Player::updateNormal(uint time) {
    Point mov;
    if(m_bMouseDown) {
        Point ptMouseVec = D3RE::get()->getMouseRay();
        Point ptCamPos = D3RE::get()->getCameraPosition();
        float myY = m_pPhysicsModel->getPosition().y;
        float t = (myY - ptCamPos.y) / ptMouseVec.y;
        Point ptMouse =  ptCamPos + ptMouseVec * t;

        //Move according to the mouse
        mov = ptMouse - m_pPhysicsModel->getPosition();
        mov.y = 0.f;
        dx = mov.x;
        dy = mov.z;

        //Distance from mouse has some effect on speed
        float scale = mov.magnitude() / 1.f;
        if(scale > 1.f) scale = 1.f;
        mov.normalize();
        mov *= scale;

        //Player faces mouse
        float theta = atan2(dx, dy);
        if(theta > 3 * M_PI / 4.f || theta < -3 * M_PI / 4.f) {
            m_iDirection = NORTH;
        } else if(theta > M_PI / 4.f && theta < 3 * M_PI / 4.f) {
            m_iDirection = EAST;
        } else if(theta > -M_PI / 4.f && theta < M_PI / 4.f) {
            m_iDirection = SOUTH;
        } else {
            m_iDirection = WEST;
        }
    } else {
        //Move according to the movement keys
        mov = Point(dx, 0 ,dy);
        mov.normalize();

        //Player faces direction of arrow keys
        if(dx > 0 && dy == 0) {
            m_iDirection = EAST;
        } else if(dx < 0 && dy == 0) {
            m_iDirection = WEST;
        } else if(dx == 0 && dy > 0) {
            m_iDirection = SOUTH;
        } else if(dx == 0 && dy < 0) {
            m_iDirection = NORTH;
        }
    }
    m_pPhysicsModel->applyForce(mov * WALK_FORCE);

    //Camera adjustment
    D3RE::get()->adjustCamAngle(m_fDeltaPitch);
    D3RE::get()->adjustCamDist(m_fDeltaZoom);

    m_pRenderModel->setFrameW(m_iDirection);
    if(dx == 0 && dy == 0) {
        m_pRenderModel->setFrameH(PANIM_STANDING);
        m_iAnimTimer = -1;
    } else {
        if(m_iAnimTimer < 0) {
            m_iAnimTimer = ANIM_TIMER_MAX;
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
Player::updateCasting(uint time) {
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

    Point ptPos = m_pPhysicsModel->getPosition();
    Vec3f v3Diff = ptMouse - ptPos;
    float theta = atan2(v3Diff.x, v3Diff.z);
    if(theta > 3 * M_PI / 4.f || theta < -3 * M_PI / 4.f) {
        m_iDirection = NORTH;
    } else if(theta > M_PI / 4.f && theta < 3 * M_PI / 4.f) {
        m_iDirection = EAST;
    } else if(theta > -M_PI / 4.f && theta < M_PI / 4.f) {
        m_iDirection = SOUTH;
    } else {
        m_iDirection = WEST;
    }

    Point ptScreenPos = (ptPos + ptMouse) / 2.f;
    D3RE::get()->moveScreenTo(ptScreenPos);

    m_pRenderModel->setFrameW(m_iDirection);
}

void
Player::updateCastingTrans(uint time) {
    if(m_iAnimTimer < 0) {
        m_iAnimTimer = ANIM_TIMER_MAX;
        m_iAnimState = m_iAnimState + 1;
        m_pRenderModel->setFrameH(m_iAnimState + m_uiAnimFrameStart);
        if(m_iAnimState > 3) {
            //Stop casting
            m_eState = PLAYER_NORMAL;
            m_iAnimState = 0;
            m_uiAnimFrameStart = PANIM_STANDING;
        }
    } else {
        --m_iAnimTimer;
    }
}

void
Player::updateClimbingTrans(uint time) {
    Point ptObjShift = Point();
    GameObject *obj = PWE::get()->find(m_uiClimbObjId);
    if(obj != NULL) {
        ptObjShift = obj->getPhysicsModel()->getLastVelocity();

        //Update the end climb height
        m_fEndClimbHeight = getObjHeight((TimePhysicsModel*)obj->getPhysicsModel(), m_uiClimbObjCmdlId);
    }
    if(m_iAnimTimer < 0) {
        m_iAnimTimer = ANIM_TIMER_MAX;
        m_iAnimState = m_iAnimState + 1;
        m_pRenderModel->setFrameH(m_iAnimState + m_uiAnimFrameStart);

        if(m_iAnimState > 8) {
            m_eState = PLAYER_NORMAL;
            if(obj == NULL) {
                m_pPhysicsModel->setSurface(NULL);
            } else {
                m_pPhysicsModel->setSurface(obj->getPhysicsModel());
            }
            m_iAnimState = 0;
            m_uiAnimFrameStart = PANIM_STANDING;
            setFlag(TPE_FLOATING, false);
            setFlag(TPE_STATIC, false);
            #define CLIMB_SHIFT 0.1F
            moveBy(m_ptClimbShift);
            handleButtonNormal(MGE::get()->getInputState());

            //Move the screen to the player position
            Point ptPos = m_pPhysicsModel->getPosition();
            D3RE::get()->moveScreenTo(ptPos);
            return;
        }
    } else {
        --m_iAnimTimer;
    }

    //Determine the current climb height via interpolation
    float fCurTime = (m_iAnimState * ANIM_TIMER_MAX + ANIM_TIMER_MAX - m_iAnimTimer) / (ANIM_TIMER_MAX * 8.f);
    float fClimbShift = fCurTime         * m_fEndClimbHeight +
                        (1.f - fCurTime) * m_fStartClimbHeight -
                        m_pPhysicsModel->getPosition().y;   //This is moveBy, not moveTo

    m_pPhysicsModel->setPhysicsChanged(true);
    moveBy(Point(ptObjShift.x, fClimbShift + ptObjShift.y, ptObjShift.z));

    //Move the screen to the player position
    Point ptPos = m_pPhysicsModel->getPosition();
    D3RE::get()->moveScreenTo(ptPos);
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
    SpellItem *item = m_inv.getCurSpell();
    if(m_bCanClimb) {
        label->updateText("Climb");
    } else if(item != NULL && m_pCurSpell == NULL) {
        label->updateText("Cast");
    } else if(m_pCurSpell != NULL && m_pCurSpell->getStatus() != SPELL_READY && m_pCurSpell->getStatus() != SPELL_ACTIVE) {
        label->updateText("Cancel");
    } else if(m_pCurSpell != NULL && m_pCurSpell->getStatus() == SPELL_READY) {
        label->updateText("Cast");
    } else if(m_pCurSpell != NULL) {
        label->updateText("Dispell");
    } else {
        label->updateText("");
    }
}

void
Player::handleButtonNormal(InputData* data) {
    //Start casting if we don't already have a spell running
    if(data->getInputState(IN_CAST) && data->hasChanged(IN_CAST)) {
        //Determine possible courses of action
        if(m_bCanClimb) {
            startClimbing();
        } else {
            startCasting(); //Tries to start casting, cancels existing spell if any
        }
    }
    m_bMouseDown = data->getInputState(IN_SELECT);
    if(!m_bMouseDown) {
        dx = 0;
        dy = 0;
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
        dy = 0;

        m_fDeltaZoom = 0.f;
    } else if(data->getInputState(IN_SHIFT)) {
        if(data->getInputState(IN_NORTH)) {
            m_fDeltaZoom = -0.1f;
        } else if(data->getInputState(IN_SOUTH)) {
            m_fDeltaZoom = 0.1f;
        } else {
            m_fDeltaZoom = 0.f;
        }
        dy = 0;

        m_fDeltaPitch = 0.f;
    } else {
        if(data->getInputState(IN_NORTH)) {
            dy = -1;
        } else if(data->getInputState(IN_SOUTH)) {
            dy = 1;
        } else {
            dy = 0;
        }
        m_fDeltaZoom = m_fDeltaPitch = 0.f;
    }

    if(data->getInputState(IN_WEST)) {
        dx = -1;
    } else if(data->getInputState(IN_EAST)) {
        dx = 1;
    } else {
        dx = 0;
    }

}

void
Player::handleButtonCasting(InputData* data) {
    //Actually cast the spell, if it is valid
    if(!data->getInputState(IN_CAST) && data->hasChanged(IN_CAST)) {
        m_eState = PLAYER_NORMAL;
        m_uiAnimFrameStart = PANIM_WALKING;
        if(m_pCurSpell != NULL) {
            if(m_pCurSpell->getStatus() == SPELL_READY) {
                m_pCurSpell->activate();
                m_eState = PLAYER_CASTING_TRANS;
                m_uiAnimFrameStart = PANIM_THROWING;
            } else {
                m_pCurSpell->deactivate();
            }
        }

        //Stop playing spell sound if it is playing
        BAE::get()->playSound(AUD_NONE, 0, AUD_CHAN_PLAYER_SPELL_BACKGROUND);
    }

    //Add points to current spells
    if(data->getInputState(IN_SELECT) && data->hasChanged(IN_SELECT)
       && m_pCurSpell != NULL && GameManager::get()->getTopVolume() != NULL) {
        m_pCurSpell->addPoint(GameManager::get()->getTopVolume(), D3RE::get()->getMousePos(), true);
    }

    if(data->getInputState(IN_RCLICK) && data->hasChanged(IN_RCLICK)
       && m_pCurSpell != NULL && GameManager::get()->getTopVolume() != NULL) {
        m_pCurSpell->addPoint(GameManager::get()->getTopVolume(), D3RE::get()->getMousePos(), false);
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
    if(data->obj->getType() == TYPE_ITEM) {
        //Pick up the item
        Item *item = (Item*)data->obj;
        PWE::get()->remove(item->getId());
        //addHudInventoryItem(item);
        //GameManager::get()->addToInventory(item, true);
        m_inv.add(item);
        BAE::get()->playSound(AUD_PICKUP);
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
        float fHeightOfObj = getObjHeight(pmdl, data->uiCollisionModel);

        Box mbx = m_pPhysicsModel->getCollisionVolume();

        //float shiftMag = data->ptShift.magnitude();
        //float moveMag = m_pPhysicsModel->getLastVelocity().magnitude();

        //Object is low enough to step onto
        if((fHeightOfObj) - (mbx.y) < 0.25) {   //Too low to climb over, so step over
            //Step up
            moveBy(Point(-data->ptShift.x, (fHeightOfObj) - (mbx.y), -data->ptShift.z));
            Point ptPos = m_pPhysicsModel->getPosition();
            D3RE::get()->moveScreenTo(ptPos);
            m_pPhysicsModel->setSurface(pmdl);
        } else {
            //We might be able to climb, if we didn't step up.
            if(fHeightOfObj < mbx.y + mbx.h) {
                //Prep for climbing
                m_fEndClimbHeight = fHeightOfObj;
                m_fStartClimbHeight = mbx.y;
                m_uiClimbObjCmdlId = data->uiCollisionModel;
                m_uiClimbObjId = data->obj->getId();
                m_ptClimbShift = Point(-data->ptShift.x, 0.f, -data->ptShift.z);
                m_bCanClimb = true;
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
    //setFlag(TPE_STATIC, true);
}

void
Player::startCasting() {
    if(m_pCurSpell == NULL) {
        SpellItem *item = m_inv.getCurSpell();
        if(item != NULL) {
            m_eState = PLAYER_CASTING;
            m_pRenderModel->setFrameH(PANIM_THROWING);
            m_pCurSpell = item->createSpell(SPELL_DURATION, 0.8f);

            //Play appropriate sound
            BAE::get()->playSound(AUD_CASTING, -1, AUD_CHAN_PLAYER_SPELL_BACKGROUND);
        }
    } else {
        m_pCurSpell->deactivate();
    }
}
