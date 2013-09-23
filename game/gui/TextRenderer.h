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
    void render(const char *str, double x, double y);
    char *splitText(const char *str, double maxw);
    void splitText(std::string &str, double maxw);
    Rect getArea(const char *str, double x, double y);

private:
    //Helper methods
    TextRenderer();
    int char2IndH(char c);
    int char2IndW(char c);
    double twMin(int iw);
    double twMax(int iw, int ih);
    double thMin(int ih);
    double thMax(int ih);
    double renderChar(char c, double x, double y);
    int setColor(const char *hexColor);

    //Members
    Image *m_pFont;
    char m_aWidths[78];

    static TextRenderer *m_pInstance;
};

#endif
