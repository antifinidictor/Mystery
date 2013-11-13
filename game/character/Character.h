#ifndef CHARACTER_H
#define CHARACTER_H
/*
 * A 'scriptable' character.
 * This class is just a dunce class that lets you dynamically plug in
 * different behaviors.
 */

#include "mge/GameObject.h"
#include "game/game_defs.h"

class Character : public GameObject
{
public:
    Character(uint id);
    virtual ~Character();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(uint time);

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
    virtual void callBack(uint cID, void *data, uint uiEventId);
private:
    uint m_uiId;
    uint m_uiFlags;
};

#endif // CHARACTER_H
