#include "Player.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/tpe.h"
#include "GameManager.h"
#include "game/spells/SourceSinkSpell.h"
#include "game/spells/FlowSpell.h"

#define DENSITY 900.f  //1000kg/m^3 ~ density of water
#define WALK_FORCE 1.f

enum PlayerAnims {
    PANIM_STANDING = 0,
    PANIM_WALKING = 1,
    PANIM_THROWING = 5,
    PANIM_PUSHING = 8,
    NUM_PANIMS
};

Player::Player(uint uiId, const Point &ptPos) {
    Image *img = D3RE::get()->getImage(IMG_PLAYER);
    int iw = img->h / img->m_iNumFramesH;
    float w = WORLD_TILE_SIZE / 2;    //img->w / img->m_iNumFramesW,
    float h = iw / (float)TEXTURE_TILE_SIZE;    //img->h / img->m_iNumFramesH;
    m_uiId = uiId;
    m_uiFlags = 0;

    Rect rcDrawArea = Rect(-w / 2, 0, w, h);
    Box bxVolume = Box(-w / 3, 0, -h / 4, 2 * w / 3, 3 * h / 4, h / 2);
    m_pPhysicsModel = new TimePhysicsModel(ptPos, DENSITY);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxVolume));
    m_pRenderModel  = new D3SpriteRenderModel(this, IMG_PLAYER, rcDrawArea);

    dx = dy = 0;
    m_fDeltaPitch = m_fDeltaZoom = 0.f;
    state = timer = 0;
    m_iDirection = SOUTH;
    m_bFirst = true;
    m_uiAnimFrameStart = 1;
    m_pPhysicsModel->setListener(this);
    m_bMouseDown = false;

    m_eState = PLAYER_NORMAL;
    //PWE::get()->addListener(this, ON_BUTTON_INPUT);

    for(uint i = 0; i < NUM_SPELL_TYPES; ++i) {
        m_aSpells[i] = NULL;
        resetSpell(i);
    }
    m_uiCurSpell = 0;
}

Player::~Player() {
    cleanSpells();
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

    //Put state information here

    return new Player(id, Point(x,y,z));
}

void
Player::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Point ptPos = m_pPhysicsModel->getPosition();
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".pos.x", ptPos.x);
    pt.put(keyBase + ".pos.y", ptPos.y);
    pt.put(keyBase + ".pos.z", ptPos.z);

    //Read state information here

}

bool Player::update(uint time) {
    //Currently needs to occur in any state that isn't "paused"
    updateSpells();

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
    default:
        //Unhandled state
        break;
    }
    m_bFirst = false;
    return false;
}


void Player::callBack(uint cID, void *data, uint id) {
    switch(id) {
    case PWE_ON_ADDED_TO_AREA:
        PWE::get()->addListener(this, ON_BUTTON_INPUT, *((uint*)data));
        //PWE::get()->setCurrentArea(*((uint*)data));
        if(!m_bFirst) {
            GameManager::get()->callBack(getId(), data, ON_AREA_FADE_IN);
        }
        dx = dy = 0;
        m_bMouseDown = false;
        break;
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
        break;
    }
}


void
Player::updateNormal(uint time) {
    Point mov;
    if(m_bMouseDown) {
        //Move according to the mouse
        mov = D3RE::get()->getMousePos() - m_pPhysicsModel->getPosition();
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
        timer = -1;
    } else {
        if(timer < 0) {
            timer = 20;
            state = ((state + 1) % 4);
            m_pRenderModel->setFrameH(state + m_uiAnimFrameStart);
            /*
            if(state == 0 || state == 2) {
                BAE::get()->playSound(AUD_STEP);
            }
            */
        } else {
            --timer;
        }
    }

    //Collisions continuously change the animation frame: make sure it gets reset
    m_uiAnimFrameStart = PANIM_WALKING;
    Point ptPos = m_pPhysicsModel->getPosition();
    D3RE::get()->moveScreenTo(ptPos);
}

