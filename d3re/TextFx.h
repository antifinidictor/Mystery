/*
 * TextFx.h
 * Defines a text-effect renderer that renders a string according to
 * certain parameters.
 */

#ifndef TEXT_FX_H
#define TEXT_FX_H

#include "mge/defs.h"
#include <string>

class Image;

class TextFx {
public:
    virtual ~TextFx();
    static void init() { m_pInstance = new TextFx(); }
    static TextFx *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    //Use these
    void render(const char *str, float x, float y, float size = 1.0);
    void setFont(uint uiFontId) { m_uiFontId = uiFontId; }
    Rect getArea(const char *str, float x, float y, float size = 1.0);
    int  getNextLine(const char *str, int start);
    float getLineHeight(float size = 1.0);

private:
    //Helper methods
    TextFx();
    int char2IndH(char c);
    int char2IndW(char c);
    float twMin(int iw);
    float twMax(int iw, int ih);
    float thMin(int ih);
    float thMax(int ih);
    float renderChar(Image *pFont, char c, float x, float y, float size);
    int setColor(const char *hexColor);

    //Members
    uint m_uiFontId;
    char m_aWidths[78];

    static TextFx *m_pInstance;
};

#endif
