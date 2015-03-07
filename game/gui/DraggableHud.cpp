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
#include "GuiButton.h"
#include "game/GameManager.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#define Y_HIDDEN (TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_SHOWN (0.F)
#define Y_SHOW_BOUND (3 * TEXTURE_TILE_SIZE - SCREEN_HEIGHT)
#define Y_HIDE_BOUND (-3 * TEXTURE_TILE_SIZE)

#define ANIM_TIMER_MAX 1

using namespace std;

#define BUTTON_SPACING 48

DraggableHud::DraggableHud(uint uiId)
    :   ContainerRenderModel(NULL, Rect(0, Y_HIDDEN, SCREEN_WIDTH, SCREEN_HEIGHT)),
        Draggable(this, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)),
        m_uiId(uiId),
        m_eState(HUD_STATE_NORMAL),
        m_bHidden(true),
        m_iAnimTimer(0),
        m_iFrame(0),
        m_pCurItem(NULL),
        m_pCurSpell(NULL),
        m_pCurElement(NULL),
        m_pMyPlayer(NULL),
        m_sInput("MySave")
{
    MGE::get()->addListener(this, ON_MOUSE_MOVE);
    MGE::get()->addListener(this, ON_BUTTON_INPUT);


    //Fill the inventory with valid item points
    Point ptValid;
    for(ptValid.y = 240; ptValid.y <= 368; ptValid.y += 64) {
        for(ptValid.x = 80; ptValid.x <= 400; ptValid.x += 64) {
            DraggableItem::addValidDropLocation(ptValid);
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

    //Start out as being down
    Point ptShift = Point(0.f, Y_SHOWN - getDrawArea().y, 0.f);
    moveBy(ptShift);
    m_bHidden = false;
    PWE::get()->setState(PWE_PAUSED);
}

DraggableHud::~DraggableHud() {
    MGE::get()->removeListener(this->getId(), ON_MOUSE_MOVE);
    MGE::get()->removeListener(this->getId(), ON_BUTTON_INPUT);
    PWE::get()->freeId(getId());

    //If this is a new-game or load-game reset, we want to be sure these get cleared
    DraggableItem::clearValidDropLocations();

    //Clear only the HUDs that were not cleared by ContainerRenderModel
    if(m_pCurSidePanel != m_pMainSidePanel)     { delete m_pMainSidePanel; }
    if(m_pCurSidePanel != m_pTypeSidePanel)     { delete m_pTypeSidePanel; }
    if(m_pCurSidePanel != m_pConfirmSidePanel)  { delete m_pConfirmSidePanel; }

    //Prevent the render engine from doing a double-delete
    D3RE::get()->getHudContainer()->remove(HUD_TOPBAR);
}

void
DraggableHud::initHud() {
    //Fill the actual hud
    initPlayerHud();
}

/*
 * Draggable
 */
/*
class MoveElementFunctor {
private:
    Point m_ptShift;
public:
    MoveElementFunctor(const Point &shift) : m_ptShift(shift) {}

    bool operator()(uint itemIndex, RenderModel *rmdl) {
        D3HudRenderModel *dmdl = dynamic_cast<D3HudRenderModel*>(rmdl);
        ContainerRenderModel *cmdl = dynamic_cast<ContainerRenderModel*>(rmdl);
        if(dmdl) {
            dmdl->moveBy(m_ptShift);
            printf("DMDL\n");
        } else if(cmdl) {
            cmdl->moveBy(m_ptShift);
            printf("CMDL\n");
        } else {
            printf("ERROR: Other type!\n");
        }
        return false;
    }
};
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

    //Shift contained items
    //MoveElementFunctor ftor(ptShift);
    //forEachModel(ftor);
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
    Point ptShift;
    if(m_bHidden) {
        if(fTop > Y_SHOW_BOUND) {
            //Sufficiently moved for it to be shown
            ptShift = Point(0.f, Y_SHOWN - fTop, 0.f);
            m_bHidden = false;
            enablePanel(m_pCurSidePanel);
        } else {
            //Move back, no change
            ptShift = Point(0.f, Y_HIDDEN - fTop, 0.f);
            PWE::get()->setState(PWE_RUNNING);
        }
    } else {
        if(fTop < Y_HIDE_BOUND) {
            //Sufficiently moved for it to hide
            ptShift = Point(0.f, Y_HIDDEN - fTop, 0.f);
            m_bHidden = true;
            disablePanel(m_pCurSidePanel);
            PWE::get()->setState(PWE_RUNNING);
        } else {
            //Move back, no change
            ptShift = Point(0.f, Y_SHOWN - fTop, 0.f);
        }
    }

    //Shift myself and my contained items
    moveBy(ptShift);
    //MoveElementFunctor ftor(ptShift);
    //forEachModel(ftor);

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
static bool typeCursorOn = false;
    switch(m_eState) {
    case HUD_STATE_TYPE_SAVE_FILE:
    case HUD_STATE_TYPE_LOAD_FILE:
    case HUD_STATE_TYPE_NEW_FILE: {
        //Update typing text
        m_pTypeSidePanel->get<D3HudRenderModel*>(MGHUD_SIDETYPE_TEXT)->updateText(m_sInput + (typeCursorOn ? "_" : ""));
        break;
    }
    default:
        break;
    }

    if(m_iAnimTimer > ANIM_TIMER_MAX) {
        m_iFrame = (m_iFrame + 1) % 8;
        m_iAnimTimer = 0;
        typeCursorOn = !typeCursorOn;

        ContainerRenderModel *panel = m_pInventoryPanel;
        ItemAnimUpdateFunctor curFrameFunctor(m_iFrame);

        //Update general item animations
        panel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)->forEachModel(curFrameFunctor);

        //Update element animations
        panel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER)->forEachModel(curFrameFunctor);

        //Update spell animations
        panel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER)->forEachModel(curFrameFunctor);

        //Update itembar images (which are set to itemid 0 if no item set)
        get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)->forEachModel(curFrameFunctor);
    } else {
        ++m_iAnimTimer;
    }
}


void
DraggableHud::clearInventory() {
    //Clear inventory
    m_pInventoryPanel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)->clear();
    m_pInventoryPanel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER)->clear();
    m_pInventoryPanel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER)->clear();

    //Clear current items
    get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT)->setFrameH(0);
    get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_SPELL)->setFrameH(0);
    get<ContainerRenderModel*>(MGHUD_ITEMBAR_CONTAINER)->get<D3HudRenderModel*>(MGHUD_ELEMENT_ITEMBAR_CUR_ITEM)->setFrameH(0);
}

void
DraggableHud::readInventory(const boost::property_tree::ptree &pt, const std::string &keyBase) {
    using boost::property_tree::ptree;
    //Read in stored inventory items
    try {
        string itemKeyBase = keyBase + ".items";
        ContainerRenderModel *itemPanel = m_pInventoryPanel
                ->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
        BOOST_FOREACH(ptree::value_type invClass, pt.get_child(itemKeyBase)) {
            string className = invClass.first.data();
            string classKeyBase = itemKeyBase + "." + className;
            ObjectFactory::get()->initClass(className);
            BOOST_FOREACH(ptree::value_type item, invClass.second) {
                string name = item.first.data();
                string key = classKeyBase + "." + name;
                uint index = pt.get(key, 0);

                //Read the appropriate item
                Item *itm = dynamic_cast<Item*>(ObjectFactory::get()->createFromTree(pt, key));
                if(itm != NULL) {
                    //Add to the inventory
                    Rect rcArea = indexToItemRect(index);
                    DraggableItem *ditem = new DraggableItem(itm, index, rcArea, this);
                    itemPanel->add(index, ditem);
                }

            }
        }
    } catch(exception &e) {
    }

    try {
        string spellKeyBase = keyBase + ".spells";
        ContainerRenderModel *spellPanel = m_pInventoryPanel
                ->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
        BOOST_FOREACH(ptree::value_type invClass, pt.get_child(spellKeyBase)) {
            string className = invClass.first.data();
            string classKeyBase = spellKeyBase + "." + className;
            ObjectFactory::get()->initClass(className);
            BOOST_FOREACH(ptree::value_type spell, invClass.second) {
                string name = spell.first.data();
                string key = classKeyBase + "." + name;
                uint index = pt.get(key, 0);

                //Read the appropriate item
                Item *item = dynamic_cast<Item*>(ObjectFactory::get()->createFromTree(pt, key));
                if(item != NULL) {
                    //Add to the inventory
                    Rect rcArea = indexToSpellRect(index);
                    DraggableElementalSpellItem *ditem = new DraggableElementalSpellItem(item, rcArea, this);
                    spellPanel->add(index, ditem);
                }
            }
        }
    } catch(exception &e) {
    }

    try {
        ContainerRenderModel *elementPanel = m_pInventoryPanel
                ->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
        string elementKeyBase = keyBase + ".elements";
        BOOST_FOREACH(ptree::value_type invClass, pt.get_child(elementKeyBase)) {
            string className = invClass.first.data();
            string classKeyBase = elementKeyBase + "." + className;
            ObjectFactory::get()->initClass(className);
            BOOST_FOREACH(ptree::value_type element, invClass.second/*pt.get_child(elementKeyBase)*/) {
                string name = element.first.data();
                string key = classKeyBase + "." + name;
                uint index = pt.get(key, 0);

                //Read the appropriate item
                Item *item = dynamic_cast<Item*>(ObjectFactory::get()->createFromTree(pt, key));
                if(item != NULL) {
                    //Add to the inventory
                    Rect rcArea = indexToElementRect(index);
                    DraggableElementalSpellItem *ditem = new DraggableElementalSpellItem(item, rcArea, this);
                    elementPanel->add(index, ditem);
                }
            }
        }
    } catch(exception &e) {
    }
}

