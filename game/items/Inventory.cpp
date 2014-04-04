#include "Inventory.h"
#include "Item.h"
#include "SpellItem.h"
#include "InventoryDisplay.h"

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
    m_pDisplay = NULL;
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

        if(m_pDisplay != NULL) {
            m_pDisplay->removeElement(index);
            m_pDisplay->addElement(uiItemId, index);
        }
        m_aElementItems[index] = item;
    } else if(uiItemId < ITEM_NUM_SPELLS) {
        //Store in the spell index spaces
        SpellItem *spellItem = dynamic_cast<SpellItem*>(item);
        if(spellItem != NULL) {
            index = uiItemId - ITEM_NUM_ELEMENTS;
            if(m_aSpellItems[index] != NULL) {
                delete m_aSpellItems;
            }
            if(m_pDisplay != NULL) {
                m_pDisplay->removeSpell(index);
                m_pDisplay->addSpell(uiItemId, index);
            }
            m_aSpellItems[index] = spellItem;
        } else {
            index = -1; //The item claims to be a spell, but it is not
        }
    } else {
        for(index = 0; index < NUM_GENERAL_ITEMS; ++index) {
            if(m_aGeneralItems[index] == NULL) {
                m_aGeneralItems[index] = item;
                if(m_pDisplay != NULL) {
                    m_pDisplay->addItem(uiItemId, index);
                }
                return index;
            }
        }
        index = -1; //Could not store
    }
    return index;
}


void
Inventory::removeSpell(uint index)   {
    m_aSpellItems[index] = NULL;
    m_pDisplay->removeSpell(index);
}

void
Inventory::removeElement(uint index) {
    m_aElementItems[index] = NULL;
    m_pDisplay->removeElement(index);
}

void
Inventory::removeItem(uint index) {
    m_aGeneralItems[index] = NULL;
    m_pDisplay->removeItem(index);
}


void
Inventory::moveItem(uint uiStartIndex, uint uiEndIndex) {
printf(__FILE__" %d\n",__LINE__);
    Item *pItemOld = m_aGeneralItems[uiEndIndex];
    m_aGeneralItems[uiEndIndex] = m_aGeneralItems[uiStartIndex];
    m_aGeneralItems[uiStartIndex] = pItemOld;
printf(__FILE__" %d\n",__LINE__);
    if(pItemOld != NULL) {
printf(__FILE__" %d\n",__LINE__);
        //We need to do some special display stuff
        m_pDisplay->removeItem(uiEndIndex);
printf(__FILE__" %d\n",__LINE__);
        m_pDisplay->addItem(pItemOld->getItemId(), uiStartIndex);
printf(__FILE__" %d\n",__LINE__);
    }

printf(__FILE__" %d\n",__LINE__);
    m_pDisplay->moveItem(uiStartIndex, uiEndIndex);
printf(__FILE__" %d\n",__LINE__);
}

void
Inventory::setInventoryDisplay(InventoryDisplay *pDisplay) {
    m_pDisplay = pDisplay;  //Assumes this is not NULL

    //Inform the display about already-existing items
    for(int i = 0; i < NUM_SPELL_ITEMS; ++i) {
        if(m_aSpellItems[i] != NULL) {
            m_pDisplay->addSpell(m_aSpellItems[i]->getItemId(), i);
        }
    }
    for(int i = 0; i < NUM_ELEMENT_ITEMS; ++i) {
        if(m_aElementItems[i] != NULL) {
            m_pDisplay->addSpell(m_aElementItems[i]->getItemId(), i);
        }
    }
    for(int i = 0; i < NUM_GENERAL_ITEMS; ++i) {
        if(m_aGeneralItems[i] != NULL) {
            m_pDisplay->addSpell(m_aGeneralItems[i]->getItemId(), i);
        }
    }
}
