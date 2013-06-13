/*
 * Player.cpp
 */

#include "Player.h"
#include "ore/OrderedRenderEngine.h"
#include "tpe/TimePhysicsEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/FXSprite.h"
#include "game/spells/Spells.h"
#include "game/gameDefs.h"
#include "game/GameManager.h"
#include <stdlib.h>
#include <time.h>

using namespace std;

Player::Player(uint id, Image *img, Point pos) {
    m_uiID = id;
    m_uiFlags = 0;
    float iw = img->w / img->m_iNumFramesW,
          ih = img->h / img->m_iNumFramesH;

    Rect rcRenderArea(pos.x - iw / 2, pos.y - 3 * ih / 4, iw, ih);
    Box bxCollisionArea(pos.x - 3 * iw / 8, pos.y - ih / 4, pos.z, 3 * iw / 4, ih / 2, ih);

    //pos -= Point(draw_w / 2, draw_h / 2, 0);
    //Box bxVolume = Box(pos.x + draw_w / 8, pos.y + draw_h / 2, pos.z, 3 * draw_w / 4, draw_h / 2, draw_h);
    m_pPhysicsModel = new TimePhysicsModel(bxCollisionArea);
    m_pPhysicsModel->setListener(this);
    //m_pRenderModel = new OrderedRenderModel(img, Rect(pos.x, pos.y, draw_w, draw_h), ORE_LAYER_OBJECTS);
    m_pRenderModel = new OrderedRenderModel(img, rcRenderArea, pos.z, ORE_LAYER_OBJECTS);
    PWE::get()->addListener(this, ON_BUTTON_INPUT);
    dx = dy = 0;
    state = timer = 0;
    m_iDirection = SOUTH;
    srand(time(NULL));

    printf("Player id: %d\n", id);
}

Player::~Player() {
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

bool Player::update(uint time) {
    Point mov = Point(dx,dy,0);
    mov.normalize();
    m_pPhysicsModel->applyForce(mov);

    if(dx > 0 && dy == 0) {
        m_iDirection = EAST;
    } else if(dx < 0 && dy == 0) {
        m_iDirection = WEST;
    } else if(dx == 0 && dy > 0) {
        m_iDirection = SOUTH;
    } else if(dx == 0 && dy < 0) {
        m_iDirection = NORTH;
    }
    m_pRenderModel->setFrameW(m_iDirection);

    if(dx == 0 && dy == 0) {
        m_pRenderModel->setFrameH(0);
        timer = -1;
    } else {
        if(timer < 0) {
            timer = 20;
            state = ((state) % 4) + 1;
            m_pRenderModel->setFrameH(state);
        } else {
            --timer;
        }

        if(!(timer % 5)) {
            PartitionedWorldEngine *we = PWE::get();
            Point pos = m_pPhysicsModel->getCenter() + Point(((rand() % 5) - 2), ((rand() % 5) - 2), -m_pPhysicsModel->getCollisionVolume().h / 2);
            FXSprite *fx = new FXSprite(we->genID(), pos, FX_SMOKE, ORE_LAYER_LOW_FX);
            we->add(fx);
        }
    }

    Point ptNSP = m_pPhysicsModel->getPosition() - Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    //ptNSP = Point((int)ptNSP.x, (int)ptNSP.y, (int)ptNSP.z);
    OrderedRenderEngine::get()->moveScreenTo(ptNSP);

    return false;
}


void Player::callBack(uint cID, void *data, EventID id) {
    switch(id) {
    case ON_BUTTON_INPUT:
        handleButton((InputData*)data);
        break;
    case ON_COLLISION:
        handleCollision((HandleCollisionData*)data);
        break;
    default:
        break;
    }
}

void Player::handleButton(InputData* data) {
    if(data->getInputState(IN_NORTH)) {
        dy = -1;
    } else if(data->getInputState(IN_SOUTH)) {
        dy = 1;
    } else {
        dy = 0;
    }

    if(data->getInputState(IN_WEST)) {
        dx = -1;
    } else if(data->getInputState(IN_EAST)) {
        dx = 1;
    } else {
        dx = 0;
    }

    if(data->getInputState(IN_CAST)) {
        PartitionedWorldEngine *we = PWE::get();
        Point pos;
        switch(m_iDirection) {
        case NORTH:
            pos = m_pPhysicsModel->getPosition() + Point(0,-m_pPhysicsModel->getCollisionVolume().l,0);
            break;
        case SOUTH:
            pos = m_pPhysicsModel->getPosition() + Point(0,m_pPhysicsModel->getCollisionVolume().l,0);
            break;
        case EAST:
            pos = m_pPhysicsModel->getPosition() + Point(m_pPhysicsModel->getCollisionVolume().w,0,0);
            break;
        case WEST:
            pos = m_pPhysicsModel->getPosition() + Point(-m_pPhysicsModel->getCollisionVolume().w,0,0);
            break;
        }
        AirSpell *spell = new AirSpell(we->genID(), m_pPhysicsModel->getPosition(), pos, 5);
        we->add(spell);
    }
    
    //Begin earth-spellcasting test
    if(data->getInputState(IN_SELECT)) {
        int x = data->getInputState(MIN_MOUSE_X),
            y = data->getInputState(MIN_MOUSE_Y);
        GameObject *obj = ORE::get()->getObjAtPos(x, y);
        if(obj != NULL) {
            printf("Clicked on object %d\n", obj->getID());
        } else {
            printf("No objs found\n");
        }
    }
}

void Player::handleCollision(HandleCollisionData *data) {

}

Player *Player::read(FileManager *mgr) {
    PartitionedWorldEngine *we = PWE::get();
    fstream *fin = mgr->getFileHandle();
    //Order:
    //{image id} {Point pos}
    uint uiImgID;
    Point ptPos;
    fin->read((char*)(&uiImgID), sizeof(uint));
    fin->read((char*)(&ptPos),   sizeof(Point));

    return new Player(we->genID(), mgr->getImageRes(uiImgID), ptPos);
}

void Player::write(FileManager *mgr) {
    fstream *fout = mgr->getFileHandle();
    Point ptPos = m_pPhysicsModel->getPosition();
    uint uiImgID = m_pRenderModel->getImage()->m_uiID;
    ObjType eType = OT_PLAYER;

    fout->write((char*)(&eType),  sizeof(ObjType));
    fout->write((char*)(uiImgID), sizeof(uint));
    fout->write((char*)(&ptPos),  sizeof(float));
}