class WriteItemModelFunctor {
    boost::property_tree::ptree &m_pt;
    const std::string m_keyBase;
public:
    WriteItemModelFunctor(boost::property_tree::ptree &pt, const std::string &keyBase)
        :   m_pt(pt),
            m_keyBase(keyBase)
    {
    }

    bool operator()(uint index, RenderModel *rm) {
        //Cast to the appropriate type
        DraggableItem *item = dynamic_cast<DraggableItem*>(rm);
        if(item != NULL) {
            string keyBase = m_keyBase + "." + item->getItem()->getClassName() + ".item" + boost::lexical_cast<string>(index);
            m_pt.put(keyBase, index);
            item->getItem()->write(m_pt, keyBase);
        }
        return false;
    }
};

class WriteSpellModelFunctor {
    boost::property_tree::ptree &m_pt;
    const std::string m_keyBase;
public:
    WriteSpellModelFunctor(boost::property_tree::ptree &pt, const std::string &keyBase)
        :   m_pt(pt),
            m_keyBase(keyBase)
    {
    }

    bool operator()(uint index, RenderModel *rm) {
        //Cast to the appropriate type
        DraggableElementalSpellItem *item = dynamic_cast<DraggableElementalSpellItem*>(rm);
        if(item != NULL) {
            string keyBase = m_keyBase + "." + item->getItem()->getClassName() + ".spell" + boost::lexical_cast<string>(index);
            m_pt.put(keyBase, index);
            item->getItem()->write(m_pt, keyBase);
        }
        return false;
    }
};

