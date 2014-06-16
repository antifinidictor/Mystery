#ifndef WORKLIST_ITEM_H
#define WORKLIST_ITEM_H

class WorklistItem {
public:
    virtual ~WorklistItem() {}

    //Should store any data internally
    virtual void update() = 0;
};

#endif
