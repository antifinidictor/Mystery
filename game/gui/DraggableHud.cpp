#include "DraggableHud.h"
#include "DraggableItem.h"
#include "DraggableElementalSpellItem.h"

#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include "bae/BasicAudioEngine.h"
#include "mge/ModularEngine.h"
#include "game/items/Item.h"
#include "game/items/SpellItem.h"
#include "mge/Event.h"

#define Y_HIDDEN (TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_SHOWN (0.F)
#define Y_SHOW_BOUND (3 * TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_HIDE_BOUND (-3 * TEXTURE_TILE_SIZE)

#define ANIM_TIMER_MAX 1

using namespace std;

DraggableHud::DraggableHud(uint uiId)
    : ContainerRenderModel(Rect(0, Y_HIDDEN, SCREEN_WIDTH, SCREEN_HEIGHT)),
      Draggable(this, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT))
{
    m_uiId = uiId;

    m_bHidden = true;
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);
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

    m_pCurItem = NULL;
    m_pCurElement = NULL;
    m_pCurSpell = NULL;
}

DraggableHud::~DraggableHud() {
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);

    //Render model should be deleted by the render engine
    //delete m_pRenderModel;
}

/*
 * Draggable
 */
void
DraggableHud::onFollow(const Point &diff) {
    float fTop = getDrawArea().y;
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
    float fTop = getDrawArea().y;
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
    //Remove any items scheduled for removal
    removeScheduledItems();

    if(m_iAnimTimer > ANIM_TIMER_MAX) {
        m_iFrame = (m_iFrame + 1) % 8;
        m_iAnimTimer = 0;

        ContainerRenderModel *panel;
        ItemAnimUpdateFunctor curFrameFunctor(m_iFrame);

        //Update general item animations
        panel = get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
        panel->forEachModel(curFrameFunctor);

        //Update element animations
        panel = get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
        panel->forEachModel(curFrameFunctor);

        //Update spell animations
        panel = get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
        panel->forEachModel(curFrameFunctor);

        //Update itembar images (which are set to itemid 0 if no item set)
        panel = get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER);
        panel->forEachModel(curFrameFunctor);
    } else {
        ++m_iAnimTimer;
    }
}

void
DraggableHud::moveBy(const Point &ptShift) {
    ContainerRenderModel::moveBy(ptShift);
}

class ItemFindEmptySlotFunctor {
public:
    ItemFindEmptySlotFunctor() {
        m_uiIndex = 0;
    }

    bool operator()(uint itemIndex, RenderModel *rmdl) {
        printf("My index vs their index: %d/%d\n", m_uiIndex, itemIndex);
        //Assuming we iterate in order
        if(itemIndex > m_uiIndex) {
            return true;    //Found an empty index
        } else {
            m_uiIndex++;
            return false;   //Don't stop iterating
        }
    }

    uint m_uiIndex;
};

bool
DraggableHud::addItem(Item *pItem, bool bMakeCurrent) {
    //The item ID will tell us where to put it in the inventory
    uint uiItemId = pItem->getItemId();

    //Wrap the item in the appropriate draggable render model and stash on the
    // appropriate inventory gui panel
    if(uiItemId < ITEM_NUM_ELEMENTS) {
        //Wrap the element and store it
        Rect rcArea = indexToElementRect(uiItemId);
        DraggableElementalSpellItem *pDraggableItem =
            new DraggableElementalSpellItem(pItem, rcArea, this);
        get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER)->add(uiItemId, pDraggableItem);

    } else if(uiItemId < ITEM_NUM_SPELLS) {
        //Wrap the spell and store it
        Rect rcArea = indexToSpellRect(uiItemId);
        DraggableElementalSpellItem *pDraggableItem =
            new DraggableElementalSpellItem(pItem, rcArea, this);
        get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER)->add(uiItemId, pDraggableItem);

    } else {
        //Find an empty item slot
        ItemFindEmptySlotFunctor ftor;
        get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)->forEachModel(ftor);
        if(ftor.m_uiIndex >= NUM_GENERAL_ITEMS) {
            return false;   //Failed to add
        }

        //Wrap the item and store it
        Rect rcArea = indexToItemRect(ftor.m_uiIndex);
        DraggableItem *pDraggableItem = new DraggableItem(pItem, ftor.m_uiIndex, rcArea, this);
        get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)->add(ftor.m_uiIndex, pDraggableItem);
    }


    if(bMakeCurrent) {
        makeCurrent(pItem);
    }

    return true;
}