void
DraggableHud::writeInventory(boost::property_tree::ptree &pt, const std::string &keyBase) {
    //Write stored inventory items
    ContainerRenderModel *itemPanel = m_pInventoryPanel
            ->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    ContainerRenderModel *spellPanel = m_pInventoryPanel
            ->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
    ContainerRenderModel *elementPanel = m_pInventoryPanel
            ->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);

    WriteItemModelFunctor itemFtor(pt, keyBase + ".items");
    WriteSpellModelFunctor spellFtor(pt, keyBase + ".spells");
    WriteSpellModelFunctor elementFtor(pt, keyBase + ".elements");
    itemPanel->forEachModel(itemFtor);
    spellPanel->forEachModel(spellFtor);
    elementPanel->forEachModel(elementFtor);
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
        //printf("My index vs their index: %d/%d\n", m_uiIndex, itemIndex);
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

    ContainerRenderModel *panel = m_pInventoryPanel;

    //Wrap the item in the appropriate draggable render model and stash on the
    // appropriate inventory gui panel
    if(uiItemId < ITEM_NUM_ELEMENTS) {
        //Wrap the element and store it
        Rect rcArea = indexToElementRect(uiItemId);
        DraggableElementalSpellItem *pDraggableItem =
            new DraggableElementalSpellItem(pItem, rcArea, this);
        panel->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER)->add(uiItemId, pDraggableItem);

    } else if(uiItemId < ITEM_NUM_SPELLS) {
        //Wrap the spell and store it
        Rect rcArea = indexToSpellRect(uiItemId);
        DraggableElementalSpellItem *pDraggableItem =
            new DraggableElementalSpellItem(pItem, rcArea, this);
        panel->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER)->add(uiItemId, pDraggableItem);

    } else {
        //Find an empty item slot
        ItemFindEmptySlotFunctor ftor;
        panel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)->forEachModel(ftor);
        if(ftor.m_uiIndex >= NUM_GENERAL_ITEMS) {
            return false;   //Failed to add
        }

        //Wrap the item and store it
        Rect rcArea = indexToItemRect(ftor.m_uiIndex);
        DraggableItem *pDraggableItem = new DraggableItem(pItem, ftor.m_uiIndex, rcArea, this);
        panel->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER)->add(ftor.m_uiIndex, pDraggableItem);
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
    case ON_SAVE_GAME: {
        //Present the player with game-save options
        GameManager::get()->getCurGameFileRoot(m_sInput);
        m_eState = HUD_STATE_TYPE_SAVE_FILE;
        prepSideTypeHud("Enter a save name:", "Save Game", "Cancel");
        break;
    }
    case ON_LOAD_GAME:
        GameManager::get()->getCurGameFileRoot(m_sInput);
        m_eState = HUD_STATE_TYPE_LOAD_FILE;
        prepSideTypeHud("Enter a save name:", "Load Game", "Cancel");
        break;
    case ON_NEW_GAME:
        GameManager::get()->getCurGameFileRoot(m_sInput);
        m_eState = HUD_STATE_TYPE_NEW_FILE;
        prepSideTypeHud("Choose a save name:", "New Game", "Cancel");
        break;
    case ON_QUIT_GAME:
        prepSideConfirmHud("Are you sure?", "Quit", "Cancel");
        m_eState = HUD_STATE_QUIT;
        break;
    case ON_ACTION_BUTTON:
        switch(m_eState) {
        case HUD_STATE_TYPE_SAVE_FILE:
            //Done getting the save file name, actually save the game
            GameManager::get()->saveGame(m_sInput);
            break;
        case HUD_STATE_TYPE_LOAD_FILE:
            GameManager::get()->loadGame(m_sInput);
            break;
        case HUD_STATE_TYPE_NEW_FILE:
            GameManager::get()->newGame(m_sInput);
            break;
        case HUD_STATE_QUIT:
            MGE::get()->stop();
        default:
            break;
        }

        //Afterwards, return to normal
        prepSideButtonHud();
        m_eState = HUD_STATE_NORMAL;

        break;
    case ON_INACTION_BUTTON:
        prepSideButtonHud();
        m_eState = HUD_STATE_NORMAL;
        break;
    case ON_BUTTON_INPUT: {
        //We might be typing
        switch(m_eState) {
        case HUD_STATE_TYPE_SAVE_FILE:
        case HUD_STATE_TYPE_LOAD_FILE:
        case HUD_STATE_TYPE_NEW_FILE:
            status = typeOnKeyPress((InputData*)data);
            break;
        default:
            break;
        }
        if(status == EVENT_CAUGHT) {
            break;
        }

        //Otherwise, continue. ON_BUTTON_INPUT needs to be handled by Draggable
    }
    default:
        status = Draggable::callBack(uiEventHandlerId, data, uiEventId);
    }
    return status;
}

