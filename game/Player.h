#ifndef PLAYER_H
#define PLAYER_H

#include "mge/GameObject.h"
#include "mge/Event.h"
#include "game/game_defs.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/TimePhysicsEngine.h"

#include "game/spells/Spell.h"
#include "game/items/Item.h"
#include "game/gui/DraggableHud.h"
#include <vector>

class Player : public GameObject
{
public:
    Player(uint uiId, const Point &ptPos);
    virtual ~Player();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_PLAYER; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Player"; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual int callBack(uint cID, void *data, uint uiEventId);

private:
    enum PlayerState {
        PLAYER_NORMAL,
        PLAYER_TALKING,
        PLAYER_CASTING,
        PLAYER_CLIMBING_TRANS,
        NUM_PLAYER_STATES
    };

    enum PlayerAction {
        PLAYER_LOOK,
        PLAYER_SPRINT,
        PLAYER_CLIMB,
        NUM_PLAYER_ACTIONS
    };

    void updateNormal(float fDeltaTime);
    void updateCasting(float fDeltaTime);
    void updateClimbingTrans(float fDeltaTime);
    void updateSpells();
    void upateHud();

    void handleButtonNormal(InputData* data);
    void handleButtonCasting(InputData *data);
    void handleCollision(HandleCollisionData *data);

    float getObjHeight(TimePhysicsModel *pmdl, uint uiCmdlId);
    void startClimbing();
    void startCasting();

    uint m_uiId, m_uiFlags;
    D3SpriteRenderModel *m_pRenderModel;
    TimePhysicsModel   *m_pPhysicsModel;

    float m_iStrafeSpeed, m_iForwardSpeed;
    int m_iDirection;
    int m_iAnimTimer, m_iAnimState;
    float m_fDeltaZoom, m_fDeltaPitch;  //Camera deltas
    uint m_uiAnimFrameStart;
    bool m_bFirst;
    bool m_bMouseDown;
    PlayerState m_eState;
    PlayerAction m_eAction;

    //You can only cast one spell at a time
    Spell *m_pCurSpell;

    float m_fEndClimbHeight;
    float m_fStartClimbHeight;
    Point m_ptClimbShift;
    uint m_uiClimbObjCmdlId;
    uint m_uiClimbObjId;    //ID of the object player is climbing up
    bool m_bCanClimb;

    bool m_bSprinting;

    uint m_uiHealth;
    uint m_uiMaxHealth;

    DraggableHud *m_pHud;
};

#endif // PLAYER_H
