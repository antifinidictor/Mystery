/*
 * EditorManager.cpp
 */
#include "EditorManager.h"
#include "EditorCursor.h"
#include "game/gui/GuiButton.h"
#include "game/world/Wall.h"
#include "game/GameManager.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/ObjectFactory.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

#define UNUSED_TEXTURE_ID 0xFFFFFFFF

EditorManager *EditorManager::m_pInstance;
EditorManager::BUTTON_TEXT_SIZE = 0.8f;

EditorManager::EditorManager(uint uiId) {
    m_uiId = uiId;
    m_uiFlags = 0;
    PWE::get()->setState(PWE_PAUSED);
    m_pEditorCursor = NULL;
    m_skState.push(ED_STATE_INIT);
    m_uiCurAreaId = 1;
    m_uiAreaFirst = 0;
    m_uiCurAreaId = 0;
    m_uiObjFirst = 0;
    m_uiHudObjButtonIdStart = 0;
    m_uiHudAreaButtonIdStart = 0;
    m_uiObjMax = 0;
    initListPanelFunc = NULL;
    m_sFile = "res/world.info";
    m_uiCurImageId = 3;

    MGE::get()->addListener(this, ON_BUTTON_INPUT);

    m_pBasePanel = new ContainerRenderModel(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
    D3RE::get()->getHudContainer()->add(ED_HUD_BASE_PANE, m_pBasePanel);

}

EditorManager::~EditorManager() {
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);
}