void
DraggableHud::removeItem(uint invIndex) {
    ContainerRenderModel *panel = m_pInventoryPanel
                                    ->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        m_lsItemsToRemove.push_back(invIndex);  //Schedule for later when it's safer
    }
}

void
DraggableHud::removeSpell(uint invIndex) {
    ContainerRenderModel *panel = m_pInventoryPanel
                                    ->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
    RenderModel *item = panel->get<RenderModel*>(invIndex);
    if(item != NULL) {
        m_lsSpellsToRemove.push_back(invIndex);  //Schedule for later when it's safer
    }
}

void
DraggableHud::removeElement(uint invIndex) {
    ContainerRenderModel *panel = m_pInventoryPanel
                                    ->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
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
    panel = m_pInventoryPanel
            ->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
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
            item->removeItem();    //Makes sure the item thumbnail does not try to delete the actual item
            panel->remove(*it);
            delete item;
        }
    }
    m_lsItemsToRemove.clear();

    //Remove spells
    panel = m_pInventoryPanel
            ->get<ContainerRenderModel*>(MGHUD_SPELL_CONTAINER);
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
            item->removeItem();    //Makes sure the item thumbnail does not try to delete the actual item
            panel->remove(*it);
            delete item;
        }
    }
    m_lsSpellsToRemove.clear();

    //Remove elements
    panel = m_pInventoryPanel
            ->get<ContainerRenderModel*>(MGHUD_ELEMENT_CONTAINER);
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
            item->removeItem();    //Makes sure the item thumbnail does not try to delete the actual item
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
    ContainerRenderModel *panel = m_pInventoryPanel
                                ->get<ContainerRenderModel*>(MGHUD_ITEM_CONTAINER);
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
        TEXTURE_TILE_SIZE * 14,
        TEXTURE_TILE_SIZE
    );
    Rect rcSidePanel = Rect(
        rcInventoryPanel.x + rcInventoryPanel.w,
        0,  //SCREEN_HEIGHT,
        TEXTURE_TILE_SIZE * 4,
        TEXTURE_TILE_SIZE
    );
    Rect rcItembarPanel = Rect(
        TEXTURE_TILE_SIZE * 16,
        SCREEN_HEIGHT - TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 3,
        TEXTURE_TILE_SIZE
    );
    ContainerRenderModel *healthPanel = new ContainerRenderModel(NULL, rcHealthPanel);
    ContainerRenderModel *itembarPanel = new ContainerRenderModel(NULL, rcItembarPanel);
    ContainerRenderModel *spellPanel = new ContainerRenderModel(NULL, rcInventoryPanel);
    ContainerRenderModel *itemPanel = new ContainerRenderModel(NULL, rcInventoryPanel);

    //We keep these two specially so that they can be restored instantly
    m_pInventoryPanel = new ContainerRenderModel(NULL, rcInventoryPanel);
    m_pMainSidePanel = new ContainerRenderModel(NULL, rcSidePanel);
    m_pTypeSidePanel = new ContainerRenderModel(NULL, rcSidePanel);
    m_pConfirmSidePanel = new ContainerRenderModel(NULL, rcSidePanel);

    ContainerRenderModel *elementPanel = new ContainerRenderModel(NULL, rcInventoryPanel);
    panel->add(MGHUD_HEALTH_CONTAINER, healthPanel);
    panel->add(MGHUD_ITEMBAR_CONTAINER, itembarPanel);
    panel->add(MGHUD_MAIN_CONTAINER, m_pInventoryPanel);
    panel->add(MGHUD_SIDEBUTTON_CONTAINER, m_pMainSidePanel);
    m_pCurSidePanel = m_pMainSidePanel;

    m_pInventoryPanel->add(MGHUD_SPELL_CONTAINER, spellPanel);
    m_pInventoryPanel->add(MGHUD_ITEM_CONTAINER, itemPanel);
    m_pInventoryPanel->add(MGHUD_ELEMENT_CONTAINER, elementPanel);

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
    D3HudRenderModel *label = new D3HudRenderModel("area", rcAreaLabel, new TextRenderer::BasicCharacterFilter(1.0f));
    label->centerHorizontally(true);
    panel->add(MGHUD_CUR_AREA, label);

    label = new D3HudRenderModel("action", rcActionLabel, new TextRenderer::BasicCharacterFilter(0.8f));
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

    /*
     * Side panels
     */
    initSideButtonHud(m_pMainSidePanel);
    initSideTypeHud(m_pTypeSidePanel);
    initSideConfirmHud(m_pConfirmSidePanel);
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

    D3HudRenderModel *label = new D3HudRenderModel("99", Rect(BAR_X+BAR_WIDTH/2.f - 10.f,BAR_Y,20.f,BAR_SIZE), new TextRenderer::BasicCharacterFilter(0.8f));
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


