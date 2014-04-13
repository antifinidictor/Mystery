#ifndef DRAGGABLEHUD_H
#define DRAGGABLEHUD_H

#include "Draggable.h"
#include "game/items/Item.h"
#include "d3re/ContainerRenderModel.h"
#include <map>
#include <list>

#define ITEM_NONE 0
enum ElementItemIds {
    ITEM_ELEMENT_EARTH = 1,
    ITEM_ELEMENT_AIR,
    ITEM_ELEMENT_FIRE,
    ITEM_ELEMENT_WATER,
    ITEM_ELEMENT_TIME,
    ITEM_NUM_ELEMENTS
};

enum SpellItemIds {
    ITEM_SPELL_CYCLIC = ITEM_NUM_ELEMENTS,
    ITEM_SPELL_FLOW,
    ITEM_SPELL_VORTEX,
    ITEM_NUM_SPELLS
};

enum GeneralItemIds {
    ITEM_TEST = ITEM_NUM_SPELLS,
    ITEM_NUM_ITEMS
};

#define NUM_SPELL_ITEMS     (ITEM_NUM_SPELLS - ITEM_NUM_ELEMENTS)
#define NUM_ELEMENT_ITEMS   (ITEM_NUM_ELEMENTS - ITEM_ELEMENT_EARTH)
#define NUM_GENERAL_ITEMS   17

#define CUR_GENERIC_ITEM_INDEX (NUM_GENERAL_ITEMS + 1)
#define DROP_GENERIC_ITEM_INDEX (NUM_GENERAL_ITEMS)
#define CUR_SPELL_ITEM_INDEX (0)
#define DROP_SPELL_ITEM_INDEX (1)

class Listener;
class SpellItem;

class DraggableHud : public ContainerRenderModel, public Draggable
{
public:
    DraggableHud(uint uiId);
    virtual ~DraggableHud();

	//virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);
    virtual uint getId() { return m_uiId; }

    virtual void onFollow(const Point &diff);
    virtual void onStartDragging();
    virtual void onEndDragging();

    virtual void moveBy(const Point &ptShift);



    //Inventory properties
    bool       addItem(Item *pItem, bool bMakeCurrent = true);
    Item      *getCurItem()     { return m_pCurItem; }
    SpellItem *getCurSpell()    { return m_pCurSpell; }
    Item      *getCurElement()  { return m_pCurElement; }
    void registerPlayer(Listener *pPlayer) { m_pMyPlayer = pPlayer; }

    //Override callback to respond to some events
	virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);

	void makeCurrent(Item *pItem);

    //From InventoryDisplay
    virtual void removeItem(uint invIndex);
    virtual void removeSpell(uint invIndex);
    virtual void removeElement(uint invIndex);
    virtual void moveItem(uint startIndex, uint endIndex);


    void updateItemAnimations();

private:

    void initPlayerHud();
    void initHealthBarHud(ContainerRenderModel *panel);
    void initItemBarHud(ContainerRenderModel *panel);
    void initSideButtonHud(ContainerRenderModel *panel);

    void removeScheduledItems();

    Rect indexToItemRect(uint index);
    Rect indexToSpellRect(uint index);
    Rect indexToElementRect(uint index);

    uint m_uiId;

    bool m_bHidden;
    int m_iAnimTimer;
    int m_iFrame;

    Item *m_pCurItem;
    SpellItem *m_pCurSpell;
    Item *m_pCurElement;

    Listener *m_pMyPlayer;
    std::list<uint> m_lsItemsToRemove;
    std::list<uint> m_lsSpellsToRemove;
    std::list<uint> m_lsElementsToRemove;


    class ItemDebugFunctor {
    public:
        ItemDebugFunctor(DraggableHud *pHud) {
            m_pHud = pHud;
        }

        bool operator()(uint itemIndex, RenderModel *rmdl);

    private:
        DraggableHud *m_pHud;
    };
};

#endif // DRAGGABLEHUD_H
