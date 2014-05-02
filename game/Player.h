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


    //Models and similar
    D3SpriteRenderModel *m_pRenderModel;
    TimePhysicsModel    *m_pPhysicsModel;
    DraggableHud        *m_pHud;

    //Identification
    uint m_uiId;
    flag_t m_uiFlags;

    //Direction and movement information
    float m_iStrafeSpeed, m_iForwardSpeed;
    float m_fDeltaZoom, m_fDeltaPitch;  //Camera deltas
    int m_iDirection;

    //Animation
    int m_iAnimTimer, m_iAnimState;
    uint m_uiAnimFrameStart;

    //State & action information
    bool m_bFirst;
    bool m_bMouseDown;
    bool m_bCanClimb;
    bool m_bSprinting;
    PlayerState m_eState;
//    PlayerAction m_eAction;
    uint m_uiHealth;
    uint m_uiMaxHealth;

    //Spell info
    Spell *m_pCurSpell;

    //Climbing info
    uint m_uiClimbObjId;        //ID of the object player is climbing up
    uint m_uiClimbObjCmdlId;    //ID of the collision model the player is climbing up
    Point m_ptClimbShift;           //The direction in which you are climbing
    Point m_ptStartClimbPos;        //Where you were when you start climbing
};

#endif // PLAYER_H
