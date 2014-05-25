#include "TextDisplay.h"
#include "d3re/d3re.h"
#include <stdlib.h>
using namespace std;


TextDisplay *TextDisplay::m_pInstance = NULL;

TextDisplay::TextDisplay()
{
    m_uiNextTextId = 0;
}

TextDisplay::~TextDisplay()
{
    //dtor
}


void
TextDisplay::registerText(const std::string &text, OnTextDoneCallback cb, float fRate) {
    ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_BOTTOMBAR);
    Rect rcPanelArea = panel->getDrawArea();
    Rect rcTextArea = TextRenderer::get()->getArea(text.c_str(), rcPanelArea.w, rcPanelArea.h / 2);
    rcTextArea.y -= rcTextArea.h / 2 + fabs((float)(rand() % 10)) * 5;
    D3HudRenderModel *hmdl = new D3HudRenderModel(text, rcTextArea);

    //
    m_lsTexts.push_back(TextInfo(m_uiNextTextId, cb, fRate));

    panel->add(m_uiNextTextId++, hmdl);
}

void
TextDisplay::update(float fDeltaTime) {
    //Make the text scroll
    ContainerRenderModel *panel = D3RE::get()->getHudContainer()->get<ContainerRenderModel*>(HUD_BOTTOMBAR);
    list<TextInfo>::iterator it;

    for(it = m_lsTexts.begin(); it != m_lsTexts.end(); ) {
        D3HudRenderModel *rm = panel->get<D3HudRenderModel*>(it->m_uiHudId);
        Point pos = rm->getPosition();
        if(rm->getPosition().x + rm->getDrawArea().w < 0) {
            //Callback
            it->m_cb(it->m_uiHudId);

            //Remove the render model
            panel->remove(it->m_uiHudId);
            delete rm;

            //Remove the text info and go to the next list element
            it = m_lsTexts.erase(it);
        } else {
            //Move the text
            rm->moveBy(Point(-it->m_fRate, 0.f, 0.f));

            //Go to the next element in the list
            it++;
        }
    }
}
