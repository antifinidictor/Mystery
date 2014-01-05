#ifndef DRAGGABLEHUD_H
#define DRAGGABLEHUD_H

#include "Draggable.h"

class Inventory;

class DraggableHud : public Draggable
{
public:
    DraggableHud(uint uiId);
    virtual ~DraggableHud();

    virtual void write(boost::property_tree::ptree &pt, const std::string &keyBase) {}

	//virtual int callBack(uint uiEventHandlerId, void *data, uint uiEventId);
    virtual void onFollow(const Point &diff);
    virtual void onStartDragging();
    virtual void onEndDragging();

    ContainerRenderModel *getHudContainer() { return m_pRenderModel; }

    void updateItemAnimations(Inventory *inv);

protected:
private:
    bool m_bHidden;
    int m_iAnimTimer;
    int m_iFrame;
};

#endif // DRAGGABLEHUD_H
