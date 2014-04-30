#include "Character.h"
#include "Action.h"
#include "WanderAction.h"
#include "pwe/PartitionedWorldEngine.h"
#define DENSITY 900.f  //1000kg/m^3 ~ density of water
#define WALK_FORCE 1.0f
#define ANIM_TIMER_MAX 3

enum NpcAnim {
    NPC_STANDING,
    NPC_WALKING,
    NUM_NPC_ANIMS
};

Character::Character(uint uiId, uint uiImageId, Point ptPos) {
    m_uiId = uiId;
    m_uiFlags = 0;

    Image *img = D3RE::get()->getImage(uiImageId);
    int iw = img->h / img->m_iNumFramesH;
    float w = WORLD_TILE_SIZE / 2;    //img->w / img->m_iNumFramesW,
    float h = iw / (float)TEXTURE_TILE_SIZE;    //img->h / img->m_iNumFramesH;
    m_uiId = uiId;
    m_uiFlags = 0;

    float fBxWidth = 2 * w / 3;
    float fBxHeight = 3 * h / 4;

    Rect rcDrawArea = Rect(-w / 2, 0, w, h);
    Box bxVolume = Box(-fBxWidth / 2, 0, -fBxWidth / 2, fBxWidth, fBxHeight, fBxWidth);
    m_pPhysicsModel = new TimePhysicsModel(ptPos, DENSITY);
    m_pPhysicsModel->addCollisionModel(new BoxCollisionModel(bxVolume));
    m_pRenderModel = new D3SpriteRenderModel(m_pPhysicsModel, img->m_uiID, rcDrawArea);
    m_pRenderModel->setFrameW(SOUTH);
    m_iDirection = SOUTH;
    m_pCurAction = new WanderAction(this);//NoAction();
    m_pPhysicsModel->setListener(m_pCurAction);

    m_iAnimTimer = 0;
    m_uiAnimState = 0;
}

Character::~Character() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
    delete m_pCurAction;
}

GameObject*
Character::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = PWE::get()->reserveId(pt.get(keyBase + ".id", 0));
    uint imgId = pt.get(keyBase + ".imgId", 0);
    float x = pt.get(keyBase + ".pos.x", 0.f);
    float y = pt.get(keyBase + ".pos.y", 0.f);
    float z = pt.get(keyBase + ".pos.z", 0.f);

    //Put state information here

    return new Character(id, imgId, Point(x,y,z));
}

void
Character::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Point ptPos = m_pPhysicsModel->getPosition();
    pt.put(keyBase + ".id", getId());
    pt.put(keyBase + ".imgId", m_pRenderModel->getImageId());
    pt.put(keyBase + ".pos.x", ptPos.x);
    pt.put(keyBase + ".pos.y", ptPos.y);
    pt.put(keyBase + ".pos.z", ptPos.z);

    //Read state information here
}

bool
Character::update(float fDeltaTime) {
    m_pCurAction->update(fDeltaTime);
    return false;
}

int
Character::callBack(uint cID, void *data, uint uiEventId) {
    return EVENT_DROPPED;
}


void
Character::moveTowards(const Point &pt, float speed) {
    //Commonly used Action: Moves the character one step towards the specified destination
    Point ptPos = m_pPhysicsModel->getPosition();
    Point ptDiff = pt - ptPos;
    ptDiff.y = 0.f;
    ptDiff.normalize();
    m_pPhysicsModel->applyForce(ptDiff * WALK_FORCE * speed);

    //Face the correct direction
    float myAngle = atan2(ptDiff.z, ptDiff.x);
    float myRelativeAngle = myAngle - D3RE::get()->getLookAngle();

    m_iDirection = angle2dir(myAngle - M_PI / 2);
    m_pRenderModel->setFrameW(angle2dir(myRelativeAngle));

    //Animate walking
    if(m_iAnimTimer < 0) {
        m_iAnimTimer = ANIM_TIMER_MAX;
        m_uiAnimState = ((m_uiAnimState + 1) % 4);
        m_pRenderModel->setFrameH(m_uiAnimState + NPC_WALKING);
    } else {
        --m_iAnimTimer;
    }
}

void
Character::standStill() {
    m_pRenderModel->setFrameH(NPC_STANDING);
}
