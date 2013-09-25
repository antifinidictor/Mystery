#ifndef EDITOR_OBJECT_H
#define EDITOR_OBJECT_H

#include "mge/GameObject.h"
#include "mge/Event.h"

#include "editor/SelectionRenderModel.h"
#include "editor/editor_defs.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/tpe.h"

class EditorObject : public GameObject, public Listener
{
public:
    EditorObject(uint uiId, uint uiAreaId, const Point &ptPos);
    virtual ~EditorObject();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(uint time);

    virtual uint getID()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return ED_TYPE_EDITOR_OBJECT; }
    virtual const std::string getClass()        { return "EditorObject"; }
    
    virtual void moveBy(Point ptShift);

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual void callBack(uint cID, void *data, uint id);

private:
    void enterTextHandleKey(InputData *data);
    void normalStateHandleKey(InputData *data);
    
    Point toTile(const Point &pt);

    uint m_uiId, m_uiFlags, m_uiAreaId;
    SelectionRenderModel *m_pRenderModel;
    AbstractTimePhysicsModel   *m_pPhysicsModel;

    std::string m_sInput;
    
    Point m_ptTilePos;
    int dx, dy;
    float m_fDeltaZoom, m_fDeltaPitch;  //Camera deltas
};

#endif // EDITOR_OBJECT_H