void
DraggableHud::initSideButtonHud(ContainerRenderModel *panel) {
#define SIDEBAR_MARGIN 5
    Rect rcItemNameLabel = Rect(
        SIDEBAR_MARGIN,
        TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 6 - SIDEBAR_MARGIN,
        TEXTURE_TILE_SIZE
    );
    Rect rcItemDescLabel = Rect(
        rcItemNameLabel.x,
        rcItemNameLabel.y + rcItemNameLabel.h,
        rcItemNameLabel.w,
        TEXTURE_TILE_SIZE * 3
    );
    Point ptNewGameButtonPos(
        rcItemDescLabel.x + rcItemDescLabel.w / 2 - BUTTON_WIDTH / 2,
        rcItemDescLabel.y + rcItemDescLabel.h,
        0
    );
    Point ptLoadButtonPos(
        ptNewGameButtonPos.x,
        ptNewGameButtonPos.y + BUTTON_SPACING,
        0
    );
    Point ptSaveButtonPos(
        ptLoadButtonPos.x,
        ptLoadButtonPos.y + BUTTON_SPACING,
        0
    );
    Point ptQuitButtonPos(
        ptSaveButtonPos.x,
        ptSaveButtonPos.y + BUTTON_SPACING,
        0
    );
    Point ptOptionsButtonPos(
        ptQuitButtonPos.x,
        ptQuitButtonPos.y + BUTTON_SPACING,
        0
    );
    D3HudRenderModel *label = new D3HudRenderModel("", rcItemNameLabel, new TextRenderer::BasicCharacterFilter(1.0f));
    label->centerHorizontally(true);
    panel->add(MGHUD_SIDEBUTTON_ITEMNAME, label);

    label = new D3HudRenderModel("", rcItemDescLabel, new TextRenderer::BasicCharacterFilter(0.8f));
    panel->add(MGHUD_SIDEBUTTON_ITEMDESC, label);

    GuiButton *btn;
    btn = new GuiButton(panel, this, ON_NEW_GAME, "New", ptNewGameButtonPos, 1.f);
    panel->add(MGHUD_SIDEBUTTON_NEWBUTTON, btn);

    btn = new GuiButton(panel, this, ON_LOAD_GAME, "Load", ptLoadButtonPos, 1.f);
    panel->add(MGHUD_SIDEBUTTON_LOADBUTTON, btn);

    btn = new GuiButton(panel, this, ON_SAVE_GAME, "Save", ptSaveButtonPos, 1.f);
    panel->add(MGHUD_SIDEBUTTON_SAVEBUTTON, btn);

    btn = new GuiButton(panel, this, ON_QUIT_GAME, "Quit", ptQuitButtonPos, 1.f);
    panel->add(MGHUD_SIDEBUTTON_QUITBUTTON, btn);

    btn = new GuiButton(panel, this, ON_OPTIONS, "Options", ptOptionsButtonPos, 1.f);
    panel->add(MGHUD_SIDEBUTTON_OPTIONSBUTTON, btn);


    //Enable buttons
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_NEWBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_LOADBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_SAVEBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_QUITBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_OPTIONSBUTTON)->enable();
}


