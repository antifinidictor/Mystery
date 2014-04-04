#include "DraggableHud.h"
#include "DraggableItem.h"
#include "DraggableElementalSpellItem.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "bae/BasicAudioEngine.h"
#include "mge/ModularEngine.h"
#include "game/items/Inventory.h"
#include "game/items/Item.h"
#include "mge/Event.h"

#define Y_HIDDEN (TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_SHOWN (0.F)
#define Y_SHOW_BOUND (3 * TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_HIDE_BOUND (-3 * TEXTURE_TILE_SIZE)

#define ANIM_TIMER_MAX 1

using namespace std;

DraggableHud::DraggableHud(uint uiId)
    : Draggable(uiId,
                Rect(0, Y_HIDDEN, SCREEN_WIDTH, SCREEN_HEIGHT)
                )
{
    m_bHidden = true;
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);
    m_pRenderModel = new ContainerRenderModel(getClickArea());
    m_iAnimTimer = 0;
    m_iFrame = 0;

    m_pMyPlayer = NULL;

    //Fill the actual hud
    initPlayerHud();

    //Fill the inventory with valid item points
    Point ptValid;
    for(ptValid.y = 240; ptValid.y <= 368; ptValid.y += 64) {
        for(ptValid.x = 80; ptValid.x <= 400; ptValid.x += 64) {
                DraggableItem::addValidDropLocation(ptValid);
                printf("Valid location (%f,%f)\n", ptValid.x, ptValid.y);
        }
    }

    //The element/spell drop-location is the same as the item drop-location,
    // which is available at the drop point
    DraggableElementalSpellItem::setDropPoint(ptValid);

    //The current item location
    ptValid = Point(592, 464, 0);
    DraggableItem::addValidDropLocation(ptValid);

    //The current spell/element location
    ptValid = Point(528, 464, 0);
    DraggableElementalSpellItem::setMakeCurrentPoint(ptValid);
}

DraggableHud::~DraggableHud() {
printf(__FILE__" %d\n",__LINE__);
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
printf(__FILE__" %d\n",__LINE__);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);
printf(__FILE__" %d\n",__LINE__);

    delete m_pRenderModel;
printf(__FILE__" %d\n",__LINE__);
}

/*
 * Draggable
 */
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
    moveBy(ptShift);
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
            moveBy(ptShift);
            m_bHidden = false;
        } else {
            Point ptShift = Point(0.f, Y_HIDDEN - fTop, 0.f);
            moveBy(ptShift);
            PWE::get()->setState(PWE_RUNNING);
        }
    } else {
        if(fTop < Y_HIDE_BOUND) {
            Point ptShift = Point(0.f, Y_HIDDEN - fTop, 0.f);
            moveBy(ptShift);
            m_bHidden = true;
            PWE::get()->setState(PWE_RUNNING);
        } else {
            Point ptShift = Point(0.f, Y_SHOWN - fTop, 0.f);
            moveBy(ptShift);
        }
    }

    if(m_bHidden) {
        BAE::get()->playSound(AUD_POPDOWN);
    }
}


/*
 * Item animations
 */
class ItemAnimUpdateFunctor {
public:
    ItemAnimUpdateFunctor(uint uiFrameW) {
        m_uiFrameW = uiFrameW;
    }

    bool operator()(uint itemIndex, RenderModel *rmdl) {
        D3HudRenderModel *hudMdl = dynamic_cast<D3HudRenderModel*>(rmdl);
        if(hudMdl != NULL) {
            hudMdl->setFrameW(m_uiFrameW);
        }
        return false;   //Don't stop iterating
    }

private:
    uint m_uiFrameW;
};

void
DraggableHud::updateItemAnimations() {
    if(m_iAnimTimer > ANIM_TIMER_MAX) {
        m_iFrame = (m_iFrame + 1) % 8;
        m_iAnimTimer = 0;

        ContainerRenderModel *panel;
        ItemAnimUpdateFunctor curFrameFunctor(m_iFrame);

        //Update general item animations
        panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
        panel->forEachModel(curFrameFunctor);

        //Update element animations
        panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
        panel->forEachModel(curFrameFunctor);

        //Update spell animations
        panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
        panel->forEachModel(curFrameFunctor);

        //Update itembar images (which are set to itemid 0 if no item set)
        panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER);
        panel->forEachModel(curFrameFunctor);
    } else {
        ++m_iAnimTimer;
    }
}

void
DraggableHud::moveBy(const Point &ptShift) {
    Draggable::onFollow(ptShift);
    m_pRenderModel->moveBy(ptShift);
}

