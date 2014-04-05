#ifndef POSITIONABLE_H
#define POSITIONABLE_H

class Positionable {
public:
    virtual ~Positionable() {}
    virtual Point getPosition() = 0;
    virtual void  moveBy(const Point &ptShift) = 0;    //non-physics shift
};

#endif // POSITIONABLE_H