void
EditorManager::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
EditorManager::update(float fDeltaTime) {

    //Handle events
    while(!m_qEvents.empty()) {
        uint eventId = m_qEvents.front();
        switch(eventId) {
        case ED_HUD_OP_FINALIZE: {
            //Do some additional stuff here before popping the state
            switch(m_skState.top()) {
            case ED_STATE_LOAD_FILE:
                m_sFile = m_pEditorCursor->getText();
                ObjectFactory::get()->read(m_sFile);
                initAreaListPanel(m_uiAreaFirst);
                break;
            case ED_STATE_SAVE_FILE:
                D3RE::get()->freeImage(0);
                m_sFile = m_pEditorCursor->getText();
                ObjectFactory::get()->write(m_sFile);
                D3RE::get()->createImage(IMG_NONE, "res/gui/noImage.png");
                break;
            case ED_STATE_NAME_AREA:
                PWE::get()->setAreaName(PWE::get()->getCurrentArea(), m_pEditorCursor->getText());
                initAreaListPanel(m_uiAreaFirst);
                break;
            case ED_STATE_CREATE_OBJECT: {
                PWE::get()->freeId(m_uiCurObjId);   //The object will attempt to reserve it in a minute
                GameObject *obj = ObjectFactory::get()->createFromAttributes();
                PWE::get()->add(obj);
                break;
              }
            case ED_STATE_ENTER_UINT: {
                string s = m_pEditorCursor->getText();
                try {
                    m_pCurData->setAttribute(m_sCurKey, boost::lexical_cast<uint>(s));
                } catch(boost::bad_lexical_cast &){
                    printf("ERROR %s %d: Failed to convert %s to uint\n", __FILE__, __LINE__, s.c_str());
                }
                break;
              }
            case ED_STATE_ENTER_INT: {
                string s = m_pEditorCursor->getText();
                try {
                    m_pCurData->setAttribute(m_sCurKey, boost::lexical_cast<int>(s));
                } catch(boost::bad_lexical_cast &){
                    printf("ERROR %s %d: Failed to convert %s to int\n", __FILE__, __LINE__, s.c_str());
                }
                break;
              }
            case ED_STATE_ENTER_FLOAT: {
                string s = m_pEditorCursor->getText();
                try {
                    m_pCurData->setAttribute(m_sCurKey, boost::lexical_cast<float>(s));
                } catch(boost::bad_lexical_cast &){
                    printf("ERROR %s %d: Failed to convert %s to float\n", __FILE__, __LINE__, s.c_str());
                }
                break;
              }
            case ED_STATE_ENTER_STRING: {
                string s = m_pEditorCursor->getText();
                m_pCurData->setAttribute(m_sCurKey, s);
                break;
              }
            case ED_STATE_ENTER_COLOR: {
                string s = m_pEditorCursor->getText();
                tokenizer<> tok(s);
                int i = 0;
                Color cr = Color(0xFF,0xFF,0xFF);
                for(tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it){
                    try {
                        if(i == 0) {
                            cr.r = lexical_cast<uint>(*it);
                        } else if(i == 1) {
                            cr.g = lexical_cast<uint>(*it);
                        } else {
                            cr.b = lexical_cast<uint>(*it);
                        }
                        i++;
                    } catch(boost::bad_lexical_cast &){
                        printf("ERROR %s %d: Failed to convert item %d of %s to color\n", __FILE__, __LINE__, i, s.c_str());
                    }
                }
                m_pCurData->setAttribute(m_sCurKey, cr);
                break;
              }
            case ED_STATE_NEW_TEXTURE: {
                string s = m_pEditorCursor->getText();
                D3RE::get()->createImage(m_uiCurImageId++, s);
                break;
              }
            case ED_STATE_SELECT_VOLUME: {
                Box bx = m_pEditorCursor->getVolume();
                m_pCurData->setAttribute(m_sCurKey, bx);
                break;
              }
            case ED_STATE_SELECT_POINT: {
                Point pt = m_pEditorCursor->getPosition();
                m_pCurData->setAttribute(m_sCurKey, pt);
                break;
              }
            default:
                break;
            }
          }
            //Continue without breaking
        case ED_HUD_OP_CHOOSE_TEXTURE:
        case ED_HUD_OP_CANCEL:
            popState();
            break;
        case ED_HUD_OP_LOAD_WORLD:
            m_pEditorCursor->setText(m_sFile);
            pushState(ED_STATE_LOAD_FILE);
            break;
        case ED_HUD_OP_SAVE_WORLD:
            m_pEditorCursor->setText(m_sFile);
            pushState(ED_STATE_SAVE_FILE);
            break;
        case ED_HUD_OP_NEW_OBJ:
            pushState(ED_STATE_LIST_OBJECTS);
            break;
        case ED_HUD_OP_CHOOSE_OBJ: {
            m_uiCurObjId = PWE::get()->genId();
            ostringstream obName;
            obName << m_sCurClassName << m_uiCurObjId;
            m_pCurData = &ObjectFactory::get()->initObject(m_sCurClassName, obName.str());
            m_pCurData->setAttribute("id", m_uiCurObjId);
            printf("%s id = %d\n",obName.str().c_str(), m_pCurData->getAttribute("id", 0));

            pushState(ED_STATE_CREATE_OBJECT);
            break;
          }
        case ED_HUD_OP_SNAP_X:
            m_pEditorCursor->snapX();
            break;
        case ED_HUD_OP_SNAP_Y:
            m_pEditorCursor->snapY();
            break;
        case ED_HUD_OP_SNAP_Z:
            m_pEditorCursor->snapZ();
            break;
        case ED_HUD_OP_ATTR: {
            switch(m_eCurAttrType) {
            case ATYPE_RESOURCE_ID: {
                pushState(ED_STATE_LIST_TEXTURES);
                break;
              }
            case ATYPE_UINT:
            case ATYPE_OBJECT_ID: {
                m_pEditorCursor->setText("");
                pushState(ED_STATE_ENTER_UINT);
                break;
              }
            case ATYPE_INT: {
                m_pEditorCursor->setText("");
                pushState(ED_STATE_ENTER_INT);
                break;
              }
            case ATYPE_FLOAT: {
                m_pEditorCursor->setText("");
                pushState(ED_STATE_ENTER_FLOAT);
                break;
              }
            case ATYPE_POINT: {
                pushState(ED_STATE_SELECT_POINT);
                break;
              }
            case ATYPE_RECT: {
                pushState(ED_STATE_LIST_TEXTURES);
                break;
              }
            case ATYPE_BOX: {
                pushState(ED_STATE_SELECT_VOLUME);
                break;
              }
            case ATYPE_COLOR: {
                m_pEditorCursor->setText("");
                pushState(ED_STATE_ENTER_COLOR);
                break;
              }
            case ATYPE_STRING: {
                m_pEditorCursor->setText("");
                pushState(ED_STATE_ENTER_STRING);
                break;
              }
            default:
                break;
            }
            break;
          }
        case ED_HUD_OP_NEW_TEXTURE:
            m_pEditorCursor->setText("res/");
            pushState(ED_STATE_NEW_TEXTURE);
            break;
        case ED_HUD_OP_NEW_AREA: {
            uint uiAreaId = PWE::get()->generateArea();
            PWE::get()->setCurrentArea(uiAreaId);
            m_uiCurAreaId = uiAreaId;
            m_pEditorCursor->moveToArea(uiAreaId);
            m_pEditorCursor->setText(PWE::get()->getAreaName(uiAreaId));
            pushState(ED_STATE_NAME_AREA);
            break;
        }
        case ED_HUD_OP_RENAME_AREA:
            m_pEditorCursor->setText(PWE::get()->getAreaName(PWE::get()->getCurrentArea()));
            pushState(ED_STATE_NAME_AREA);
            break;
        case ED_HUD_OP_UP_AREA:
        case ED_HUD_OP_DOWN_AREA:
        case ED_HUD_OP_GO_TO_AREA:
            initAreaListPanel(m_uiAreaFirst);
            break;
        case ED_HUD_OP_UP:
        case ED_HUD_OP_DOWN:
            (this->*initListPanelFunc)(m_uiObjFirst);
            break;
        default:
            break;
        }
        m_qEvents.pop();
    }


    switch(m_skState.top()) {
    case ED_STATE_INIT:
        //Add basic hud containers
        initHud();
        pushState(ED_STATE_MAIN);   //Push on main state

        //Continue into the main state
    case ED_STATE_MAIN:
        break;
    case ED_STATE_TEST_GAME:
        GameManager::get()->update(fDeltaTime);
        break;
    default:
        break;
    }

    if(m_pEditorCursor != NULL) {
        m_pEditorCursor->update(fDeltaTime);
    }

    return false;
}