void
DraggableHud::initSideTypeHud(ContainerRenderModel *panel) {
    Rect rcMessageLabel = Rect(
        SIDEBAR_MARGIN,
        TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 6 - SIDEBAR_MARGIN,
        TEXTURE_TILE_SIZE
    );
    Rect rcTextLabel = Rect(
        rcMessageLabel.x,
        rcMessageLabel.y + rcMessageLabel.h,
        rcMessageLabel.w,
        TEXTURE_TILE_SIZE * 3
    );
    Point ptActionButtonPos(
        rcTextLabel.x + rcTextLabel.w / 2 - BUTTON_WIDTH / 2,
        rcTextLabel.y + rcTextLabel.h,
        0
    );
    Point ptInactionButtonPos(
        ptActionButtonPos.x,
        ptActionButtonPos.y + BUTTON_SPACING,
        0
    );
    D3HudRenderModel *label = new D3HudRenderModel("", rcMessageLabel, new TextRenderer::BasicCharacterFilter(1.0f));
    label->centerHorizontally(true);
    panel->add(MGHUD_SIDETYPE_MESSAGE, label);

    label = new D3HudRenderModel("", rcTextLabel, new TextRenderer::BasicCharacterFilter(0.9f));
    panel->add(MGHUD_SIDETYPE_TEXT, label);

    GuiButton *btn;
    btn = new GuiButton(panel, this, ON_ACTION_BUTTON, "", ptActionButtonPos, 1.f);
    btn->disable();
    panel->add(MGHUD_SIDETYPE_ACTION_BUTTON, btn);

    btn = new GuiButton(panel, this, ON_INACTION_BUTTON, "", ptInactionButtonPos, 1.f);
    btn->disable();
    panel->add(MGHUD_SIDETYPE_INACTION_BUTTON, btn);
}

