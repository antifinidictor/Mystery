/*
 * FXSprite
 * Defines a game object that is purely for effect
 */

#ifndef FX_SPRITE
#define FX_SPRITE
#include "mge/GameObject.h"
#include "mge/defs.h"
#include "mge/Image.h"
#include "tpe/TimePhysicsModel.h"
#include "ore/OrderedRenderModel.h"
#include "game/GameDefs.h"

enum FXType {
    FX_SMOKE,
    FX_SPARKLE,
    FX_DUST,
    FX_NUM_IDS
};

class FXSprite : public GameObject {
public:
    //Constructor/Destructor
    FXSprite(uint id, Point pos, FXType type, int iLayer);
    virtual ~FXSprite();

    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType() { return OBJ_DISPLAY; }

    virtual bool update(uint time);

    //Adjustments
    void setDuration(int iTime) { m_iMaxTime = m_iTimer = iTime; }

    //models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;
    int m_iCurFrame, m_iTimer, m_iMaxTime;
    OrderedRenderModel *m_pRenderModel;
    TimePhysicsModel  *m_pPhysicsModel;
};

#endif
