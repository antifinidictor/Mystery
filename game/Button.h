/*
 * Button.h
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "mge/defs.h"
#include "game/Clickable.h"
#include "ore/OrderedRenderModel.h"
#include "tpe/TimePhysicsModel.h"

class Button : public Clickable {
public:
    //Constructor(s)/Destructor
    Button(uint uiID, Image *img, Point pos);
    Button(uint uiID, Image *img, Point pos, const char* text);
    virtual ~Button();

    //General
    virtual bool update(uint time);

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

protected:
    virtual Rect getResponseArea() { return m_pButtonRenderModel->getDrawArea(); }

private:
    TimePhysicsModel   *m_pPhysicsModel;
    RenderModel        *m_pRenderModel;
    OrderedRenderModel *m_pButtonRenderModel;
};

#endif