void
DraggableHud::initSideConfirmHud(ContainerRenderModel *panel) {
    Rect rcMessageLabel = Rect(
        SIDEBAR_MARGIN,
        TEXTURE_TILE_SIZE,
        TEXTURE_TILE_SIZE * 6 - SIDEBAR_MARGIN,
        TEXTURE_TILE_SIZE
    );
    Rect rcBlankLabel = Rect(   //This exists just to make spacing more convenient
        rcMessageLabel.x,
        rcMessageLabel.y + rcMessageLabel.h,
        rcMessageLabel.w,
        TEXTURE_TILE_SIZE * 3
    );
    Point ptActionButtonPos(
        rcBlankLabel.x + rcBlankLabel.w / 2 - BUTTON_WIDTH / 2,
        rcBlankLabel.y + rcBlankLabel.h,
        0
    );
    Point ptInactionButtonPos(
        ptActionButtonPos.x,
        ptActionButtonPos.y + BUTTON_SPACING,
        0
    );
    D3HudRenderModel *label = new D3HudRenderModel("", rcMessageLabel, new TextRenderer::BasicCharacterFilter(1.0f));
    label->centerHorizontally(true);
    panel->add(MGHUD_SIDECONFIRM_MESSAGE, label);

    GuiButton *btn;
    btn = new GuiButton(panel, this, ON_ACTION_BUTTON, "", ptActionButtonPos, 1.f);
    btn->disable();
    panel->add(MGHUD_SIDECONFIRM_ACTION_BUTTON, btn);

    btn = new GuiButton(panel, this, ON_INACTION_BUTTON, "", ptInactionButtonPos, 1.f);
    btn->disable();
    panel->add(MGHUD_SIDECONFIRM_INACTION_BUTTON, btn);
}

void
DraggableHud::prepSideTypeHud(const std::string &sMessageLabel, const std::string &sActionLabel, const std::string &sInactionLabel) {
    disablePanel(m_pCurSidePanel);

    //Set up message/button text
    m_pTypeSidePanel->get<D3HudRenderModel*>(MGHUD_SIDETYPE_MESSAGE)->updateText(sMessageLabel);
    m_pTypeSidePanel->get<D3HudRenderModel*>(MGHUD_SIDETYPE_TEXT)->updateText(m_sInput);
    m_pTypeSidePanel->get<GuiButton*>(MGHUD_SIDETYPE_ACTION_BUTTON)->updateText(sActionLabel);
    m_pTypeSidePanel->get<GuiButton*>(MGHUD_SIDETYPE_INACTION_BUTTON)->updateText(sInactionLabel);
    m_pTypeSidePanel->get<GuiButton*>(MGHUD_SIDETYPE_ACTION_BUTTON)->enable();
    m_pTypeSidePanel->get<GuiButton*>(MGHUD_SIDETYPE_INACTION_BUTTON)->enable();

    //Remove whichever container is currently there
    remove(MGHUD_SIDEBUTTON_CONTAINER);

    //Add the appropriate container
    add(MGHUD_SIDEBUTTON_CONTAINER, m_pTypeSidePanel);
    m_pCurSidePanel = m_pTypeSidePanel;

    //Setup typing input map
    GameManager::get()->setTypingInputMapping();
}

void
DraggableHud::prepSideConfirmHud(const std::string &sMessageLabel, const std::string &sActionLabel, const std::string &sInactionLabel) {
    disablePanel(m_pCurSidePanel);

    //Set up message/button text
    m_pConfirmSidePanel->get<D3HudRenderModel*>(MGHUD_SIDECONFIRM_MESSAGE)->updateText(sMessageLabel);
    m_pConfirmSidePanel->get<GuiButton*>(MGHUD_SIDECONFIRM_ACTION_BUTTON)->updateText(sActionLabel);
    m_pConfirmSidePanel->get<GuiButton*>(MGHUD_SIDECONFIRM_INACTION_BUTTON)->updateText(sInactionLabel);
    m_pConfirmSidePanel->get<GuiButton*>(MGHUD_SIDECONFIRM_ACTION_BUTTON)->enable();
    m_pConfirmSidePanel->get<GuiButton*>(MGHUD_SIDECONFIRM_INACTION_BUTTON)->enable();

    //Remove whichever container is currently there
    remove(MGHUD_SIDEBUTTON_CONTAINER);

    //Add the appropriate container
    add(MGHUD_SIDEBUTTON_CONTAINER, m_pConfirmSidePanel);
    m_pCurSidePanel = m_pConfirmSidePanel;
}

