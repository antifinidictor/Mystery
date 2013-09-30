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
    EditorHudButton(RenderModel *parent, uint uiEventId, const std::string &label, const Point &ptPos, float textSize = 1.f)
        : D3HudRenderModel(IMG_BUTTON,
                           Rect(ptPos.x, ptPos.y, BUTTON_WIDTH, BUTTON_HEIGHT),
                           label,
                           Point(5,5,0),
                           textSize)
    {
        m_uiId = PWE::get()->genID();
        m_uiEventId = uiEventId;
        m_pParent = parent;

        //Add myself to the MGE
        MGE::get()->addListener(this, ON_MOUSE_MOVE);
        MGE::get()->addListener(this, ON_BUTTON_INPUT);
    }

    virtual ~EditorHudButton() {
        MGE::get()->removeListener(m_uiId, ON_MOUSE_MOVE);
        MGE::get()->removeListener(m_uiId, ON_BUTTON_INPUT);
    }

    //From listener
    virtual uint getID() { return m_uiId; }
    virtual void callBack(uint cID, void *data, uint eventId) {
        switch(eventId) {
        case ON_BUTTON_INPUT:
        case ON_MOUSE_MOVE:
            handleMouseEvent((InputData*)data);
            break;
        default:
            break;
        }
    }

private:
    enum ButtonFrames {
        HUD_BUTTON_UP,
        HUD_BUTTON_SELECT,
        HUD_BUTTON_DOWN
    };

    void handleMouseEvent(InputData *data) {
        Point ptMouse = Point(data->getInputState(MIN_MOUSE_X), data->getInputState(MIN_MOUSE_Y), 0)
            - m_pParent->getPosition();
        if(ptInRect(ptMouse, getDrawArea())) {
            if(data->getInputState(IN_SELECT)) {
                setFrameH(HUD_BUTTON_DOWN);
            } else {
                setFrameH(HUD_BUTTON_SELECT);
                if(data->hasChanged(IN_SELECT)) {
                    m_sText = getText();
                    EditorManager::get()->callBack(m_uiId, &m_sText, m_uiEventId);
                }
            }
        } else {
            setFrameH(HUD_BUTTON_UP);
        }
    }

    RenderModel *m_pParent;

    uint m_uiId, m_uiEventId;
    std::string m_sText;
};

#endif //HUD_BUTTON_H
