/*
 * TextBubble.cpp
 */

#include "TextBubble.h"
#include "ore/OrderedRenderEngine.h"
#include "pwe/PartitionedWorldEngine.h"

#define MARGIN_SIZE 5
#define TIMER_LENGTH 60

#define BUBBLE_BLANK 0
#define BUBBLE_NEXT  1
#define BUBBLE_CLOSE 2

#define TEXT_SIZE 0.6f


TextBubble::TextBubble(uint uiID, Image *pSpeechBubbleImg, const char *szText, EventHandler *pSource) :
        Clickable(uiID) {
    m_iTimer = TIMER_LENGTH;
    m_iCurFrame = 0;
    int iBubbleW = pSpeechBubbleImg->w / pSpeechBubbleImg->m_iNumFramesW,
        iBubbleH = pSpeechBubbleImg->h / pSpeechBubbleImg->m_iNumFramesH;
    int iMaxW = iBubbleW - MARGIN_SIZE * 2,
        iMaxH = iBubbleH - MARGIN_SIZE * 2;
    m_bDone = false;
    m_iNumLines = iMaxH / TextRenderer::get()->getLineHeight(TEXT_SIZE);

    //Divide up text
    char *szDivText = TextRenderer::get()->splitText(szText, iMaxW, TEXT_SIZE);

    //Create models
    Rect rcArea(0, 0, iBubbleW, iBubbleH);
    m_pPhysicsModel = new TimePhysicsModel(rcArea);
    m_pRenderModel = new CompositeRenderModel();
    m_pSpeechBubbleRM = new OrderedRenderModel(pSpeechBubbleImg, rcArea, 0.f, ORE_LAYER_HIGH_FX);
    m_pTextRM = new TextRenderModel(szDivText, Point(rcArea) + Point(MARGIN_SIZE,MARGIN_SIZE,0), TEXT_SIZE);
    m_pRenderModel->add(m_pSpeechBubbleRM);
    m_pRenderModel->add(m_pTextRM);

    m_iEnd = 0;

    updateDisplay();
    /*
    TextRenderer::get()->getNextLine(szDivText, 0);
    m_pTextRM->updateTextRange(0, m_iEnd);
    */
    if(TextRenderer::get()->getNextLine(szDivText, m_iEnd) == m_iEnd) {
        m_iNextFrame = BUBBLE_CLOSE;
    } else {
        m_iNextFrame = BUBBLE_NEXT;
    }

    //add listener
    addListener(this, ON_ACTIVATE);
    printf("Text bubble \"%s\" has id %d\n", szText, uiID);

    free(szDivText);
}

TextBubble::~TextBubble() {
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

bool TextBubble::update(uint time) {
    if(--m_iTimer < 0) {
        m_iCurFrame = (m_iCurFrame == BUBBLE_BLANK) ? m_iNextFrame : BUBBLE_BLANK;
        m_pSpeechBubbleRM->setFrameH(m_iCurFrame);
        m_iTimer = TIMER_LENGTH;

    }
    return m_bDone;
}

void TextBubble::callBack(uint cID, void *data, EventID id) {
    //int iNextEnd;
    switch(id) {
    case ON_ACTIVATE:
    /*
        iNextEnd = TextRenderer::get()->getNextLine(m_pTextRM->getText(), m_iEnd);
        if(iNextEnd != m_iEnd) {
            m_pTextRM->updateTextRange(m_iEnd, iNextEnd);
            m_iEnd = iNextEnd;
        } else {
            m_bDone = true; //kill the speech bubble
        }
    */
        updateDisplay();
        if(m_iEnd == TextRenderer::get()->getNextLine(m_pTextRM->getText(), m_iEnd)) {
            m_iNextFrame = BUBBLE_CLOSE;
            m_iCurFrame = BUBBLE_CLOSE;
            m_pSpeechBubbleRM->setFrameH(m_iCurFrame);
        }

        break;
    case ON_SELECT:
    case ON_DESELECT:
        break;
    default:
        Clickable::callBack(cID, data, id);
        break;
    }
}

void TextBubble::updateDisplay() {
    int iNextEnd = m_iEnd;
    for(int i = 0; i < m_iNumLines; ++i) {
        iNextEnd = TextRenderer::get()->getNextLine(m_pTextRM->getText(), iNextEnd);
    }    
    if(iNextEnd != m_iEnd) {
        m_pTextRM->updateTextRange(m_iEnd, iNextEnd);
        m_iEnd = iNextEnd;
    } else {
        m_bDone = true; //kill the speech bubble
    }
}

