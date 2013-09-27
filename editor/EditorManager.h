/*
 * EditorManager.cpp
 * This class handles most of the logic of the editor.
 * The EditorCursor handles input events and renders the current location.
 */

#ifndef EDITOR_MANAGER_H
#define EDITOR_MANAGER_H

#include "pwe/PartitionedWorldEngine.h"
#include "editor/editor_defs.h"
#include <stack>
#include <queue>

class EditorCursor;

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
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "EditorManager"; }

    virtual RenderModel  *getRenderModel()  { return NULL; }
    virtual PhysicsModel *getPhysicsModel() { return NULL; }

    virtual void callBack(uint cID, void *data, uint id);

    void setEditorCursor(EditorCursor *obj) { m_pEditorCursor = obj; }
    //EditorState getState() { return m_eState; }

private:
    EditorManager(uint uiId);
    virtual ~EditorManager();

    void pushState(EditorState eState);
    void popState();

    void cleanState(EditorState eState);
    void initState(EditorState eState);

    void initHud();
    void initMainHud();
    void initLoadHud();
    void initSaveHud();


/*
    void prepState();

    void initConstHud();
    void initMainHud();
    void initCreateObjectHud();
    void initLoadFileHud();
    void initSaveFileHud();
    void initListObjectHud();
*/
    static EditorManager *m_pInstance;

    uint m_uiId, m_uiFlags;
    EditorCursor *m_pEditorCursor;
    //EditorState m_eState, m_eNewState;
    std::stack<EditorState> m_skState;
    std::queue<uint> m_qEvents;

    static const float BUTTON_TEXT_SIZE = 0.8f;
};

#endif //EDITOR_MANAGER_H
