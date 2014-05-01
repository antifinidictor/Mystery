/*
 * EditorCursor
 * Responsible for handling and translating inputs for the editor
 */
#ifndef EDITOR_CURSOR_H
#define EDITOR_CURSOR_H

#include "mge/GameObject.h"
#include "mge/Event.h"

#include "editor/SelectionRenderModel.h"
#include "editor/editor_defs.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "tpe/tpe.h"

class EditorCursor : public GameObject
{
public:
    EditorCursor(uint uiId, uint uiAreaId, const Point &ptPos);
    virtual ~EditorCursor();

    //File i/o
    static GameObject* read(const boost::property_tree::ptree &pt, const std::string &keyBase);
    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase);

    //General
    virtual bool update(float fDeltaTime);

    virtual uint getId()                        { return m_uiId; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    virtual uint getType()                      { return ED_TYPE_EDITOR_OBJECT; }
    virtual const std::string getClass()        { return getClassName(); }
    static const std::string getClassName()     { return "EditorCursor"; }

    //Render model
    virtual RenderModel  *getRenderModel()      { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel()     { return m_pPhysicsModel; }

    //Input
    virtual int callBack(uint cID, void *data, uint id);

    void setState(EditorCursorState eState);
    void moveToArea(uint uiAreaTo);

    Box getVolume() { return m_pRenderModel->getVolume(); }
    Point getPosition() { return m_pPhysicsModel->getPosition(); }
    std::string getText() { return m_sInput; }
    void clearText() { m_sInput.clear(); }
    void setText(const std::string &s) { m_sInput = s; }
    void snapX();
    void snapY();
    void snapZ();

private:
    //state-specific update functions
    void staticUpdate();
    void moveUpdate();
    void selectVolumeUpdate();
    void selectRectUpdate();
    void typeUpdate();

    //state-specific input handling functions
    int staticOnKeyPress(InputData *data);
    int moveOnKeyPress(InputData *data);
    int selectVolumeOnKeyPress(InputData *data);
    int selectRectOnKeyPress(InputData *data);
    int typeOnKeyPress(InputData *data);

    Point toTile(const Point &pt);
    Point getTileShift();

    uint m_uiId;
    flag_t m_uiFlags;
    uint m_uiAreaId;
    SelectionRenderModel *m_pRenderModel;
    AbstractTimePhysicsModel *m_pPhysicsModel;

    //state info
    EditorCursorState m_eState;

    //text input state info
    std::string m_sInput;
    uint m_uiBlinkTimer;

    //Camera and world position info
    Point m_ptTilePos, m_ptInitSelectPos;
    Point m_ptDeltaPos;
    float m_fDeltaZoom, m_fDeltaPitch;  //Camera deltas
};

#endif //EDITOR_CURSOR_H
