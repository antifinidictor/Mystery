#ifndef INVENTORY_H
#define INVENTORY_H
/*
 * Game-specific inventory class
 * This class stores elements, spells, and general items separately
 */

#include "game/game_defs.h"

class Item;
class SpellItem;
class InventoryDisplay;

#define ITEM_NONE 0
enum ElementItemIds {
    ITEM_ELEMENT_EARTH = 1,
    ITEM_ELEMENT_AIR,
    ITEM_ELEMENT_FIRE,
    ITEM_ELEMENT_WATER,
    ITEM_NUM_ELEMENTS
};

enum SpellItemIds {
    ITEM_SPELL_CYCLIC = ITEM_NUM_ELEMENTS,
    ITEM_SPELL_FLOW,
    ITEM_NUM_SPELLS
};

enum GeneralItemIds {
    ITEM_TEST = ITEM_NUM_SPELLS,
    ITEM_NUM_ITEMS
};

#define NUM_SPELL_ITEMS     2
#define NUM_ELEMENT_ITEMS   4
#define NUM_GENERAL_ITEMS   18

#define ITEM_SPELL_INDEX 0
#define ITEM_ELEMENT_INDEX 1
#define ITEM_GENERIC_INDEX 2

#define CUR_GENERIC_ITEM_INDEX (NUM_GENERAL_ITEMS)
#define DROP_GENERIC_ITEM_INDEX (NUM_GENERAL_ITEMS - 1)
#define CUR_SPELL_ITEM_INDEX (0)
#define DROP_SPELL_ITEM_INDEX (1)

class Inventory
{
public:
    Inventory();
    virtual ~Inventory();

    int add(Item *item);    //Returns the index of the item, -1 if it could not be stored
    SpellItem *getSpell(uint index)   { return m_aSpellItems[index]; }
    Item *getElement(uint index) { return m_aElementItems[index]; }
    Item *getItem(uint index) { return m_aGeneralItems[index]; }
    void removeSpell(uint index);
    void removeElement(uint index);
    void removeItem(uint index);
    void moveItem(uint uiStartIndex, uint uiEndIndex);

    SpellItem *getCurSpell() { return m_aSpellItems[m_uiCurSpell]; }
    Item *getCurItem() { return m_aGeneralItems[m_uiCurItem]; }
    Item *getCurElement() { return m_aElementItems[m_uiCurElement]; }

    void setCurSpell(uint curSpell) { m_uiCurSpell = curSpell; }
    void setCurItem(uint curItem) { m_uiCurItem = curItem; }
    void setCurElement(uint curElement) { m_uiCurElement = curElement; }
    void setInventoryDisplay(InventoryDisplay *pDisplay);
protected:
private:
    SpellItem *m_aSpellItems[NUM_SPELL_ITEMS];
    Item *m_aElementItems[NUM_ELEMENT_ITEMS];
    Item *m_aGeneralItems[NUM_GENERAL_ITEMS];

    //One spell/element combination is active
    uint m_uiCurSpell;
    uint m_uiCurElement;

    //One item is active
    uint m_uiCurItem;
    InventoryDisplay *m_pDisplay;
};

#endif // INVENTORY_H
