/*
 * GuiButton
 * A render model that listens for input events, then calls the appropriate method
 */
#ifndef HUD_BUTTON_H
#define HUD_BUTTON_H

#include "d3re/d3re.h"
#include "editor/editor_defs.h"
#include "editor/EditorManager.h"
#include "mge/ModularEngine.h"

#define BUTTON_WIDTH 128
#define BUTTON_HEIGHT 32

class GuiButton : public D3HudRenderModel, public Listener {
public:
    GuiButton(Positionable *parent, Listener *pListener, uint uiEventId, const std::string &label, const Point &ptPos, float textSize = 1.f)
        :   D3HudRenderModel(
                IMG_BUTTON,
                Rect(ptPos.x, ptPos.y, BUTTON_WIDTH, BUTTON_HEIGHT),
                label,
                Point(0,0,0),
                textSize
            ),
            m_uiId(PWE::get()->genId()),    //For listener use
            m_uiEventId(uiEventId),
            m_uiHudId(s_uiHudId++),         //Ensures consecutive IDs for Editor use
            m_bDisabled(false),
            m_pListener(pListener),
            m_sText(label)
    {
//printf(__FILE__" %d: Button with text '%s' has id %d\n",__LINE__, label.c_str(), m_uiId);
        //Add myself to the MGE
        MGE::get()->addListener(this, ON_MOUSE_MOVE);
        MGE::get()->addListener(this, ON_BUTTON_INPUT);

        centerVertically(true);
        centerHorizontally(true);
    }

    virtual ~GuiButton() {
        MGE::get()->removeListener(m_uiId, ON_MOUSE_MOVE);
        MGE::get()->removeListener(m_uiId, ON_BUTTON_INPUT);
        PWE::get()->freeId(m_uiId);
    }

    uint getHudID() { return m_uiHudId; }

    //From listener
    virtual uint getId() { return m_uiId; }
    virtual int callBack(uint cID, void *data, uint eventId) {
        //Disable inactive buttons
        if(m_bDisabled) {
            return EVENT_DROPPED;
        }

        bool bAlwaysDrop = false;
        switch(eventId) {
        case ON_MOUSE_MOVE:
            bAlwaysDrop = true;
        case ON_BUTTON_INPUT:
            return handleMouseEvent((InputData*)data, bAlwaysDrop);
        default:
            return EVENT_DROPPED;
        }
    }

    void enable() { m_bDisabled = false; }
    void disable() { m_bDisabled = true; }

private:
    enum ButtonFrames {
        HUD_BUTTON_UP,
        HUD_BUTTON_SELECT,
        HUD_BUTTON_DOWN
    };

    int handleMouseEvent(InputData *data, bool bAlwaysDrop) {
        //Get the parent of the HUD-Rmdl. The HUD-Rmdl's position takes into
        // account both its parent and its draw area, which we don't want since
        // we are using the raw draw area.
        Point ptPos = m_pParent->getPosition();
        Rect rcDrawArea = getDrawArea();
        Point ptMouse = Point(
            data->getInputState(MIN_MOUSE_X) - ptPos.x,
            data->getInputState(MIN_MOUSE_Y) - ptPos.y,
            0
        );
        int status = EVENT_DROPPED;
        if(ptInRect(ptMouse, rcDrawArea)) {
            if(data->getInputState(IN_SELECT)) {
                setFrameH(HUD_BUTTON_DOWN);
                if(!bAlwaysDrop) {
                    status = EVENT_CAUGHT;
                }
            } else {
                setFrameH(HUD_BUTTON_SELECT);
                if(data->hasChanged(IN_SELECT)) {
                    m_sText = getText();
                    m_pListener->callBack(m_uiHudId, &m_sText, m_uiEventId);
                    if(!bAlwaysDrop) {
                        status = EVENT_CAUGHT;
                    }
                }
            }
        } else {
            setFrameH(HUD_BUTTON_UP);
        }
        return status;
    }

    uint m_uiId, m_uiEventId, m_uiHudId;
    bool m_bDisabled;
    Listener *m_pListener;
    static uint s_uiHudId;  //Defined in GameManager
    std::string m_sText;
};

#endif //HUD_BUTTON_H