int
DraggableHud::callBack(uint uiEventHandlerId, void *data, uint uiEventId) {
    int status = EVENT_DROPPED;
    switch(uiEventId) {
    case ON_ITEM_DROPPED: {
        printf("Item dropped!\n");
        ItemDropEvent *event = (ItemDropEvent*)data;
        uint uiItemId = event->item->getItemId();
        //An EVENT_ITEM_CANNOT_DROP flag means that we successfully reacted to the
        //item/spell/element, but its position on the hud should not be changed
        status = EVENT_ITEM_CANNOT_DROP;
        if(uiItemId < ITEM_NUM_ELEMENTS) {
            //Either make the element current or drop it
            if(event->itemNewIndex == CUR_SPELL_ITEM_INDEX) {
                makeCurrent(event->item);
            } else {
                removeElement(event->itemOldIndex);
                m_pMyPlayer->callBack(uiEventHandlerId, data, uiEventId);
            }
        } else if(uiItemId < ITEM_NUM_SPELLS) {
            //Either make the element current or drop it
            if(event->itemNewIndex == CUR_SPELL_ITEM_INDEX) {
                makeCurrent(event->item);
            } else {
                removeSpell(event->itemOldIndex);
                m_pMyPlayer->callBack(uiEventHandlerId, data, uiEventId);
            }
        } else if(event->itemNewIndex == CUR_GENERIC_ITEM_INDEX ||
                  (event->itemOldIndex == event->itemNewIndex && event->distance < 5.f)) {
            //Generic item should be made current
            makeCurrent(event->item);
        } else if(event->itemNewIndex == DROP_GENERIC_ITEM_INDEX) {
            //Generic item should be dropped on the ground.  Remove from HUD and pass on to player
            removeItem(event->itemOldIndex);
            m_pMyPlayer->callBack(uiEventHandlerId, data, uiEventId);
        } else if(event->itemOldIndex != event->itemNewIndex) {
            //Generic item should be moved
            moveItem(event->itemOldIndex, event->itemNewIndex);
            status = EVENT_ITEM_CAN_DROP;
        }
        break;
    }
    default:
        status = Draggable::callBack(uiEventHandlerId, data, uiEventId);
    }
    return status;
}

void
DraggableHud::removeItem(uint invIndex) {
    ContainerRenderModel *panel = get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        m_lsItemsToRemove.push_back(invIndex);  //Schedule for later when it's safer
    }
}

void
DraggableHud::removeSpell(uint invIndex) {
    ContainerRenderModel *panel = get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        m_lsSpellsToRemove.push_back(invIndex);  //Schedule for later when it's safer
    }
}

void
DraggableHud::removeElement(uint invIndex) {
    ContainerRenderModel *panel = get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        m_lsElementsToRemove.push_back(invIndex);  //Schedule for later when it's safer
    }
}

void
DraggableHud::removeScheduledItems() {
    ContainerRenderModel *panel;
    list<uint>::iterator it;

    //Remove generic items
    panel = get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    for(it = m_lsItemsToRemove.begin(); it != m_lsItemsToRemove.end(); ++it) {
        DraggableItem *item = panel->get<DraggableItem*>(*it);
        if(item != NULL) {
            //Make the item not current
            if(m_pCurItem != NULL && m_pCurItem->getId() == item->getId()) {
                m_pCurItem = NULL;

                get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
                    ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM)
                    ->setFrameH(ITEM_NONE);
            }
            panel->remove(*it);
            delete item;
        }
    }
    m_lsItemsToRemove.clear();

    //Remove spells
    panel = get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
    for(it = m_lsSpellsToRemove.begin(); it != m_lsSpellsToRemove.end(); ++it) {
        DraggableElementalSpellItem *item = panel->get<DraggableElementalSpellItem*>(*it);
        if(item != NULL) {
            //Make the item not current
            if(m_pCurSpell != NULL && m_pCurSpell->getId() == item->getId()) {
                m_pCurSpell = NULL;

                get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
                    ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL)
                    ->setFrameH(ITEM_NONE);
            }
            panel->remove(*it);
            delete item;
        }
    }
    m_lsSpellsToRemove.clear();

    //Remove elements
    panel = get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
    for(it = m_lsElementsToRemove.begin(); it != m_lsElementsToRemove.end(); ++it) {
        DraggableElementalSpellItem *item = panel->get<DraggableElementalSpellItem*>(*it);
        if(item != NULL) {
            //Make the item not current
            if(m_pCurElement != NULL && m_pCurElement->getId() == item->getId()) {
                m_pCurElement = NULL;

                get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
                    ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT)
                    ->setFrameH(ITEM_NONE);
            }
            panel->remove(*it);
            delete item;
        }
    }
    m_lsElementsToRemove.clear();
}

