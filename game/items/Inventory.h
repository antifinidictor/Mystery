#ifndef INVENTORY_H
#define INVENTORY_H
/*
 * Game-specific inventory class
 * This class stores elements, spells, and general items separately
 */

#include "game/game_defs.h"

class Item;

enum ElementItemIds {
    ITEM_ELEMENT_EARTH,
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

class Inventory
{
public:
    Inventory();
    virtual ~Inventory();

    int add(Item *item);    //Returns the index of the item, -1 if it could not be stored
    Item *getSpell(uint index)   { return m_aSpellItems[index]; }
    Item *getElement(uint index) { return m_aElementItems[index]; }
    Item *getGeneral(uint index) { return m_aGeneralItems[index]; }
    void removeSpell(uint index)   { m_aSpellItems[index] = NULL; }
    void removeElement(uint index) { m_aElementItems[index] = NULL; }
    void removeGeneral(uint index) { m_aGeneralItems[index] = NULL; }

protected:
private:
    Item *m_aSpellItems[NUM_SPELL_ITEMS];
    Item *m_aElementItems[NUM_ELEMENT_ITEMS];
    Item *m_aGeneralItems[NUM_GENERAL_ITEMS];
    //Item *m_pLeftUse, *m_pRightUse;
};

#endif // INVENTORY_H
