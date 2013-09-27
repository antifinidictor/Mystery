/*
 * EditorCursor.cpp
 */

#include "EditorCursor.h"
#include "EditorManager.h"
#include <string>
using namespace std;


EditorCursor::EditorCursor(uint uiId, uint uiAreaId, const Point &ptPos) {
    m_uiId = uiId;
    m_uiFlags = 0;
    m_uiAreaId = uiAreaId;
    m_uiBlinkTimer = 0;

    m_ptTilePos = toTile(ptPos);

    Box bxVolume = Box(m_ptTilePos.x, m_ptTilePos.y, m_ptTilePos.z, TILE_SIZE, TILE_SIZE, TILE_SIZE);
    m_pPhysicsModel = new NullTimePhysicsModel(bxVolume);
    m_pRenderModel  = new SelectionRenderModel(bxVolume, Color(0x0, 0x0, 0xFF));

    m_ptDeltaPos = Point();
    m_fDeltaPitch = m_fDeltaZoom = 0.f;

    PWE *we = PWE::get();
    we->addListener(this, ON_BUTTON_INPUT, m_uiAreaId);
    we->addListener(this, PWE_ON_AREA_SWITCH, m_uiAreaId);

    D3HudRenderModel *posText = new D3HudRenderModel("(?,?,?)", Rect(0,0,BUTTON_WIDTH,BUTTON_HEIGHT));
    D3RE::get()->getHudContainer()->add(ED_HUD_CURSOR_POS, posText);
    m_eState = EDC_STATE_MOVE;
}

EditorCursor::~EditorCursor() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;

    PWE *we = PWE::get();
    we->removeListener(this->getID(), ON_BUTTON_INPUT, m_uiAreaId);
    we->removeListener(this->getID(), PWE_ON_AREA_SWITCH, m_uiAreaId);
}


GameObject*
EditorCursor::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    return NULL;
}

void
EditorCursor::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
}

bool
EditorCursor::update(uint time) {
    switch(m_eState) {
    case EDC_STATE_STATIC:
        staticUpdate();
        break;
    case EDC_STATE_MOVE:
        moveUpdate();
        break;
    case EDC_STATE_SELECT_VOL:
        selectVolumeUpdate();
        break;
    case EDC_STATE_SELECT_RECT:
        selectRectUpdate();
        break;
    case EDC_STATE_TYPE_FIELD:
    case EDC_STATE_TYPE:
        typeUpdate();
        break;
    default:
        break;
    }
    return false;
}


void
EditorCursor::callBack(uint cID, void *data, uint eventId) {
    switch(eventId) {
    case PWE_ON_AREA_SWITCH: {
        EditorManager::get()->setEditorCursor(this);
        break;
      }
    case ON_BUTTON_INPUT: {
        switch(m_eState) {
        case EDC_STATE_STATIC: {
            staticOnKeyPress((InputData*)data);
            break;
            }
        case EDC_STATE_MOVE:
            moveOnKeyPress((InputData*)data);
            break;
        case EDC_STATE_SELECT_VOL:
            selectVolumeOnKeyPress((InputData*)data);
            break;
        case EDC_STATE_SELECT_RECT:
            selectRectOnKeyPress((InputData*)data);
            break;
        case EDC_STATE_TYPE_FIELD:
        case EDC_STATE_TYPE:
            typeOnKeyPress((InputData*)data);
            break;
        default:
            break;
        }
        break;
      }
    default:
        break;
    }
}


void
EditorCursor::setState(EditorCursorState eState) {
    //Clean up old state
    switch(m_eState) {
    case EDC_STATE_STATIC: {
        break;
      }
    case EDC_STATE_MOVE: {
        break;
      }
    case EDC_STATE_SELECT_VOL: {
        break;
      }
    case EDC_STATE_SELECT_RECT: {
        break;
      }
    case EDC_STATE_TYPE_FIELD:
    case EDC_STATE_TYPE: {
        break;
      }
    default:
        break;
    }

    //Set up new state
    m_eState = eState;
    switch(m_eState) {
    case EDC_STATE_STATIC: {
        m_pRenderModel->setVolume(Box(m_ptTilePos.x, m_ptTilePos.y, m_ptTilePos.z, TILE_SIZE, TILE_SIZE, TILE_SIZE));
        m_pRenderModel->setColor(Color(0x0,0x0,0xFF));
        break;
      }
    case EDC_STATE_MOVE: {
        m_pRenderModel->setVolume(Box(m_ptTilePos.x, m_ptTilePos.y, m_ptTilePos.z, TILE_SIZE, TILE_SIZE, TILE_SIZE));
        m_pRenderModel->setColor(Color(0x0,0x0,0xFF));
        break;
      }
    case EDC_STATE_SELECT_VOL: {
        m_pRenderModel->setColor(Color(0xFF,0x0,0x0));
        m_ptInitSelectPos = m_ptTilePos;
        break;
      }
    case EDC_STATE_SELECT_RECT: {
        break;
      }
    case EDC_STATE_TYPE_FIELD:
    case EDC_STATE_TYPE: {
        break;
      }
    default:
        break;
    }
}

