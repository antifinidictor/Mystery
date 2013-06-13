/*
 * TextBubble
 * Opens a text bubble.  The text bubble accepts a text input, breaks up the
 * text into manageable portions, and handles clicks
 */
#ifndef TEXT_BUBBLE_H
#define TEXT_BUBBLE_H

#include "mge/defs.h"
#include "mge/GameObject.h"
#include "mge/Event.h"
#include "ore/OrderedRenderModel.h"
#include "tpe/TimePhysicsModel.h"
#include "game/CompositeRenderModel.h"
#include "game/TextRenderModel.h"
#include "game/Clickable.h"
#include "game/GameDefs.h"

class TextBubble : public Clickable {
public:
    //Constructor(s)/Destructor
    TextBubble(uint uiID, Image *pSpeechBubbleImg, const char *szText, EventHandler *pSource);
    virtual ~TextBubble();

    //General
    virtual bool update(uint time);

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }
    
    //Listener
	virtual void callBack(uint cID, void *data, EventID id);

protected:
    virtual Rect getResponseArea() { return m_pSpeechBubbleRM->getDrawArea(); }

private:
    int m_iTimer,
        m_iCurFrame,
        m_iNextFrame;
    int m_iEnd;
    int m_iNumLines;
    bool m_bDone;
    TimePhysicsModel   *m_pPhysicsModel;
    CompositeRenderModel *m_pRenderModel;
    OrderedRenderModel  *m_pSpeechBubbleRM;
    TextRenderModel     *m_pTextRM;
    
    //Helper methods
    void updateDisplay();
};

#endif