void
DraggableHud::addItem(uint itemId, uint invIndex) {
    //x/y position of the item on the screen
    float x = (2 + 2 * (invIndex % 6)) * TEXTURE_TILE_SIZE;
    float y = (7 + 2 * (invIndex / 6)) * TEXTURE_TILE_SIZE;

    Rect rcArea = Rect(x, y, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    DraggableItem *pItem = new DraggableItem(PWE::get()->genId(), itemId, invIndex, rcArea, m_pMyPlayer);
    m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)
        ->add(invIndex, pItem->getRenderModel());
}

void
DraggableHud::addSpell(uint itemId, uint invIndex) {
    float x = (10) * TEXTURE_TILE_SIZE;
    float y = (2 + 2 * (invIndex % 2)) * TEXTURE_TILE_SIZE;

    Rect rcArea = Rect(x, y, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    DraggableElementalSpellItem *pItem =
        new DraggableElementalSpellItem(PWE::get()->genId(), itemId, invIndex, rcArea, m_pMyPlayer);
    m_pRenderModel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER)
        ->add(invIndex, pItem->getRenderModel());
}

void
DraggableHud::addElement(uint itemId, uint invIndex) {
printf(__FILE__" %d\n",__LINE__);
    float x = (2 + 2 * (invIndex % 2)) * TEXTURE_TILE_SIZE;
    float y = (2 + 2 * (invIndex / 2)) * TEXTURE_TILE_SIZE;

    Rect rcArea = Rect(x, y, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    DraggableElementalSpellItem *pItem =
        new DraggableElementalSpellItem(PWE::get()->genId(), itemId, invIndex, rcArea, m_pMyPlayer);
    m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER)
        ->add(invIndex, pItem->getRenderModel());
}

void
DraggableHud::removeItem(uint invIndex) {
    ContainerRenderModel *panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        panel->remove(invIndex);
        delete item;
    }
}

void
DraggableHud::removeSpell(uint invIndex) {
    ContainerRenderModel *panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        panel->remove(invIndex);
        delete item;
    }
}

void
DraggableHud::removeElement(uint invIndex) {
    ContainerRenderModel *panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        panel->remove(invIndex);
        delete item;
    }
}

void
DraggableHud::setCurrentItem(uint itemId) {
    m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
        ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM)
        ->setFrameH(itemId);
}

void
DraggableHud::setCurrentSpell(uint itemId) {
    m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
        ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL)
        ->setFrameH(itemId);
}

void
DraggableHud::setCurrentElement(uint itemId) {
    m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
        ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT)
        ->setFrameH(itemId);
}

void
DraggableHud::moveItem(uint startIndex, uint endIndex) {
    ContainerRenderModel *panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(startIndex);
    if(item != NULL) {
        panel->remove(startIndex);
        panel->add(endIndex, item);
    }
}

void
DraggableHud::moveSpell(uint startIndex, uint endIndex) {
    ContainerRenderModel *panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(startIndex);
    if(item != NULL) {
        panel->remove(startIndex);
        panel->add(endIndex, item);
    }
}

void
DraggableHud::moveElement(uint startIndex, uint endIndex) {
    ContainerRenderModel *panel = m_pRenderModel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(startIndex);
    if(item != NULL) {
        panel->remove(startIndex);
        panel->add(endIndex, item);
    }
}

void
DraggableHud::initPlayerHud() {
    //const uint hudBackdropId = D3RE::get()->getImageId("hudBackdrop");
    ContainerRenderModel *panel = D3RE::get()->getHudContainer();
    panel->add(HUD_TOPBAR, m_pRenderModel);

    panel = m_pRenderModel;

    uint uiHudBackdropImgId = D3RE::get()->getImageId("guiBackdrop");
    Rect rcHudBackdropArea = Rect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT);
    D3HudRenderModel *pBackground = new D3HudRenderModel(uiHudBackdropImgId, rcHudBackdropArea);
    m_pRenderModel->add(MGHUD_BACKDROP, pBackground);

    Rect rcHealthPanel = Rect(
        TEXTURE_TILE_SIZE,
        SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE
    );
    Rect rcInventoryPanel = Rect(
        0,  //TEXTURE_TILE_SIZE * 8,
        0,  //SCREEN_HEIGHT,
        TEXTURE_TILE_SIZE * 10,
        TEXTURE_TILE_SIZE
    );
    Rect rcItembarPanel = Rect(
        TEXTURE_TILE_SIZE * 16,
        SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE
    );
    ContainerRenderModel *healthPanel = new ContainerRenderModel(rcHealthPanel);
    ContainerRenderModel *spellPanel = new ContainerRenderModel(rcInventoryPanel);
    ContainerRenderModel *itemPanel = new ContainerRenderModel(rcInventoryPanel);
    ContainerRenderModel *elementPanel = new ContainerRenderModel(rcInventoryPanel);
    ContainerRenderModel *itembarPanel = new ContainerRenderModel(rcItembarPanel);
    //rightEdge->setFrameH(2);
    //middle->setFrameH(1);
    //middle->setRepsW((SCREEN_WIDTH) / TEXTURE_TILE_SIZE - 2);

    //panel->add(MGHUD_LEFT_EDGE, leftEdge);
    //panel->add(MGHUD_MIDDLE, middle);
    //panel->add(MGHUD_RIGHT_EDGE, rightEdge);
    panel->add(MGHUD_HEALTH_CONTAINER, healthPanel);
    panel->add(MGHUD_SPELL_CONTAINER, spellPanel);
    panel->add(MGHUD_ITEM_CONTAINER, itemPanel);
    panel->add(MGHUD_ELEMENT_CONTAINER, elementPanel);
    panel->add(MGHUD_ITEMBAR_CONTAINER, itembarPanel);

    /*
     * Area name and current-action label
     */
