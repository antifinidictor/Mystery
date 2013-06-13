/*
 * ClickableItem.h
 * Very similar to the button, except it has no control over its image- everything is
 * controlled by the listener class.
 */

#ifndef CLICKABLE_ITEM_H
#define CLICKABLE_ITEM_H

#include "mge/defs.h"
#include "game/Clickable.h"
#include "ore/OrderedRenderModel.h"
#include "tpe/TimePhysicsModel.h"

class ClickableItem : public Clickable {
public:
    //Constructor(s)/Destructor
    ClickableItem(uint uiID, Image *img, Point pos, bool bFreeListener);
    virtual ~ClickableItem();

    //General
    virtual bool update(uint time);

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

protected:
    virtual Rect getResponseArea() { return m_pRenderModel->getDrawArea(); }

private:
    TimePhysicsModel   *m_pPhysicsModel;
    OrderedRenderModel *m_pRenderModel;
    bool m_bFreeListener;
};

#endif
