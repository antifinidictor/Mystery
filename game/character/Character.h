#ifndef CHARACTER_H
#define CHARACTER_H
/*
 * A 'scriptable' character.
 * This class is just a dunce class that lets you dynamically plug in
 * different behaviors.
 *
 * Character scripting is done by giving a character a set of tasks.
 * To make a new task, extend the basic task interface.
 * A task's execute method is called until it returns true, indicating
 * that the task is complete.
 *
 * The task intercepts all events.
 */

#include "mge/GameObject.h"
#include "game/game_defs.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
class Action;

class Character : public GameObject
{
public:
    Character(uint id, uint uiImageId, Point ptPos);
    virtual ~Character();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return TYPE_GENERAL; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "Character"; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual int callBack(uint cID, void *data, uint uiEventId);

    void setFrame(int iFrame) { m_pRenderModel->setFrameH(iFrame); }
    void setDirection(int iFrame) { m_pRenderModel->setFrameW(iFrame); m_iDirection = iFrame; }
    int getDirection() { return m_iDirection; }

    //Single-update functions: Call once per update
    void moveTowards(const Point &pt, float speed = 1.f);
    void standStill();
private:
    uint m_uiId;
    uint m_uiFlags;

    D3SpriteRenderModel *m_pRenderModel;
    TimePhysicsModel   *m_pPhysicsModel;
    Action *m_pCurAction;

    int m_iAnimTimer;
    uint m_uiAnimState;
    int m_iDirection;
};

#endif // CHARACTER_H
