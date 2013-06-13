/*
 * TextRenderModel
 * A render model... for text!
 */

#ifndef TEXT_RENDER_MODEL_H
#define TEXT_RENDER_MODEL_H

#include "game/TextRenderer.h"

class TextRenderModel : public RenderModel {
public:
    TextRenderModel(const char *szText, Point ptPos, float size = 1.0f) {
        m_szText = (char *)malloc(sizeof(char)*strlen(szText));
        strcpy(m_szText,szText);
        m_ptPosition = ptPos;
        m_fSize = size;
        m_iStart = 0;
        m_iEnd = strlen(m_szText);
    }

    virtual ~TextRenderModel() {
        free(m_szText);
    }

    virtual void render(RenderEngine *re) {
        Point ptPos = m_ptPosition - re->getRenderOffset();
        TextRenderer::get()->render(m_szText, ptPos.x, ptPos.y, m_iStart, m_iEnd, m_fSize);
    }

    virtual void moveBy(Point ptShift) {
        m_ptPosition += ptShift;
    }

    virtual Point getPosition() {
        return m_ptPosition;
    }

    virtual Rect getDrawArea() {
        return TextRenderer::get()->getArea(m_szText, m_ptPosition.x, m_ptPosition.y);
    }

    void updateText(const char *szText) {
        free(m_szText);
        m_szText = (char *)malloc(sizeof(char)*strlen(szText));
        strcpy(m_szText,szText);
    }
    
    void updateTextRange(int iStart, int iEnd) {
        m_iStart = iStart;
        m_iEnd = iEnd;
    }
    
    char *getText() { return m_szText; }

private:
    Point m_ptPosition;
    char *m_szText;
    float m_fSize;
    int m_iStart, m_iEnd;
};

#endif