int
EditorManager::callBack(uint cID, void *data, uint eventId) {
    //If the data needs to be handled, handle it here.  Otherwise push an event.
    int status = EVENT_CAUGHT;
    switch(eventId) {
    case ON_BUTTON_INPUT: {
        InputData *input = (InputData*)data;
        if(input->getInputState(IN_TOGGLE_DEBUG_MODE) && input->hasChanged(IN_TOGGLE_DEBUG_MODE)) {
            if(m_skState.top() == ED_STATE_TEST_GAME) {
                popState();
            } else {
                pushState(ED_STATE_TEST_GAME);
            }
        } else {
            status = EVENT_DROPPED;
        }
        break;
      }
    case ED_HUD_OP_FINALIZE:
        //Do some additional stuff here before popping the state
        break;
    case ED_HUD_OP_GO_TO_AREA: {
        //Get the actual area in question
        uint uiAreaId = m_uiAreaFirst + cID - m_uiHudAreaButtonIdStart;
        PWE::get()->setCurrentArea(uiAreaId);
        m_uiCurAreaId = uiAreaId;
        m_pEditorCursor->moveToArea(uiAreaId);
        break;
      }
    case ED_HUD_OP_CHOOSE_OBJ: {
        uint uiClassId = m_uiObjFirst + cID - m_uiHudObjButtonIdStart;
        m_sCurClassName = *m_vClasses[uiClassId];
        break;
      }
    case ED_HUD_OP_ATTR: {
        uint uiAttrId = m_uiObjFirst + cID - m_uiHudObjButtonIdStart;
        list<ObjectFactory::AttributeInfo>::iterator iter = m_pCurData->m_lsAttributeInfo.begin();
        for(uint i = 0; i < uiAttrId; ++i) {
            ++iter; //Skip uiAttrId attributes
        }
        m_sCurKey = iter->m_sAttributeKey;
        m_eCurAttrType = iter->m_eType;
        //Now handle specific attributes
        break;
      }
    case ED_HUD_OP_CHOOSE_TEXTURE: {
        uint uiTexId = m_uiObjFirst + cID - m_uiHudObjButtonIdStart;
        m_pCurData->setAttribute(m_sCurKey, uiTexId);
        break;
      }
    case ED_HUD_OP_UP_AREA:
        if(m_uiAreaFirst > 0) {
            m_uiAreaFirst--;
        }
        break;
    case ED_HUD_OP_DOWN_AREA:
        if(m_uiAreaFirst < m_vAreas.size() - 1) {
            m_uiAreaFirst++;
        }
        break;
    case ED_HUD_OP_UP:
        if(m_uiObjFirst > 0) {
            m_uiObjFirst--;
        }
        break;
    case ED_HUD_OP_DOWN:
        if(m_uiObjFirst < m_uiObjMax) {
            m_uiObjFirst++;
        }
        break;
    default:
        status = EVENT_DROPPED;
        break;
    }
    m_qEvents.push(eventId);
    return status;
}


