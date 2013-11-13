#ifndef TEXTDISPLAY_H
#define TEXTDISPLAY_H

#include <string>
#include <list>
#include "game/game_defs.h"

#define DEFAULT_RATE 1.0f

typedef void (*OnTextDoneCallback)(uint textId);

class ContainerRenderModel;

class TextDisplay
{
public:
    static void init() { m_pInstance = new TextDisplay(); }
    static void clean() { delete m_pInstance; }
    static TextDisplay *get() { return m_pInstance; }

    void registerText(const std::string &text, OnTextDoneCallback cb, float fRate = DEFAULT_RATE);
    void update(uint time);

private:
    TextDisplay();
    virtual ~TextDisplay();

    static TextDisplay *m_pInstance;

    struct TextInfo {
        float m_fRate;
        OnTextDoneCallback m_cb;
        uint m_uiHudId;

        TextInfo() {
            m_uiHudId = 0;
            m_cb = NULL;
            m_fRate = 0.f;
        }

        TextInfo(uint uiHudId, OnTextDoneCallback cb, float fRate) {
            m_uiHudId = uiHudId;
            m_cb = cb;
            m_fRate = fRate;
        }
    };
    uint m_uiNextTextId;

    std::list<TextInfo> m_lsTexts;
};

#endif // TEXTDISPLAY_H
