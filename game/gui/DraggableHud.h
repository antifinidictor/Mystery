#ifndef DRAGGABLEHUD_H
#define DRAGGABLEHUD_H

#include "Draggable.h"
#include "game/items/InventoryDisplay.h"
#include <map>
#include <list>

class Inventory;
class Item;
class Listener;

class DraggableHud : public InventoryDisplay, public Draggable
{
public:
    DraggableHud(uint uiId);
    virtual ~DraggableHud();

    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase) {}

	//virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);
    virtual void onFollow(const Point &diff);
    virtual void onStartDragging();
    virtual void onEndDragging();

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    ContainerRenderModel *getHudContainer() { return m_pRenderModel; }


    virtual void moveBy(const Point &ptShift);

    //From InventoryDisplay
    virtual void addItem(uint itemId, uint invIndex);
    virtual void addSpell(uint itemId, uint invIndex);
    virtual void addElement(uint itemId, uint invIndex);
    virtual void removeItem(uint invIndex);
    virtual void removeSpell(uint invIndex);
    virtual void removeElement(uint invIndex);
    virtual void setCurrentItem(uint itemId);
    virtual void setCurrentSpell(uint itemId);
    virtual void setCurrentElement(uint itemId);
    virtual void moveItem(uint startIndex, uint endIndex);
    virtual void moveSpell(uint startIndex, uint endIndex);
    virtual void moveElement(uint startIndex, uint endIndex);

    void registerPlayer(Listener *pPlayer) { m_pMyPlayer = pPlayer; }

    void updateItemAnimations();

private:

    void initPlayerHud();
    void initHealthBarHud(ContainerRenderModel *panel);
    void initItemBarHud(ContainerRenderModel *panel);
    void initSideButtonHud(ContainerRenderModel *panel);

    void removeScheduledItems();

    float indexToItemX(uint index);
    float indexToItemY(uint index);

    float indexToSpellX(uint index);
    float indexToSpellY(uint index);

    float indexToElementX(uint index);
    float indexToElementY(uint index);

    ContainerRenderModel *m_pRenderModel;

    bool m_bHidden;
    int m_iAnimTimer;
    int m_iFrame;
    Listener *m_pMyPlayer;
    std::map<uint, Draggable*> m_mItems;
    std::map<uint, Draggable*> m_mSpells;
    std::map<uint, Draggable*> m_mElements;
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