void
EditorManager::pushState(EditorState eState) {
    cleanState(m_skState.top());
    m_skState.push(eState);
    initState(m_skState.top());
}

void
EditorManager::popState() {
    cleanState(m_skState.top());
    m_skState.pop();
    initState(m_skState.top());
}

void
EditorManager::cleanState(EditorState eState) {
    switch(eState) {
    case ED_STATE_ENTER_UINT:
    case ED_STATE_ENTER_INT:
    case ED_STATE_ENTER_FLOAT:
    case ED_STATE_ENTER_STRING:
    case ED_STATE_ENTER_COLOR:
    case ED_STATE_NEW_TEXTURE:
    case ED_STATE_NAME_AREA:
    case ED_STATE_SAVE_FILE:
    case ED_STATE_LOAD_FILE:
        m_pBasePanel
            ->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE)
            ->clear();
        //Also clear the right pane
    case ED_STATE_SELECT_VOLUME:
    case ED_STATE_SELECT_POINT:
    case ED_STATE_LIST_TEXTURES:
    case ED_STATE_CREATE_OBJECT:
    case ED_STATE_LIST_OBJECTS:
    case ED_STATE_MAIN:
        m_pBasePanel
            ->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE)
            ->clear();
        break;
    case ED_STATE_TEST_GAME:
        PWE::get()->add(m_pEditorCursor);
        PWE::get()->setState(PWE_PAUSED);
        D3RE::get()->getHudContainer()->add(ED_HUD_BASE_PANE, m_pBasePanel);
        break;
    default:
        break;
    }
}

void
EditorManager::initState(EditorState eState) {
    switch(eState) {
    case ED_STATE_TEST_GAME:
        PWE::get()->remove(m_pEditorCursor->getId());
        D3RE::get()->getHudContainer()->remove(ED_HUD_BASE_PANE);
        PWE::get()->setState(PWE_RUNNING);
        break;
    case ED_STATE_MAIN:
        //Restore main hud
        initMainHud();
        break;
    case ED_STATE_SAVE_FILE:
        initEnterTextHud("Enter filename:", "Save it!");
        break;
    case ED_STATE_LOAD_FILE:
        initEnterTextHud("Enter filename:", "Load it!");
        break;
    case ED_STATE_NAME_AREA:
        initEnterTextHud("Enter area name:", "Set name!");
        break;
    case ED_STATE_LIST_OBJECTS:
        initNewObjHud();
        initListPanelFunc = &EditorManager::initClassListPanel;
        break;
    case ED_STATE_CREATE_OBJECT:
        initCreateObjHud();
        initListPanelFunc = &EditorManager::initAttributeListPanel;
        break;
    case ED_STATE_LIST_TEXTURES:
        initTextureHud();
        initListPanelFunc = &EditorManager::initTextureListPanel;
        break;
    case ED_STATE_ENTER_UINT:
        initEnterTextHud("Enter unsigned integer:", "Done");
        break;
    case ED_STATE_ENTER_INT:
        initEnterTextHud("Enter signed integer:", "Done");
        break;
    case ED_STATE_ENTER_FLOAT:
        initEnterTextHud("Enter floating point number:", "Done");
        break;
    case ED_STATE_ENTER_STRING:
        initEnterTextHud("Enter string:", "Done", false);
        break;
    case ED_STATE_ENTER_COLOR:
        initEnterTextHud("Enter an rgb color (ex: \"255 255 255\"):", "Done");
        break;
    case ED_STATE_NEW_TEXTURE:
        initEnterTextHud("Enter texture file:", "Done");
        break;
    case ED_STATE_SELECT_VOLUME:
        initSelectionHud(EDC_STATE_SELECT_VOL);
        break;
    case ED_STATE_SELECT_POINT:
        initSelectionHud(EDC_STATE_MOVE);
        break;
    default:
        break;
    }
}

