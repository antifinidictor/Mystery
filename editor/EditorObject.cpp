#include "EditorObject.h"
#include "EditorManager.h"
#include "bae/BasicAudioEngine.h"
#include <string>
using namespace std;

EditorObject::EditorObject(uint uiId, uint uiAreaId, const Point &ptPos) {
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
    we->addListener(this, ON_BUTTON_INPUT, uiAreaId);
    we->addListener(this, PWE_ON_AREA_SWITCH, uiAreaId);
}

EditorObject::~EditorObject() {
    delete m_pPhysicsModel;
    delete m_pRenderModel;
}

GameObject*
EditorObject::read(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    uint id = pt.get(keyBase + ".id", 0);
    uint uiAreaId = pt.get(keyBase + ".areaId", 0);
    float x = pt.get(keyBase + ".pos.x", 0.f);
    float y = pt.get(keyBase + ".pos.y", 0.f);
    float z = pt.get(keyBase + ".pos.z", 0.f);

    //Put state information here

    return new EditorObject(id, uiAreaId, Point(x,y,z));
}

void
EditorObject::write(boost::property_tree::ptree &pt, const std::string &keyBase) {
    Point ptPos = m_pPhysicsModel->getPosition();
    pt.put(keyBase + ".id", m_uiId);
    pt.put(keyBase + ".areaId", m_uiAreaId);
    pt.put(keyBase + ".pos.x", ptPos.x);
    pt.put(keyBase + ".pos.y", ptPos.y);
    pt.put(keyBase + ".pos.z", ptPos.z);

    //Read state information here

}

//Editor objects are controlled by the EditorManager.
// The Manager responds to all events.
bool
EditorObject::update(uint time) {
    moveBy(m_ptDeltaPos);

    D3RE::get()->adjustCamAngle(m_fDeltaPitch);
    D3RE::get()->adjustCamDist(m_fDeltaZoom);

    D3RE::get()->moveScreenTo(m_pPhysicsModel->getPosition());

    std::ostringstream posText;

    if(EditorManager::get()->getState() == ED_STATE_NORMAL) {
        posText << "#0000FF#(" << m_ptTilePos.x << "," << m_ptTilePos.y << "," << m_ptTilePos.z << ")";
    } else if(EditorManager::get()->getState() == ED_STATE_SELECT) {
        Box bxVol = m_pRenderModel->getVolume();
        posText << "#FF0000#(" << m_ptTilePos.x << "," << m_ptTilePos.y << "," << m_ptTilePos.z << " : " << bxVol.w << "," << bxVol.h << "," << bxVol.l << ")";
    } else {
        m_uiBlinkTimer++;
        if(m_uiBlinkTimer == 50) {
            D3RE::get()->getHudElement(ED_TEXT)->updateText(m_sInput + "_");
        } else if(m_uiBlinkTimer >= 100) {
            D3RE::get()->getHudElement(ED_TEXT)->updateText(m_sInput);
            m_uiBlinkTimer = 0;
        }
    }
    D3RE::get()->getHudElement(ED_HUD_CURSOR_POS)->updateText(posText.str());
    return false;
}

void
EditorObject::moveBy(Point ptShift) {
    m_pPhysicsModel->moveBy(ptShift);

    //Tile size jump?
    Point ptTileShift = toTile(m_pPhysicsModel->getPosition() - m_ptTilePos - Point(TILE_SIZE / 2, TILE_SIZE / 2, TILE_SIZE / 2));
    m_ptTilePos += Point(ptTileShift);
    if(EditorManager::get()->getState() == ED_STATE_NORMAL) {
        m_pRenderModel->moveBy(ptTileShift);
    } else if(EditorManager::get()->getState() == ED_STATE_SELECT) {
        Box bxVolume = Box(
            m_ptTilePos.x < m_ptInitSelectPos.x ? m_ptTilePos.x : m_ptInitSelectPos.x,
            m_ptTilePos.y < m_ptInitSelectPos.y ? m_ptTilePos.y : m_ptInitSelectPos.y,
            m_ptTilePos.z < m_ptInitSelectPos.z ? m_ptTilePos.z : m_ptInitSelectPos.z,
           (m_ptTilePos.x < m_ptInitSelectPos.x ? (m_ptInitSelectPos.x - m_ptTilePos.x) : (m_ptTilePos.x - m_ptInitSelectPos.x)) + TILE_SIZE,
           (m_ptTilePos.y < m_ptInitSelectPos.y ? (m_ptInitSelectPos.y - m_ptTilePos.y) : (m_ptTilePos.y - m_ptInitSelectPos.y)) + TILE_SIZE,
           (m_ptTilePos.z < m_ptInitSelectPos.z ? (m_ptInitSelectPos.z - m_ptTilePos.z) : (m_ptTilePos.z - m_ptInitSelectPos.z)) + TILE_SIZE
        );
        m_pRenderModel->setVolume(bxVolume);
    }
}