void
EditorCursor::moveToArea(uint uiAreaTo) {
    PWE *we = PWE::get();
    we->removeListener(this->getID(), ON_BUTTON_INPUT, m_uiAreaId);
    we->removeListener(this->getID(), PWE_ON_AREA_SWITCH, m_uiAreaId);

    we->moveObjectToArea(getID(), m_uiAreaId, uiAreaTo);

    m_uiAreaId = uiAreaTo;
    we->addListener(this, ON_BUTTON_INPUT, m_uiAreaId);
    we->addListener(this, PWE_ON_AREA_SWITCH, m_uiAreaId);

    //Handle any other state-change stuff here
}


//state-specific update functions
void
EditorCursor::staticUpdate() {
    //TODO: Anything here?
}

void
EditorCursor::moveUpdate() {
    //Move the cursor
    m_pPhysicsModel->moveBy(m_ptDeltaPos);

    //Tile size jump?
    Point ptTileShift = toTile(m_pPhysicsModel->getPosition() - m_ptTilePos - Point(TILE_SIZE / 2, TILE_SIZE / 2, TILE_SIZE / 2));
    m_ptTilePos += Point(ptTileShift);
    m_pRenderModel->moveBy(ptTileShift);

    //Update the camera
    D3RE::get()->adjustCamAngle(m_fDeltaPitch);
    D3RE::get()->adjustCamDist(m_fDeltaZoom);

    D3RE::get()->moveScreenTo(m_pPhysicsModel->getPosition());

    //Update HUD display of the position
    std::ostringstream posText;
    posText << "#0000FF#(" << m_ptTilePos.x << "," << m_ptTilePos.y << ","
            << m_ptTilePos.z << ")";

    D3RE::get()->getHudContainer()->get<D3HudRenderModel*>(ED_HUD_CURSOR_POS)->updateText(posText.str());
}

void
EditorCursor::selectVolumeUpdate() {
    //Move the cursor
    m_pPhysicsModel->moveBy(m_ptDeltaPos);

    //Tile size jump?
    Point ptTileShift = toTile(m_pPhysicsModel->getPosition() - m_ptTilePos - Point(TILE_SIZE / 2, TILE_SIZE / 2, TILE_SIZE / 2));
    m_ptTilePos += Point(ptTileShift);
    Box bxVolume = Box(
        m_ptTilePos.x < m_ptInitSelectPos.x ? m_ptTilePos.x : m_ptInitSelectPos.x,
        m_ptTilePos.y < m_ptInitSelectPos.y ? m_ptTilePos.y : m_ptInitSelectPos.y,
        m_ptTilePos.z < m_ptInitSelectPos.z ? m_ptTilePos.z : m_ptInitSelectPos.z,
       (m_ptTilePos.x < m_ptInitSelectPos.x ? (m_ptInitSelectPos.x - m_ptTilePos.x) : (m_ptTilePos.x - m_ptInitSelectPos.x)) + TILE_SIZE,
       (m_ptTilePos.y < m_ptInitSelectPos.y ? (m_ptInitSelectPos.y - m_ptTilePos.y) : (m_ptTilePos.y - m_ptInitSelectPos.y)) + TILE_SIZE,
       (m_ptTilePos.z < m_ptInitSelectPos.z ? (m_ptInitSelectPos.z - m_ptTilePos.z) : (m_ptTilePos.z - m_ptInitSelectPos.z)) + TILE_SIZE
    );
    m_pRenderModel->setVolume(bxVolume);

    std::ostringstream posText;
    posText << "#FF0000#(" << m_ptTilePos.x << "," << m_ptTilePos.y << ","
            << m_ptTilePos.z << " : " << bxVolume.w << "," << bxVolume.h
            << "," << bxVolume.l << ")";
    D3HudRenderModel *cpos = D3RE::get()->getHudContainer()->get<D3HudRenderModel*>(ED_HUD_CURSOR_POS);
    cpos->updateText(posText.str());
}

void
EditorCursor::selectRectUpdate() {
    //TODO: implement
}

void
EditorCursor::typeUpdate() {
    m_uiBlinkTimer++;
    if(m_uiBlinkTimer == 50) {
        D3RE::get()->getHudContainer()
            ->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE)
            ->get<D3HudRenderModel*>(ED_HUD_FIELD_TEXT)
            ->updateText(m_sInput + "_");
    } else if(m_uiBlinkTimer >= 100) {
        D3RE::get()->getHudContainer()
            ->get<ContainerRenderModel*>(ED_HUD_MIDDLE_PANE)
            ->get<D3HudRenderModel*>(ED_HUD_FIELD_TEXT)
            ->updateText(m_sInput);
        m_uiBlinkTimer = 0;
    }
}

