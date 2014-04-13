/*
 * EditorHudButton
 * A render model that listens for input events, then calls the appropriate method
 */
#ifndef HUD_BUTTON_H
#define HUD_BUTTON_H

#include "d3re/d3re.h"
#include "editor/editor_defs.h"
#include "editor/EditorManager.h"
#include "mge/ModularEngine.h"

class EditorHudButton : public D3HudRenderModel, public Listener {
public:
    EditorHudButton(Positionable *parent, uint uiEventId, const std::string &label, const Point &ptPos, float textSize = 1.f)
        : D3HudRenderModel(IMG_BUTTON,
                           Rect(ptPos.x, ptPos.y, BUTTON_WIDTH, BUTTON_HEIGHT),
                           label,
                           Point(5,5,0),
                           textSize)
    {
        m_uiId = PWE::get()->genId();   //For listener use
        m_uiHudId = s_uiHudId++;    //Ensures consecutive IDs for Editor use
        m_uiEventId = uiEventId;
        m_pParent = parent;

        //Add myself to the MGE
        MGE::get()->addListener(this, ON_MOUSE_MOVE);
        MGE::get()->addListener(this, ON_BUTTON_INPUT);
    }

    virtual ~EditorHudButton() {
        MGE::get()->removeListener(m_uiId, ON_MOUSE_MOVE);
        MGE::get()->removeListener(m_uiId, ON_BUTTON_INPUT);
        PWE::get()->freeId(m_uiId);
    }

    uint getHudID() { return m_uiHudId; }

    //From listener
    virtual uint getId() { return m_uiId; }
    virtual int callBack(uint cID, void *data, uint eventId) {
        switch(eventId) {
        case ON_BUTTON_INPUT:
        case ON_MOUSE_MOVE:
            return handleMouseEvent((InputData*)data);
        default:
            return EVENT_DROPPED;
        }
    }

private:
    enum ButtonFrames {
        HUD_BUTTON_UP,
        HUD_BUTTON_SELECT,
        HUD_BUTTON_DOWN
    };

    int handleMouseEvent(InputData *data) {
        Point ptMouse = Point(data->getInputState(MIN_MOUSE_X), data->getInputState(MIN_MOUSE_Y), 0)
            - m_pParent->getPosition();
        int status = EVENT_DROPPED;
        if(ptInRect(ptMouse, getDrawArea())) {
            status = EVENT_CAUGHT;
            if(data->getInputState(IN_SELECT)) {
                setFrameH(HUD_BUTTON_DOWN);
            } else {
                setFrameH(HUD_BUTTON_SELECT);
                if(data->hasChanged(IN_SELECT)) {
                    m_sText = getText();
                    EditorManager::get()->callBack(m_uiHudId, &m_sText, m_uiEventId);
                }
            }
        } else {
            setFrameH(HUD_BUTTON_UP);
        }
        return status;
    }

    Positionable *m_pParent;

    uint m_uiId, m_uiEventId, m_uiHudId;
    static uint s_uiHudId;
    std::string m_sText;
};

#endif //HUD_BUTTON_H