void
Player::updateCasting(uint time) {
    Point ptMouse = D3RE::get()->getMousePos();
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
    if(timer < 0) {
        timer = 20;
        state = state + 1;
        m_pRenderModel->setFrameH(state + m_uiAnimFrameStart);
        if(state > 3) {
            m_eState = PLAYER_NORMAL;
            state = 0;
        }
    } else {
        --timer;
    }
}

void
Player::updateSpells() {
    for(uint i = 0; i < NUM_SPELL_TYPES; ++i) {
        if(m_aSpells[i] != NULL) {
            if(m_aSpells[i]->getStatus() == SPELL_INVALID) {
                resetSpell(i);
            } else {
                m_aSpells[i]->update();
            }
        }
    }
}

void
Player::handleButtonNormal(InputData* data) {
    if(data->getInputState(IN_CAST)) {
        m_eState = PLAYER_CASTING;
        m_pRenderModel->setFrameH(PANIM_THROWING);
        D3RE::get()->setMouseAnim(m_uiCurSpell + 1);
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
    if(!data->getInputState(IN_CAST)) {
        m_eState = PLAYER_NORMAL;
        for(uint i = 0; i < NUM_SPELL_TYPES; ++i) {
            if(m_aSpells[i] == NULL) continue;

            if(m_aSpells[i]->getStatus() == SPELL_READY) {
                m_aSpells[i]->activate();
                m_eState = PLAYER_CASTING_TRANS;
            } else if(m_aSpells[i]->getStatus() != SPELL_ACTIVE) {
                resetSpell(i);
            }
        }

        D3RE::get()->setMouseAnim(0);
    }

    //Add points to current spells
    if(data->getInputState(IN_SELECT) && data->hasChanged(IN_SELECT) && m_aSpells[m_uiCurSpell] != NULL) {
        m_aSpells[m_uiCurSpell]->addPoint(GameManager::get()->getTopVolume(), D3RE::get()->getMousePos());
    }

    //Switch current spell
    if(data->getInputState(IN_WEST) && data->hasChanged(IN_WEST)) {
        m_uiCurSpell = (m_uiCurSpell + 1) % (NUM_SPELL_TYPES - 1);
        D3RE::get()->setMouseAnim(m_uiCurSpell + 1);
    } else if(data->getInputState(IN_EAST) && data->hasChanged(IN_EAST)) {
        //Modulus behaves strangely for negatives
        if(m_uiCurSpell == 0) {
            m_uiCurSpell = NUM_SPELL_TYPES - 1;
        } else {
            m_uiCurSpell--;
        }
        D3RE::get()->setMouseAnim(m_uiCurSpell + 1);
    }
}

void
Player::cleanSpells() {
    for(uint i = 0; i < NUM_SPELL_TYPES; ++i) {
        if(m_aSpells[i] != NULL) {
            delete m_aSpells[i];
            m_aSpells[i] = NULL;
        }
    }
    m_uiCurSpell = 0;
}

void
Player::resetSpell(uint uiSpell) {
    if(m_aSpells[uiSpell] != NULL) {
        delete m_aSpells[uiSpell];
        m_aSpells[uiSpell] = NULL;
    }
    switch(uiSpell) {
    case SPELL_TYPE_SOURCE_SINK:
        m_aSpells[SPELL_TYPE_SOURCE_SINK] = new SourceSinkSpell(1000, 0.8f);
        break;
    case SPELL_TYPE_FLOW:
        m_aSpells[SPELL_TYPE_FLOW] = new FlowSpell(1000, 0.8f);
        break;
    case SPELL_TYPE_DIVIDE:
        m_aSpells[SPELL_TYPE_DIVIDE] = NULL;
        break;
    default:
        break;
    }
}

void
Player::handleCollision(HandleCollisionData *data) {
    if(data->iDirection == m_iDirection) { //!= UP && data->iDirection != DOWN) {
        m_uiAnimFrameStart = PANIM_PUSHING;
    }
}


