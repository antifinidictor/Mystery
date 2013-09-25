/*
 * EditorManager.cpp
 * This class handles most of the logic of the editor.
 * The EditorObject handles input events and renders the current location.
 */

#ifndef EDITOR_MANAGER_H
#define EDITOR_MANAGER_H

#include "pwe/PartitionedWorldEngine.h"
#include "editor/editor_defs.h"

class EditorObject;

class EditorManager : public GameObject, public Listener {
public:
    static void init() { m_pInstance = new EditorManager(PWE::get()->genID()); }
    static EditorManager *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual bool update(uint time);

    virtual uint getID()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return ED_TYPE_EDITOR_OBJECT; }
    virtual const std::string getClass()        { return "EditorManager"; }

    virtual RenderModel  *getRenderModel()  { return NULL; }
    virtual PhysicsModel *getPhysicsModel() { return NULL; }

    virtual void callBack(uint cID, void *data, uint id);
    
    void setEditorObject(EditorObject *obj) { m_pEditorObject = obj; }
    EditorState getState() { return m_eState; }

private:
    EditorManager(uint uiId);
    virtual ~EditorManager();
    
    void initMainHud();

    static EditorManager *m_pInstance;

    uint m_uiId, m_uiFlags;
    EditorObject *m_pEditorObject;
    EditorState m_eState, m_eNewState;
};

#endif //EDITOR_MANAGER_H
