#include "Inventory.h"
#include "Item.h"

Inventory::Inventory()
{
    for(int i = 0; i < NUM_SPELL_ITEMS; ++i) {
        m_aSpellItems[i] = NULL;
    }
    for(int i = 0; i < NUM_ELEMENT_ITEMS; ++i) {
        m_aElementItems[i] = NULL;
    }
    for(int i = 0; i < NUM_GENERAL_ITEMS; ++i) {
        m_aGeneralItems[i] = NULL;
    }
    m_uiCurSpell = 0;
    m_uiCurItem = 0;
    m_uiCurElement = 0;
}

Inventory::~Inventory()
{
    for(int i = 0; i < NUM_SPELL_ITEMS; ++i) {
        if(m_aSpellItems[i] != NULL) {
            delete m_aSpellItems[i];
        }
    }
    for(int i = 0; i < NUM_ELEMENT_ITEMS; ++i) {
        if(m_aElementItems[i] != NULL) {
            delete m_aElementItems[i];
        }
    }
    for(int i = 0; i < NUM_GENERAL_ITEMS; ++i) {
        if(m_aGeneralItems[i] != NULL) {
            delete m_aGeneralItems[i];
        }
    }
}

int
Inventory::add(Item *item) {
    uint uiItemId = item->getItemId();
    int index;
    if(uiItemId < ITEM_NUM_ELEMENTS) {
        //Store in the element index spaces
        index = uiItemId - 1;
        if(m_aElementItems[index] != NULL) {
            delete m_aElementItems[index];
        }
        m_aElementItems[index] = item;
    } else if(uiItemId < ITEM_NUM_SPELLS) {
        //Store in the spell index spaces
        index = uiItemId - ITEM_NUM_ELEMENTS;
        if(m_aSpellItems[index] != NULL) {
            delete m_aSpellItems;
        }
        m_aSpellItems[index] = item;
    } else {
        for(index = 0; index < NUM_GENERAL_ITEMS; ++index) {
            if(m_aGeneralItems[index] == NULL) {
                m_aGeneralItems[index] = item;
                return index;
            }
        }
        index = -1; //Could not store
    }
    return index;
}
