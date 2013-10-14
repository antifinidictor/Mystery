#ifndef PLAYER_H
#define PLAYER_H

#include "mge/GameObject.h"
#include "mge/Event.h"
#include "game/game_defs.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/TimePhysicsEngine.h"

#include "game/spells/Spell.h"

class Player : public GameObject
{
public:
    Player(uint uiId, const Point &ptPos);
    virtual ~Player();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(uint time);

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
    virtual void callBack(uint cID, void *data, uint id);

private:
    enum PlayerState {
        PLAYER_NORMAL,
        PLAYER_TALKING,
        PLAYER_CASTING,
        PLAYER_CASTING_TRANS,
        NUM_PLAYER_STATES
    };

    void updateNormal(uint time);
    void updateCasting(uint time);
    void updateCastingTrans(uint time);
    void updateSpells();

    void handleButtonNormal(InputData* data);
    void handleButtonCasting(InputData *data);
    void handleCollision(HandleCollisionData *data);

    void cleanSpells();
    void resetSpell(uint uiSpell);

    uint m_uiId, m_uiFlags;
    D3SpriteRenderModel *m_pRenderModel;
    TimePhysicsModel   *m_pPhysicsModel;

    float dx, dy;
    int m_iDirection;
    int timer, state;
    float m_fDeltaZoom, m_fDeltaPitch;  //Camera deltas
    uint m_uiAnimFrameStart;
    bool m_bFirst;
    bool m_bMouseDown;
    PlayerState m_eState;
    Spell *m_aSpells[NUM_SPELL_TYPES];
    uint m_uiCurSpell;
};

#endif // PLAYER_H