#define TEXT_BOX_WIDTH (TEXTURE_TILE_SIZE * 9)
    Rect rcAreaLabel = Rect(
        rcHealthPanel.x + rcHealthPanel.w,
        5.F + SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXT_BOX_WIDTH,
        TEXTURE_TILE_SIZE
    );
    Rect rcActionLabel = Rect(
        rcAreaLabel.x + rcAreaLabel.w,
        7.F + SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE - 12
    );
    D3HudRenderModel *label = new D3HudRenderModel("area", rcAreaLabel,1.0f);
    label->centerHorizontally(true);
    panel->add(MGHUD_CUR_AREA, label);

    label = new D3HudRenderModel("action", rcActionLabel,0.8f);
    label->centerHorizontally(true);
    label->centerVertically(true);
    panel->add(MGHUD_CUR_ACTION, label);

    /*
     * Health bar panel
     */
    initHealthBarHud(healthPanel);

    /*
     * Itembar panel
     */
    initItemBarHud(itembarPanel);
}

void
DraggableHud::initHealthBarHud(ContainerRenderModel *panel) {
    #define BAR_SIZE (TEXTURE_TILE_SIZE / 2.F)
    #define BAR_WIDTH  (TEXTURE_TILE_SIZE * 3.F)
    #define BAR_X (0.f)
    #define BAR_Y (BAR_SIZE / 2.F)
    uint barId = D3RE::get()->getImageId("hudbar");
    D3HudRenderModel *leftEdge = new D3HudRenderModel(barId, Rect(BAR_X,BAR_Y,BAR_SIZE,BAR_SIZE));
    D3HudRenderModel *middle = new D3HudRenderModel(barId, Rect(BAR_X + BAR_SIZE,BAR_Y,BAR_WIDTH - BAR_SIZE * 2,BAR_SIZE));
    D3HudRenderModel *rightEdge = new D3HudRenderModel(barId, Rect(BAR_X + BAR_WIDTH - BAR_SIZE,BAR_Y,BAR_SIZE,BAR_SIZE));

    D3HudRenderModel *label = new D3HudRenderModel("99", Rect(BAR_X+BAR_WIDTH/2.f - 10.f,BAR_Y,20.f,BAR_SIZE),0.8f);
    D3HudRenderModel *bar = new D3HudRenderModel(barId, Rect(BAR_X+1,BAR_Y,BAR_WIDTH-2,BAR_SIZE));

    rightEdge->setFrameH(2);
    middle->setFrameH(1);
    middle->setRepsW((BAR_WIDTH) / BAR_SIZE - 2);
    bar->setFrameH(3);
    bar->setImageColor(Color(0xFF,0x0,0x0));

    panel->add(MGHUD_HEALTH_BACKDROP_LEFT_EDGE, leftEdge);
    panel->add(MGHUD_HEALTH_BACKDROP_MIDDLE, middle);
    panel->add(MGHUD_HEALTH_BACKDROP_RIGHT_EDGE, rightEdge);
    panel->add(MGHUD_HEALTH_BAR, bar);
    panel->add(MGHUD_HEALTH_VALUE, label);
}

void
DraggableHud::initItemBarHud(ContainerRenderModel *panel) {
    Rect rcElementArea = Rect(0, 0, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *curElementThumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcElementArea);

    Rect rcSpellArea = Rect(0, 0, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *curSpellThumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcSpellArea);

    Rect rcItemArea = Rect(TEXTURE_TILE_SIZE * 2, 0, TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE);
    D3HudRenderModel *curItemThumbnail = new D3HudRenderModel(D3RE::get()->getImageId("items"), rcItemArea);

    panel->add(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT, curElementThumbnail);
    panel->add(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL, curSpellThumbnail);
    panel->add(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM, curItemThumbnail);
}