void
DraggableHud::prepSideButtonHud() {
    //Cleanup the current panel
    disablePanel(m_pCurSidePanel);

    remove(MGHUD_SIDEBUTTON_CONTAINER);
    add(MGHUD_SIDEBUTTON_CONTAINER, m_pMainSidePanel);
    m_pCurSidePanel = m_pMainSidePanel;

    //Enable buttons
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_NEWBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_LOADBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_SAVEBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_QUITBUTTON)->enable();
    m_pMainSidePanel->get<GuiButton*>(MGHUD_SIDEBUTTON_OPTIONSBUTTON)->enable();


    //Make sure the appropriate input mapping is setup
    GameManager::get()->setDefaultInputMapping();   //TODO:
}

class PanelEnablerFunctor {
public:
    PanelEnablerFunctor(bool bEnable)
        :   m_bEnable(bEnable)
    {
    }

    bool operator()(uint itemIndex, RenderModel *rmdl) {
        GuiButton *button = dynamic_cast<GuiButton*>(rmdl);
        if(button != NULL) {
            if(m_bEnable) {
                button->enable();
            } else {
                button->disable();
            }
        }
        return false;
    }
    bool m_bEnable;
};

void
DraggableHud::disablePanel(ContainerRenderModel *panel) {
    PanelEnablerFunctor ftor(false);
    panel->forEachModel(ftor);
}

void
DraggableHud::enablePanel(ContainerRenderModel *panel) {
    PanelEnablerFunctor ftor(true);
    panel->forEachModel(ftor);
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


int
DraggableHud::typeOnKeyPress(InputData *data) {
    int status = EVENT_DROPPED;
    if(data->getInputState(KIN_LETTER_PRESSED) && data->hasChanged(KIN_LETTER_PRESSED)) {
        status = EVENT_CAUGHT;
        uint letters = data->getLettersDown();
        uint l = 1;
        for(int i = 0; i < 26; ++i) {
            if(letters & l && data->getInputState(IN_SHIFT)) {
                m_sInput.append(1, 'A' + i);
            } else if(letters & l) {
                m_sInput.append(1, 'a' + i);
            }
            l = l << 1;
        }
    }
    if(data->getInputState(KIN_NUMBER_PRESSED) && data->hasChanged(KIN_NUMBER_PRESSED)) {
        status = EVENT_CAUGHT;
        uint numbers = data->getNumbersDown();
        uint n = 1;
        for(int i = 0; i < 10; ++i) {
            if(numbers & n) {
                m_sInput.append(1, '0' + i);
            }
            n = n << 1;
        }
    }

    if(data->getInputState(TP_IN_SPACE) && data->hasChanged(TP_IN_SPACE)) {
        m_sInput.append(1, ' ');
        status = EVENT_CAUGHT;
    }
    if(data->getInputState(TP_IN_PERIOD) && data->hasChanged(TP_IN_PERIOD)) {
        m_sInput.append(1, '.');
        status = EVENT_CAUGHT;
    }
    if(data->getInputState(TP_IN_UNDERSCORE) && data->hasChanged(TP_IN_UNDERSCORE)) {
        if(data->getInputState(IN_SHIFT)) {
            m_sInput.append(1, '_');
            status = EVENT_CAUGHT;
        } else {
            m_sInput.append(1, '-');
            status = EVENT_CAUGHT;
        }
    }
    if(data->getInputState(TP_IN_SLASH) && data->hasChanged(TP_IN_SLASH)) {
        m_sInput.append(1, '/');
        status = EVENT_CAUGHT;
    }
    if(data->getInputState(TP_IN_COLON) && data->hasChanged(TP_IN_COLON)) {
        m_sInput.append(1, ':');
        status = EVENT_CAUGHT;
    }
    if(data->getInputState(TP_IN_BACKSPACE) && data->hasChanged(TP_IN_BACKSPACE) && m_sInput.size() > 0) {
        m_sInput.resize(m_sInput.size() - 1);
        status = EVENT_CAUGHT;
    }
    if(data->getInputState(TP_IN_ENTER) && data->hasChanged(TP_IN_ENTER)) {
        //Handle positive end-of-type event
        callBack(getId(), NULL, ON_ACTION_BUTTON);
        status = EVENT_CAUGHT;
    }
    return status;
}