void
EditorObject::callBack(uint cID, void *data, uint eventId) {
    //This is a hack, but it should largely work: When any event is called, this editor object's area has come into play
    if(eventId == PWE_ON_AREA_SWITCH) {
        EditorManager::get()->setEditorObject(this);
        prepState(EditorManager::get()->getState());    //on state change
        return;
    }

    switch(EditorManager::get()->getState()) {
    case ED_STATE_NORMAL:
    case ED_STATE_SELECT:
        if(eventId == ON_BUTTON_INPUT) {
            normalStateHandleKey((InputData*)data);
        }
        break;
    case ED_STATE_LOAD_FILE:
    case ED_STATE_SAVE_FILE:
        if(eventId == ON_BUTTON_INPUT) {
            enterTextHandleKey((InputData*)data);
        }
        break;
    default:
        break;
    }
}

void
EditorObject::prepState(EditorState eState) {
    switch(eState) {
    case ED_STATE_NORMAL:
        m_pRenderModel->setVolume(Box(m_ptTilePos.x, m_ptTilePos.y, m_ptTilePos.z, TILE_SIZE, TILE_SIZE, TILE_SIZE));
        m_pRenderModel->setColor(Color(0x0,0x0,0xFF));
        break;
    case ED_STATE_SELECT:
        m_pRenderModel->setColor(Color(0xFF,0x0,0x0));
        m_ptInitSelectPos = m_ptTilePos;
        break;
    default:
        break;
    }
}


void
EditorObject::enterTextHandleKey(InputData *data) {
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
        m_sInput.append(1, '\n');
    }
    D3RE::get()->getHudElement(ED_TEXT)->updateText(m_sInput);
}

void EditorObject::normalStateHandleKey(InputData *data) {
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

void EditorObject::pitchStateHandleKey(InputData *data) {
    if(data->getInputState(IN_CTRL)) {
        if(data->getInputState(IN_NORTH)) {
            m_fDeltaPitch = M_PI / 100;
        } else if(data->getInputState(IN_SOUTH)) {
            m_fDeltaPitch = -M_PI / 100;
        } else {
            m_fDeltaPitch = 0.f;
        }
        m_ptDeltaPos.z = 0;

        m_fDeltaZoom = 0.f;
    } else if(data->getInputState(IN_SHIFT)) {
        if(data->getInputState(IN_NORTH)) {
            m_fDeltaZoom = -1.0f;
        } else if(data->getInputState(IN_SOUTH)) {
            m_fDeltaZoom = 1.0f;
        } else {
            m_fDeltaZoom = 0.f;
        }
        m_ptDeltaPos.z = 0;

        m_fDeltaPitch = 0.f;
    } else {
        if(data->getInputState(IN_NORTH)) {
            m_ptDeltaPos.z = -1;
        } else if(data->getInputState(IN_SOUTH)) {
            m_ptDeltaPos.z = 1;
        } else {
            m_ptDeltaPos.z = 0;
        }
        m_fDeltaZoom = m_fDeltaPitch = 0.f;
    }

    if(data->getInputState(IN_WEST)) {
        m_ptDeltaPos.x = -1;
    } else if(data->getInputState(IN_EAST)) {
        m_ptDeltaPos.x = 1;
    } else {
        m_ptDeltaPos.x = 0;
    }
}


Point
EditorObject::toTile(const Point &pt) {
    Point res = Point(
        ((int)pt.x) / TILE_SIZE,
        ((int)pt.y) / TILE_SIZE,
        ((int)pt.z) / TILE_SIZE
    );
    res *= TILE_SIZE;
    return res;
}

