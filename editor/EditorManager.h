/*
 * EditorManager.cpp
 * This class handles most of the logic of the editor.
 * The EditorCursor handles input events and renders the current location.
 */

#ifndef EDITOR_MANAGER_H
#define EDITOR_MANAGER_H

#include "pwe/PartitionedWorldEngine.h"
#include "editor/editor_defs.h"
#include "game/ObjectFactory.h"
#include <stack>
#include <queue>

class EditorCursor;

class EditorManager : public GameObject {
public:
    static void init() { m_pInstance = new EditorManager(PWE::get()->genId()); }
    static EditorManager *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return ED_TYPE_EDITOR_OBJECT; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "EditorManager"; }

    virtual RenderModel  *getRenderModel()  { return NULL; }
    virtual PhysicsModel *getPhysicsModel() { return NULL; }

    virtual int callBack(uint cID, void *data, uint id);

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
    void initEnterTextHud(const std::string &label, const std::string &finalizeLabel, bool isField = true);

    void initAreaPanel();
    void initAreaListPanel(uint uiAreaFirst);

    void initNewObjHud();
    void initClassListPanel(uint uiStart);

    void initCreateObjHud();
    void initAttributeListPanel(uint uiStart);


    void initTextureHud();
    void initTextureListPanel(uint uiStart);

    void initSelectionHud(EditorCursorState eState);
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

    uint m_uiId;
    flag_t m_uiFlags;
    EditorCursor *m_pEditorCursor;
    //EditorState m_eState, m_eNewState;
    std::stack<EditorState> m_skState;
    std::queue<uint> m_qEvents;
    std::vector<uint> m_vAreas;

    //Area list
    uint m_uiHudAreaButtonIdStart;
    uint m_uiAreaFirst;
    uint m_uiCurAreaId;

    //Right-panel list (classes, attribues, textures, etc)
    uint m_uiHudObjButtonIdStart; //start of obj ids;
    uint m_uiObjFirst;  //first item in the list
    uint m_uiObjMax;
    void (EditorManager::*initListPanelFunc)(uint uiFirst);

    //Class and Attribute list properties
    std::vector<const std::string*> m_vClasses;
    std::string m_sCurClassName;
    ObjectFactory::FactoryData *m_pCurData;
    uint m_uiCurObjId;
    std::string m_sCurKey;
    AttributeType m_eCurAttrType;

    std::string m_sFile;

    uint m_uiCurImageId;

    static const float BUTTON_TEXT_SIZE = 0.8f;
};

#endif //EDITOR_MANAGER_H
