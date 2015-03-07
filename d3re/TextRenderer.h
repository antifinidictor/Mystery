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
    struct CharacterFilter {
        virtual ~CharacterFilter() {}
        virtual float getSize(int index, int adjustedIndex, int line) = 0;
        virtual float getOffsetSize(int index, int adjustedIndex, int line) = 0;
    };

    struct BasicCharacterFilter : public CharacterFilter {
        float m_fTextSize;
        BasicCharacterFilter(float fTextSize)
            : m_fTextSize(fTextSize)
        {
        }

        virtual ~BasicCharacterFilter() {}

        virtual float getSize(int index, int adjustedIndex, int line) {
            return m_fTextSize;
        }

        virtual float getOffsetSize(int index, int adjustedIndex, int line) {
            return m_fTextSize;
        }
    };

    virtual ~TextRenderer();
    static void init() { m_pInstance = new TextRenderer(); }
    static TextRenderer *get() { return m_pInstance; }
    static void clean() { delete m_pInstance; }

    //Use these
    void render(const char *str, float x, float y, float charSize = 1.0);
    void render(const char *str, float x, float y, CharacterFilter *filter);
    void splitText(std::string &str, float maxw, float charSize = 1.0);
    void splitText(std::string &str, float maxw, CharacterFilter *filter);
    void setFont(uint uiFontId) { m_uiFontId = uiFontId; }
    Rect getArea(const std::string &str, float x, float y, float charSize = 1.0);
    Rect getArea(const std::string &str, float x, float y, CharacterFilter *filter);
    int  getNextLine(const char *str, int start);
    float getLineHeight(float charSize = 1.0);


private:
    //Helper methods
    TextRenderer();
    int char2IndH(char c);
    int char2IndW(char c);
    float twMin(int iw);
    float twMax(int iw, int ih);
    float thMin(int ih);
    float thMax(int ih);
    float renderChar(Image *pFont, char c, float x, float y, float charSize, float offsetSize);
    int setColor(const char *hexColor);

    //Members
    uint m_uiFontId;
    char m_aWidths[78];

    static TextRenderer *m_pInstance;
};

#endif