void
EditorManager::initHud() {
    ContainerRenderModel *leftPane = new ContainerRenderModel(Rect(0, BUTTON_HEIGHT, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT)),
                       *middlePane = new ContainerRenderModel(Rect(BUTTON_WIDTH, 0, SCREEN_WIDTH - BUTTON_WIDTH * 2, SCREEN_HEIGHT)),
                        *rightPane = new ContainerRenderModel(Rect(SCREEN_WIDTH - BUTTON_WIDTH, 0, BUTTON_WIDTH, SCREEN_HEIGHT))
    ;
    ContainerRenderModel *hud = m_pBasePanel;
    hud->add(ED_HUD_LEFT_PANE,   leftPane);
    hud->add(ED_HUD_MIDDLE_PANE, middlePane);
    hud->add(ED_HUD_RIGHT_PANE,  rightPane);

    initAreaPanel();
}


void
EditorManager::initMainHud() {
    ContainerRenderModel *rpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);

    int i = 0;
    GuiButton *loadWorld = new GuiButton(rpanel, this, ED_HUD_OP_LOAD_WORLD, "Load World", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *saveWorld  = new GuiButton(rpanel, this, ED_HUD_OP_SAVE_WORLD, "Save World", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *newObj     = new GuiButton(rpanel, this, ED_HUD_OP_NEW_OBJ, "New Object", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *newTex     = new GuiButton(rpanel, this, ED_HUD_OP_NEW_TEXTURE, "New Texture", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);

    rpanel->add(ED_HUD_MAIN_LOAD_WORLD, loadWorld);
    rpanel->add(ED_HUD_MAIN_SAVE_WORLD, saveWorld);
    rpanel->add(ED_HUD_MAIN_NEW_OBJ, newObj);
    rpanel->add(ED_HUD_MAIN_NEW_TEXTURE, newTex);

    if(m_pEditorCursor) {
        m_pEditorCursor->setState(EDC_STATE_MOVE);
    }
}

void
EditorManager::initEnterTextHud(const std::string &slabel, const std::string &finalizeLabel, bool isField) {
    ContainerRenderModel *rpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    ContainerRenderModel *mpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE);

    int i = 0;
    GuiButton *cancel = new GuiButton(rpanel, this, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE),
        *finalize  = new GuiButton(rpanel, this, ED_HUD_OP_FINALIZE, finalizeLabel, Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    D3HudRenderModel *label = new D3HudRenderModel(UNUSED_TEXTURE_ID, Rect(0,0,SCREEN_WIDTH - BUTTON_WIDTH*2,BUTTON_HEIGHT), slabel, Point());
    D3HudRenderModel *text = new D3HudRenderModel(UNUSED_TEXTURE_ID, Rect(0,BUTTON_HEIGHT,SCREEN_WIDTH - BUTTON_WIDTH*2,SCREEN_HEIGHT), "", Point());

    rpanel->add(ED_HUD_SAVE_CANCEL, cancel);
    rpanel->add(ED_HUD_SAVE_SAVE, finalize);
    mpanel->add(ED_HUD_FIELD_LABEL, label);
    mpanel->add(ED_HUD_FIELD_TEXT, text);

    if(m_pEditorCursor) {

        m_pEditorCursor->setState(isField ? EDC_STATE_TYPE_FIELD : EDC_STATE_TYPE);
    }
}


#define MAX_LIST_SIZE 12
void
EditorManager::initAreaPanel() {
    ContainerRenderModel *lpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_LEFT_PANE);

    int i = 0;
    GuiButton *newArea    = new GuiButton(lpanel, this, ED_HUD_OP_NEW_AREA, "New area", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *renameArea = new GuiButton(lpanel, this, ED_HUD_OP_RENAME_AREA, "Rename area", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *upArea     = new GuiButton(lpanel, this, ED_HUD_OP_UP_AREA, "UP", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *downArea   = new GuiButton(lpanel, this, ED_HUD_OP_DOWN_AREA, "DOWN", Point(0.f ,SCREEN_HEIGHT - BUTTON_HEIGHT * 2, 0.f), BUTTON_TEXT_SIZE);
    ContainerRenderModel *lspanel = new ContainerRenderModel(Rect(0.f, BUTTON_HEIGHT * i, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT * (i + 1)), Point(0.f, BUTTON_HEIGHT, 0.f));

    lpanel->add(ED_HUD_AREA_NEW, newArea);
    lpanel->add(ED_HUD_AREA_RENAME, renameArea);
    lpanel->add(ED_HUD_AREA_UP, upArea);
    lpanel->add(ED_HUD_AREA_DOWN, downArea);
    lpanel->add(ED_HUD_AREA_LIST_PANE, lspanel);

    initAreaListPanel(0);
}

void
EditorManager::initAreaListPanel(uint uiAreaFirst) {
    ContainerRenderModel *lspanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_LEFT_PANE)->get<ContainerRenderModel*>(ED_HUD_AREA_LIST_PANE);
    lspanel->clear();

    m_vAreas.clear();
    PWE::get()->getAreas(m_vAreas);

    uint i = 0;
    for(vector<uint>::iterator iter = m_vAreas.begin() + uiAreaFirst; iter < m_vAreas.end() && i < MAX_LIST_SIZE-2; ++iter) {
        ostringstream name;
        if((*iter) == m_uiCurAreaId) {
            name << "#00FF00#" << *iter << " " << PWE::get()->getAreaName(*iter);
        } else {
            name << *iter << " " << PWE::get()->getAreaName(*iter);
        }
        GuiButton *area = new GuiButton(lspanel, this, ED_HUD_OP_GO_TO_AREA, name.str(), Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
        lspanel->add(i, area);
        if(i == 0) {
            m_uiHudAreaButtonIdStart = area->getHudID();
        }
        ++i;
    }
}

void
EditorManager::initNewObjHud() {
    ContainerRenderModel *rpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    m_uiObjFirst = 0;

    int i = 0;
    GuiButton *cancel = new GuiButton(rpanel, this, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *upArea     = new GuiButton(rpanel, this, ED_HUD_OP_UP, "UP", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *downArea   = new GuiButton(rpanel, this, ED_HUD_OP_DOWN, "DOWN", Point(0.f ,SCREEN_HEIGHT - BUTTON_HEIGHT, 0.f), BUTTON_TEXT_SIZE);
    ContainerRenderModel *lspanel = new ContainerRenderModel(Rect(0.f, BUTTON_HEIGHT * i, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT * (i + 1)), Point(SCREEN_WIDTH - BUTTON_WIDTH, 0.f, 0.f));

    rpanel->add(ED_HUD_NEW_OBJ_CANCEL, cancel);
    rpanel->add(ED_HUD_NEW_OBJ_UP, upArea);
    rpanel->add(ED_HUD_NEW_OBJ_DOWN, downArea);
    rpanel->add(ED_HUD_NEW_OBJ_LIST_PANE, lspanel);

    initClassListPanel(m_uiObjFirst);
}

void
EditorManager::initClassListPanel(uint uiStart) {
    ContainerRenderModel *lspanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE)->get<ContainerRenderModel*>(ED_HUD_NEW_OBJ_LIST_PANE);
    lspanel->clear();

    //Get available classes
    vector<const string *>::iterator iter;
    m_vClasses.clear();

    ObjectFactory::get()->getClassList(m_vClasses);
    m_uiObjMax = m_vClasses.size() - 1;

    //Display available classes
    uint i = 0;
    for(iter = m_vClasses.begin() + uiStart; iter != m_vClasses.end() && i < MAX_LIST_SIZE + 1; ++iter) {
        GuiButton *classButton = new GuiButton(lspanel, this, ED_HUD_OP_CHOOSE_OBJ, **iter, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
        lspanel->add(i, classButton);
        if(i == 0) {
            m_uiHudObjButtonIdStart = classButton->getHudID();
        }
        i++;
    }
}

void
EditorManager::initCreateObjHud() {
    ContainerRenderModel *rpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    m_uiObjFirst = 0;

    int i = 0;
    GuiButton *cancel   = new GuiButton(rpanel, this, ED_HUD_OP_CANCEL,   "Cancel",        Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *finalize = new GuiButton(rpanel, this, ED_HUD_OP_FINALIZE, "Create it!",    Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *upArea   = new GuiButton(rpanel, this, ED_HUD_OP_UP,       "UP",            Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *downArea = new GuiButton(rpanel, this, ED_HUD_OP_DOWN,     "DOWN",          Point(0.f ,SCREEN_HEIGHT - BUTTON_HEIGHT, 0.f), BUTTON_TEXT_SIZE);
    ContainerRenderModel *lspanel = new ContainerRenderModel(Rect(0.f, BUTTON_HEIGHT * i, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT * (i + 1)), Point(SCREEN_WIDTH - BUTTON_WIDTH, 0.f, 0.f));

    rpanel->add(ED_HUD_NEW_OBJ_CANCEL, cancel);
    rpanel->add(ED_HUD_NEW_OBJ_MAKE, finalize);
    rpanel->add(ED_HUD_NEW_OBJ_UP, upArea);
    rpanel->add(ED_HUD_NEW_OBJ_DOWN, downArea);
    rpanel->add(ED_HUD_NEW_OBJ_LIST_PANE, lspanel);

    initAttributeListPanel(m_uiObjFirst);

    if(m_pEditorCursor) {
        m_pEditorCursor->setState(EDC_STATE_MOVE);
    }
}

void
EditorManager::initAttributeListPanel(uint uiStart) {
    ContainerRenderModel *lspanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE)->get<ContainerRenderModel*>(ED_HUD_NEW_OBJ_LIST_PANE);
    lspanel->clear();

    //Get available attributes
    m_uiObjMax = m_pCurData->m_lsAttributeInfo.size() - 1;

    list<ObjectFactory::AttributeInfo>::iterator iter = m_pCurData->m_lsAttributeInfo.begin();
    for(uint i = 0; i < uiStart; ++i) {
        ++iter; //Skip uiStart attributes
    }

    //Iterate through the attributes, handling each type appropriately
    uint i = 0;
    GuiButton *attrButton;
    for(; iter != m_pCurData->m_lsAttributeInfo.end() && i < MAX_LIST_SIZE; ++iter) {
        ostringstream attrName;
        switch(iter->m_eType) {
        case ATYPE_INT:
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, iter->m_sAttributeName, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
        case ATYPE_UINT:
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, iter->m_sAttributeName, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
        case ATYPE_FLOAT:
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, iter->m_sAttributeName, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
        case ATYPE_POINT: {
            attrName << iter->m_sAttributeName << " = " << m_pCurData->getAttribute(iter->m_sAttributeKey, Point());
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, attrName.str(), Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
          }
        case ATYPE_RECT: {
            attrName << iter->m_sAttributeName << " = " << m_pCurData->getAttribute(iter->m_sAttributeKey, Rect());
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, attrName.str(), Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
          }
        case ATYPE_BOX: {
            attrName << iter->m_sAttributeName << " = " << m_pCurData->getAttribute(iter->m_sAttributeKey, Box());
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, attrName.str(), Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
          }
        case ATYPE_COLOR: {
            Color cr = m_pCurData->getAttribute(iter->m_sAttributeKey, Color(0xFF,0xFF,0xFF));
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, iter->m_sAttributeName, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            D3HudRenderModel *tex = new D3HudRenderModel(1, Rect(BUTTON_WIDTH - TEXTURE_TILE_SIZE, BUTTON_HEIGHT * i, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE));
            tex->setImageColor(cr);
            lspanel->add(i * 2 + 1, tex);
            break;
          }
        case ATYPE_STRING:
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, iter->m_sAttributeName, Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
        case ATYPE_RESOURCE_ID: {
            uint texId = m_pCurData->getAttribute(iter->m_sAttributeKey, (uint)0);
            attrName << iter->m_sAttributeName << " = " << texId;
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, attrName.str(), Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            D3HudRenderModel *tex = new D3HudRenderModel(texId, Rect(BUTTON_WIDTH - TEXTURE_TILE_SIZE, BUTTON_HEIGHT * i, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE));
            lspanel->add(i * 2 + 1, tex);
            break;
          }
        case ATYPE_OBJECT_ID: {
            attrName << iter->m_sAttributeName << " = " << m_pCurData->getAttribute(iter->m_sAttributeKey, 0);
            attrButton = new GuiButton(lspanel, this, ED_HUD_OP_ATTR, attrName.str(), Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
            break;
          }
        default:
            printf("ERROR %s %d: Unknown attribute type %d\n", __FILE__, __LINE__, iter->m_eType);
            break;
        }
        lspanel->add(i * 2, attrButton);    //allows space for additional displays
        if(i == 0) {
            m_uiHudObjButtonIdStart = attrButton->getHudID();
        }
        i++;
    }
}


void
EditorManager::initTextureHud() {
    ContainerRenderModel *rpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    m_uiObjFirst = 0;

    int i = 0;
    GuiButton *cancel = new GuiButton(rpanel, this, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *up     = new GuiButton(rpanel, this, ED_HUD_OP_UP, "UP", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *down   = new GuiButton(rpanel, this, ED_HUD_OP_DOWN, "DOWN", Point(0.f ,SCREEN_HEIGHT - BUTTON_HEIGHT, 0.f), BUTTON_TEXT_SIZE);
    ContainerRenderModel *lspanel = new ContainerRenderModel(Rect(0.f, BUTTON_HEIGHT * i, BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT * (i + 1)), Point(SCREEN_WIDTH - BUTTON_WIDTH, 0.f, 0.f));

    rpanel->add(ED_HUD_NEW_OBJ_CANCEL, cancel);
    rpanel->add(ED_HUD_NEW_OBJ_UP, up);
    rpanel->add(ED_HUD_NEW_OBJ_DOWN, down);
    rpanel->add(ED_HUD_NEW_OBJ_LIST_PANE, lspanel);

    initTextureListPanel(m_uiObjFirst);
}

void
EditorManager::initTextureListPanel(uint uiStart) {
    ContainerRenderModel *lspanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE)->get<ContainerRenderModel*>(ED_HUD_NEW_OBJ_LIST_PANE);
    lspanel->clear();

    //Get available classes
    D3RE *re = D3RE::get();
    m_uiObjMax = re->getNumImages() - 1;

    //Display available classes
    uint i = 0;
    for(uint img = uiStart; img <= m_uiObjMax && i < MAX_LIST_SIZE; ++img) {
        GuiButton *texButton = new GuiButton(lspanel, this, ED_HUD_OP_CHOOSE_TEXTURE, "", Point(0.f ,BUTTON_HEIGHT * i, 0.f), BUTTON_TEXT_SIZE);
        D3HudRenderModel *tex = new D3HudRenderModel(img, Rect(BUTTON_WIDTH / 2 - TEXTURE_TILE_SIZE / 2, BUTTON_HEIGHT * i, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE));
        lspanel->add(i * 2, texButton);
        lspanel->add(i * 2 + 1, tex);
        if(i == 0) {
            m_uiHudObjButtonIdStart = texButton->getHudID();
        }
        i++;
    }
}

void
EditorManager::initSelectionHud(EditorCursorState eState) {
    ContainerRenderModel *rpanel = m_pBasePanel->get<ContainerRenderModel*>(ED_HUD_RIGHT_PANE);
    int i = 0;
    GuiButton *cancel = new GuiButton(rpanel, this, ED_HUD_OP_CANCEL, "Cancel", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *finalize = new GuiButton(rpanel, this, ED_HUD_OP_FINALIZE, "Select", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *snapX = new GuiButton(rpanel, this, ED_HUD_OP_SNAP_X, "Snap X", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *snapY = new GuiButton(rpanel, this, ED_HUD_OP_SNAP_Y, "Snap Y", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    GuiButton *snapZ = new GuiButton(rpanel, this, ED_HUD_OP_SNAP_Z, "Snap Z", Point(0.f ,BUTTON_HEIGHT * i++, 0.f), BUTTON_TEXT_SIZE);
    rpanel->add(ED_HUD_SEL_CANCEL, cancel);
    rpanel->add(ED_HUD_SEL_FINALIZE, finalize);
    rpanel->add(ED_HUD_SEL_SNAP_X, snapX);
    rpanel->add(ED_HUD_SEL_SNAP_Y, snapY);
    rpanel->add(ED_HUD_SEL_SNAP_Z, snapZ);

    if(m_pEditorCursor != NULL) {
        m_pEditorCursor->setState(eState);
    }
}


