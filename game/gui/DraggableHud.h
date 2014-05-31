#ifndef DRAGGABLEHUD_H
#define DRAGGABLEHUD_H

#include "Draggable.h"
#include "game/items/Item.h"
#include "d3re/ContainerRenderModel.h"
#include <map>
#include <list>

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


    void initHud();
    void updateItemAnimations();
    void clearInventory();
    void readInventory(const boost::property_tree::ptree &pt, const std::string &keyBase);
    void writeInventory(boost::property_tree::ptree &pt, const std::string &keyBase);

private:
    enum HudState {
        HUD_STATE_NORMAL,
        HUD_STATE_TYPE_SAVE_FILE,
        HUD_STATE_TYPE_LOAD_FILE,
        HUD_STATE_TYPE_NEW_FILE,
        HUD_STATE_QUIT,
        HUD_STATE_NUM_STATES
    };

    void initPlayerHud();
    void initHealthBarHud(ContainerRenderModel *panel);
    void initItemBarHud(ContainerRenderModel *panel);
    void initSideButtonHud(ContainerRenderModel *panel);
    void initSideTypeHud(ContainerRenderModel *panel);
    void initSideConfirmHud(ContainerRenderModel *panel);

    void prepSideTypeHud(const std::string &sMessageLabel, const std::string &sActionLabel, const std::string &sInactionLabel);
    void prepSideConfirmHud(const std::string &sMessageLabel, const std::string &sActionLabel, const std::string &sInactionLabel);
    void prepSideButtonHud();
    void disablePanel(ContainerRenderModel *panel);
    void enablePanel(ContainerRenderModel *panel);

    void removeScheduledItems();

    Rect indexToItemRect(uint index);
    Rect indexToSpellRect(uint index);
    Rect indexToElementRect(uint index);

    int typeOnKeyPress(InputData *data);

    uint m_uiId;
    HudState m_eState;

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

    //Some panels that may need to get swapped in or out
    ContainerRenderModel *m_pMainSidePanel;
    ContainerRenderModel *m_pTypeSidePanel;
    ContainerRenderModel *m_pConfirmSidePanel;
    ContainerRenderModel *m_pInventoryPanel;
    ContainerRenderModel *m_pCurSidePanel;

    std::string m_sInput;


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
