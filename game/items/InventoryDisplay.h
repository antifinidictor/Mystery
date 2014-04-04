#ifndef INVENTORY_DISPLAY_H
#define INVENTORY_DISPLAY_H

class InventoryDisplay {
public:
    virtual void addItem(uint itemId, uint invIndex) = 0;
    virtual void addSpell(uint itemId, uint invIndex) = 0;
    virtual void addElement(uint itemId, uint invIndex) = 0;
    virtual void removeItem(uint invIndex) = 0;
    virtual void removeSpell(uint invIndex) = 0;
    virtual void removeElement(uint invIndex) = 0;
    virtual void setCurrentItem(uint itemId) = 0;
    virtual void setCurrentSpell(uint itemId) = 0;
    virtual void setCurrentElement(uint itemId) = 0;
    virtual void moveItem(uint startIndex, uint endIndex) = 0;
    virtual void moveSpell(uint startIndex, uint endIndex) = 0;
    virtual void moveElement(uint startIndex, uint endIndex) = 0;
};


#endif // INVENTORY_DISPLAY_H