Rect
DraggableHud::indexToItemRect(uint index) {
    return Rect(
        (2 + 2 * (index % 6)) * TEXTURE_TILE_SIZE,
        (7 + 2 * (index / 6)) * TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE
    );
}

Rect
DraggableHud::indexToSpellRect(uint index) {
    index--;    //Account for NULL item
    float theta = (index * 2 * M_PI / NUM_SPELL_ITEMS - M_PI / 2);
    return Rect(
        (10 + cos(theta)) * TEXTURE_TILE_SIZE,  //x
        (3 + sin(theta)) * TEXTURE_TILE_SIZE,   //y
        TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE    //w, h
    );
}

Rect
DraggableHud::indexToElementRect(uint index) {
    index--;
    if(index < 4) {
        return Rect(
            (2 + 2 * (index % 2)) * TEXTURE_TILE_SIZE,  //x
            (2 + 2 * (index / 2)) * TEXTURE_TILE_SIZE,  //y
            TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE        //w, h
        );
    } else {
        // The 5th element is centered
        return Rect(
            3 * TEXTURE_TILE_SIZE,                      //x
            3 * TEXTURE_TILE_SIZE,                      //y
            TEXTURE_TILE_SIZE, TEXTURE_TILE_SIZE        //w, h
        );
    }
}

void
DraggableHud::makeCurrent(Item *pItem) {
    uint uiItemId = pItem->getItemId();
    if(uiItemId < ITEM_NUM_ELEMENTS) {
        m_pCurElement = pItem;

        get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
            ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT)
            ->setFrameH(uiItemId);
    } else if(uiItemId < ITEM_NUM_SPELLS) {
        m_pCurSpell = dynamic_cast<SpellItem*>(pItem);

        get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
            ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL)
            ->setFrameH(uiItemId);
    } else {
        m_pCurItem = pItem;

        get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)
            ->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM)
            ->setFrameH(uiItemId);
    }
}

void
DraggableHud::moveItem(uint startIndex, uint endIndex) {
    ContainerRenderModel *panel = get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    DraggableItem *item1 = panel->get<DraggableItem*>(startIndex);
    DraggableItem *item2 = panel->get<DraggableItem*>(endIndex);

    if(item1 != NULL) {
        panel->remove(startIndex);
    }
    if(item2 != NULL) {
        panel->remove(endIndex);
        panel->add(startIndex, item2);
        item2->snapToIndex(startIndex);
    }

    if(item1 != NULL) {
        panel->add(endIndex, item1);
    }
}

void
DraggableHud::initPlayerHud() {
    //const uint hudBackdropId = D3RE::get()->getImageId("hudBackdrop");
    ContainerRenderModel *panel = D3RE::get()->getHudContainer();
    panel->add(HUD_TOPBAR, this);

    panel = this;

    uint uiHudBackdropImgId = D3RE::get()->getImageId("guiBackdrop");
    Rect rcHudBackdropArea = Rect(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT);
    D3HudRenderModel *pBackground = new D3HudRenderModel(uiHudBackdropImgId, rcHudBackdropArea);
    add(MGHUD_BACKDROP, pBackground);

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

bool
DraggableHud::ItemDebugFunctor::operator()(uint itemIndex, RenderModel *rmdl) {
    D3HudRenderModel *hudMdl = dynamic_cast<D3HudRenderModel*>(rmdl);
    Point renderPos;
    Point expPos;
    uint renderId = 0;
    if(hudMdl != NULL) {
        Rect rc = hudMdl->getDrawArea();
        renderPos.x = rc.x + rc.w / 2;
        renderPos.y = rc.y + rc.h / 2;
        renderId = hudMdl->getFrameH();
        expPos.x = rc.w / 2;
        expPos.y = rc.h / 2;
    }

    Rect rcItemArea = m_pHud->indexToItemRect(itemIndex);
    expPos.x += rcItemArea.x;
    expPos.y += rcItemArea.y;

    printf("%d Render:   (%2.2f,%2.2f)\n", renderId, renderPos.x, renderPos.y);
    printf("? Expected: (%2.2f,%2.2f)\n\n", expPos.x, expPos.y);
    return false;   //Don't stop iterating
}
