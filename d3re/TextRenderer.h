/*
 * TextRenderer.h
 * Defines a basic text renderer.
 */

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "mge/defs.h"
#include <string>

class Image;

class TextRenderer {
public:
    virtual ~TextRenderer();
    static void init() { m_pInstance = new TextRenderer(); }
    static TextRenderer *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    //Use these
    void render(const char *str, float x, float y, float size = 1.0);
    char *splitText(const char *str, float maxw, float size = 1.0);
    void splitText(std::string &str, float maxw, float size = 1.0);
    void setFont(uint uiFontId) { m_uiFontId = uiFontId; }
    Rect getArea(const std::string &str, float x, float y, float size = 1.0);
    int  getNextLine(const char *str, int start);
    float getLineHeight(float size = 1.0);

private:
    //Helper methods
    TextRenderer();
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

    static TextRenderer *m_pInstance;
};

#endif
