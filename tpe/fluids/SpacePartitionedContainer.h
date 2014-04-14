#ifndef SPACE_PARTITIONED_CONTAINER
#define SPACE_PARTITIONED_CONTAINER

#include "mge/defs.h"

template<class ItemT>
class SpacePartitionedContainer {
public:
    virtual ItemT getAt(const Point &ptPos) = 0;
};

#endif // SPACE_PARTITIONED_CONTAINER