//state-specific input handling functions
void
EditorCursor::staticOnKeyPress(InputData *data) {
    //TODO: anything?
}

void
EditorCursor::moveOnKeyPress(InputData *data) {
    m_fDeltaPitch = 0.f;
    if(data->getInputState(IN_SHIFT)) {
        if(data->getInputState(IN_NORTH)) {
            m_ptDeltaPos.y = 1.f;
        } else if(data->getInputState(IN_SOUTH)) {
            m_ptDeltaPos.y = -1.f;
        } else {
            m_ptDeltaPos.y = 0.f;
        }
        m_ptDeltaPos.z = 0;

        m_fDeltaZoom = 0.f;
    } else if(data->getInputState(IN_CTRL)) {
        if(data->getInputState(IN_NORTH)) {
            m_fDeltaZoom = -1.0f;
        } else if(data->getInputState(IN_SOUTH)) {
            m_fDeltaZoom = 1.0f;
        } else {
            m_fDeltaZoom = 0.f;
        }
        m_ptDeltaPos.z = 0;
    } else {
        if(data->getInputState(IN_NORTH)) {
            m_ptDeltaPos.z = -1;
        } else if(data->getInputState(IN_SOUTH)) {
            m_ptDeltaPos.z = 1;
        } else {
            m_ptDeltaPos.z = 0;
        }
        m_fDeltaZoom = 0.f;
    }

    if(data->getInputState(IN_WEST)) {
        m_ptDeltaPos.x = -1;
    } else if(data->getInputState(IN_EAST)) {
        m_ptDeltaPos.x = 1;
    } else {
        m_ptDeltaPos.x = 0;
    }
}

void
EditorCursor::selectVolumeOnKeyPress(InputData *data) {
    moveOnKeyPress(data);
}

void
EditorCursor::selectRectOnKeyPress(InputData *data) {
    //TODO: Implement!
}

void
EditorCursor::typeOnKeyPress(InputData *data) {
    if(data->getInputState(KIN_LETTER_PRESSED) && data->hasChanged(KIN_LETTER_PRESSED)) {
        uint letters = data->getLettersDown();
        uint l = 1;
        for(int i = 0; i < 26; ++i) {
            if(letters & l && data->getInputState(IN_SHIFT)) {
                m_sInput.append(1, 'A' + i);
            } else if(letters & l) {
                m_sInput.append(1, 'a' + i);
            }
            l = l << 1;
        }
    }
    if(data->getInputState(KIN_NUMBER_PRESSED) && data->hasChanged(KIN_NUMBER_PRESSED)) {
        uint numbers = data->getNumbersDown();
        uint n = 1;
        for(int i = 0; i < 10; ++i) {
            if(numbers & n) {
                m_sInput.append(1, '0' + i);
            }
            n = n << 1;
        }
    }

    if(data->getInputState(ED_IN_SPACE) && data->hasChanged(ED_IN_SPACE)) {
        m_sInput.append(1, ' ');
    }
    if(data->getInputState(ED_IN_PERIOD) && data->hasChanged(ED_IN_PERIOD)) {
        m_sInput.append(1, '.');
    }
    if(data->getInputState(ED_IN_UNDERSCORE) && data->hasChanged(ED_IN_UNDERSCORE)) {
        if(data->getInputState(IN_SHIFT)) {
            m_sInput.append(1, '_');
        } else {
            m_sInput.append(1, '-');
        }
    }
    if(data->getInputState(ED_IN_SLASH) && data->hasChanged(ED_IN_SLASH)) {
        m_sInput.append(1, '/');
    }
    if(data->getInputState(ED_IN_COLON) && data->hasChanged(ED_IN_COLON)) {
        m_sInput.append(1, ':');
    }
    if(data->getInputState(ED_IN_BACKSPACE) && data->hasChanged(ED_IN_BACKSPACE)) {
        m_sInput.resize(m_sInput.size() - 1);
    }
    if(data->getInputState(ED_IN_ENTER) && data->hasChanged(ED_IN_ENTER)) {
        if(m_eState == EDC_STATE_TYPE_FIELD) {
            EditorManager::get()->callBack(this->getID(), NULL, ED_HUD_OP_FINALIZE);
        } else {
            m_sInput.append(1, '\n');
        }
    }
}


Point
EditorCursor::toTile(const Point &pt) {
    Point res = Point(
        ((int)pt.x) / TILE_SIZE,
        ((int)pt.y) / TILE_SIZE,
        ((int)pt.z) / TILE_SIZE
    );
    res *= TILE_SIZE;
    return res;
}


