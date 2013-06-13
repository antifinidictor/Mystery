/*
 * Button.cpp
 * Defines the button class
 */

#include "Button.h"
#include "game/gameDefs.h"
#include "ore/OrderedRenderEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "game/CompositeRenderModel.h"
#include "game/TextRenderModel.h"
#include "game/TextRenderer.h"

using namespace std;


Button::Button(uint uiID, Image *img, Point pos) :
        Clickable(uiID) {
    Box bxArea = Box(pos.x, pos.y, pos.z, img->w / img->m_iNumFramesW, img->h / img->m_iNumFramesH, 1);
    m_pButtonRenderModel = new OrderedRenderModel(img, bxArea, pos.z, ORE_LAYER_HIGH_FX);
    m_pPhysicsModel = new TimePhysicsModel(bxArea);
    m_pRenderModel = m_pButtonRenderModel;  //For a blank button, these are the same.
}

Button::Button(uint uiID, Image *img, Point pos, const char* text) :
        Clickable(uiID) {
    Box bxArea = Box(pos.x, pos.y, pos.z, img->w / img->m_iNumFramesW, img->h / img->m_iNumFramesH, 1);
    Rect textSize = TextRenderer::get()->getArea(text, 0.f, 0.f);
    Point textPos = bxCenter(bxArea) - Point(textSize.w / 2.f, textSize.l / 2.f, 0.f);
    m_pPhysicsModel = new TimePhysicsModel(bxArea);

    m_pButtonRenderModel = new OrderedRenderModel(img, bxArea, pos.z, ORE_LAYER_OBJECTS);
    TextRenderModel *pTextRenderModel = new TextRenderModel(text, textPos);

    CompositeRenderModel *pCM = new CompositeRenderModel();
    pCM->add(m_pButtonRenderModel);
    pCM->add(pTextRenderModel);
    m_pRenderModel = pCM;

    printf("Button \"%s\" has id %d\n", text, uiID);
}

Button::~Button() {
    delete m_pRenderModel;
    delete m_pPhysicsModel;
}

bool Button::update(uint time) {
    m_pButtonRenderModel->setFrameH(m_eState);
    return false;
}
