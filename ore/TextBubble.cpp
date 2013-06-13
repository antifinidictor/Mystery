/*
 * TextBubble.cpp
 */
#include "TextBubble.h"

void TextBubble::setState(int iState, void *data) {
    switch(iState) {
    case TB_NEXT_TEXT:
        if(nextLines()) {
            m_eState = TB_STATIC_TEXT;
        } else {
            m_eState = TB_INVISIBLE;
            setFlag(ORE_INVISIBLE, true);
        }
        break;
    case TB_PREV_TEXT:
        prevLines();
        m_eState = TB_STATIC_TEXT;
        break;
    case TB_STATIC_TEXT:
        m_eState = TB_STATIC_TEXT;
        m_pTextRM->updateText((char *)data);
        setFlag(ORE_INVISIBLE, false);
        break;
    default:
        m_eState = TB_INVISIBLE;
        setFlag(ORE_INVISIBLE, true);
    }
}


bool TextBubble::nextLines() {
    m_iTextStart = m_iTextEnd;
    for(int i = 0; i < m_iNumDisplayLines; ++i) {
        m_iTextEnd = TextRenderer::get()->getNextLine(m_pTextRM->getText(), m_iTextEnd);
    }
    
    if(m_iTextStart != m_iTextEnd) {
        m_pTextRM->updateTextRange(m_iTextStart, m_iTextEnd);
        return true;
    } else {
        return false;   //No more text found, we're done!
    }
}

bool TextBubble::prevLines() {
    m_iTextEnd = m_iTextStart;
    for(int i = 0; i < m_iNumDisplayLines; ++i) {
        m_iTextStart = TextRenderer::get()->getNextLine(m_pTextRM->getText(), m_iTextStart);
    }
    
    if(m_iTextStart != m_iTextEnd) {
        m_pTextRM->updateTextRange(m_iTextStart, m_iTextEnd);
        return true;
    } else {
        return false;   //No more text found, we're done!
    }
}
