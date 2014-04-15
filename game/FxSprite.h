#ifndef FXSPRITE_H
#define FXSPRITE_H

#include "mge/GameObject.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "game/game_defs.h"

class FxSprite : public GameObject
{
public:
    FxSprite(uint id, uint texId, int duration, const Point &ptPos, uint frameW = 0);
    virtual ~FxSprite();

    //File i/o
    //static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase) {}

    //General
    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "FxSprite"; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual int callBack(uint cID, void *data, uint uiEventId);

    void setColor(const Color &cr) { m_pRenderModel->setColor(cr); }
    const Color &getColor() const { return m_pRenderModel->getColor(); }
protected:
private:
    uint m_uiId, m_uiFlags;
    uint m_uiMaxFramesH;
    int m_iTimeToLive, m_iMaxTimeToLive;
    int m_iAnimTimer;
    NullTimePhysicsModel *m_pPhysicsModel;
    D3XZSpriteRenderModel *m_pRenderModel;
};

#endif // FXSPRITE_H
