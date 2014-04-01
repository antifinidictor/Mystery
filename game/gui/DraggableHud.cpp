#include "DraggableHud.h"
#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "bae/BasicAudioEngine.h"
#include "mge/ModularEngine.h"
#include "game/items/Inventory.h"

#define Y_HIDDEN (TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_SHOWN (0.F)
#define Y_SHOW_BOUND (3 * TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_HIDE_BOUND (-3 * TEXTURE_TILE_SIZE)

DraggableHud::DraggableHud(uint uiId)
    : Draggable(uiId,
                Rect(0, Y_HIDDEN, SCREEN_WIDTH, SCREEN_HEIGHT)
                )
{
    m_bHidden = true;
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);
    m_iAnimTimer = 0;
    m_iFrame = 0;
}

DraggableHud::~DraggableHud()
{
    //dtor
}

void
DraggableHud::onFollow(const Point &diff) {
    float fTop = m_pRenderModel->getDrawArea().y;
    Point ptShift;
    if(fTop + diff.y > Y_SHOWN) {
        ptShift.y = Y_SHOWN - fTop;
    } else if(fTop + diff.y < Y_HIDDEN) {
        ptShift.y = Y_HIDDEN - fTop;
    } else {
        ptShift.y = diff.y;
    }
    m_pPhysicsModel->moveBy(ptShift);
    m_pRenderModel->moveBy(ptShift);
}

void
DraggableHud::onStartDragging() {
    PWE::get()->setState(PWE_PAUSED);
    if(m_bHidden) {
        BAE::get()->playSound(AUD_POPUP);
    }
}

void
DraggableHud::onEndDragging() {
    float fTop = m_pRenderModel->getDrawArea().y;
    if(m_bHidden) {
        if(fTop > Y_SHOW_BOUND) {
            Point ptShift = Point(0.f, Y_SHOWN - fTop, 0.f);
            m_pPhysicsModel->moveBy(ptShift);
            m_pRenderModel->moveBy(ptShift);
            m_bHidden = false;
        } else {
            Point ptShift = Point(0.f, Y_HIDDEN - fTop, 0.f);
            m_pPhysicsModel->moveBy(ptShift);
            m_pRenderModel->moveBy(ptShift);
            PWE::get()->setState(PWE_RUNNING);
        }
    } else {
        if(fTop < Y_HIDE_BOUND) {
            Point ptShift = Point(0.f, Y_HIDDEN - fTop, 0.f);
            m_pPhysicsModel->moveBy(ptShift);
            m_pRenderModel->moveBy(ptShift);
            m_bHidden = true;
            PWE::get()->setState(PWE_RUNNING);
        } else {
            Point ptShift = Point(0.f, Y_SHOWN - fTop, 0.f);
            m_pPhysicsModel->moveBy(ptShift);
            m_pRenderModel->moveBy(ptShift);
        }
    }

    if(m_bHidden) {
        BAE::get()->playSound(AUD_POPDOWN);
    }
}

void
DraggableHud::updateItemAnimations(Inventory *inv) {
    if(m_iAnimTimer > 10) {
        m_iFrame = (m_iFrame + 1) % 8;
        m_iAnimTimer = 0;
        for(int i = 0; i < NUM_ELEMENT_ITEMS; ++i) {
            if(inv->getElement(i) != NULL) {
                uint index = i + MGHUD_ELEMENT_THUMBNAIL_START + 1;
                m_pRenderModel->get<D3HudRenderModel*>(index)->setFrameW(m_iFrame);
            }
        }
        for(int i = 0; i < NUM_SPELL_ITEMS; ++i) {
            if(inv->getSpell(i) != NULL) {
                uint index = i + MGHUD_ELEMENT_THUMBNAIL_START + ITEM_NUM_ELEMENTS;
                m_pRenderModel->get<D3HudRenderModel*>(index)->setFrameW(m_iFrame);
            }
        }
        for(int i = 0; i < NUM_GENERAL_ITEMS; ++i) {
            if(inv->getGeneral(i) != NULL) {
                uint index = i + MGHUD_ELEMENT_THUMBNAIL_START + ITEM_NUM_SPELLS;
                m_pRenderModel->get<D3HudRenderModel*>(index)->setFrameW(m_iFrame);
            }
        }

        //Update itembar images (which are set to itemid 0 if no item set)
        ContainerRenderModel *panel = D3RE::get()->getHudContainer()
            ->get<ContainerRenderModel*>(HUD_TOPBAR)
            ->get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER);
        panel->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT)->setFrameW(m_iFrame);
        panel->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL)->setFrameW(m_iFrame);
        panel->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM)->setFrameW(m_iFrame);
    } else {
        ++m_iAnimTimer;
    }
}
