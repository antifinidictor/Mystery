#include "FluidOctree.h"
#define DEFAULT_VORTON_RADIUS 0.1
#include "d3re/d3re.h"
#include "mge/ConfigManager.h"
static bool s_bDisplayBounds;
static SDL_mutex *s_mxDebugPrint = SDL_CreateMutex();

FluidOctreeNode::FluidOctreeNode(FluidOctree *pRoot, uint uiNodeId, uint uiLevel, const Box &bxBounds, float fOctreeMinRes)
    :   Octree3dNode<Vorton>(uiNodeId, uiLevel, bxBounds, fOctreeMinRes),
        m_vrtAggregate(0, bxCenter(bxBounds), DEFAULT_VORTON_RADIUS, Vec3f()),
        m_pRoot(pRoot)
{
    //ctor
    s_bDisplayBounds = ConfigManager::get()->get("test.fluid.drawBounds", false);
}

FluidOctreeNode::~FluidOctreeNode()
{
    //dtor
}

void
FluidOctreeNode::updateContents(float fTime) {
    if(s_bDisplayBounds) {
        D3RE::get()->drawBox(m_bxBounds, Color(0, 255, 255));
    }

#if 1
    //Prepare to aggregate vortons
    m_vrtAggregate = Vorton(0, bxCenter(m_bxBounds), DEFAULT_VORTON_RADIUS, Vec3f());
    m_fTotalVortMag = 0.f;

    //Update each vorton
    for(objmap_iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        //To perform the update, we need the velocity and jacobian at the vorton's position
        Point ptPos = it->second->getPosition();
        Vec3f v3Velocity = m_pRoot->getVelocityGrid()->getAt(ptPos);
        Matrix<3,3> matJacobian = m_pRoot->getJacobianGrid()->getAt(ptPos);

        //Update the vorton
        it->second->update(fTime, matJacobian, v3Velocity);

        //Aggregate this vorton
        Vec3f v3ChildVorticity = it->second->getVorticity();
        float fChildVortMag = v3ChildVorticity.magnitude();

        m_fTotalVortMag += fChildVortMag;                   //Increase the total vorticity represented here
        m_vrtAggregate.moveBy(ptPos * fChildVortMag);       //Add to the vorton position (will be normalized later)
        m_vrtAggregate.accumVorticity(v3ChildVorticity);

        //Does the vorton need to get moved to a different quadrant?
        char dirs = bxOutOfBounds(it->second->getBounds(), m_bxBounds);
        if(dirs) {
            //Leaves bounds in at least one direction
            m_lsObjsToRemove.push_back(it->first);
            m_lsObjsLeftQuadrant.push_back(it->second);
printf(__FILE__" %d: Vorton %d left quadrant %x (level %d, dirs %x)\n",
    __LINE__,
    it->first,
    m_uiNodeId,
    m_uiLevel,
    dirs
);
        } else if(addToChildren(it->second)) {
            //If the object can be added to a child's list, then it should be removed
            m_lsObjsToRemove.push_back(it->first);
        }


        //Debug displays
        if(s_bDisplayBounds) {
            if(bxOutOfBounds(it->second->getBounds(), m_bxBounds)) {
                D3RE::get()->drawCircle(ptPos, 0.1f, Color(255,128,128));
            } else {
                D3RE::get()->drawCircle(ptPos, 0.1f, Color(128,255,128));
            }
        }
    }

    bool first = true;
    printf(__FILE__" %d: Quadrant %x (level %d) has vortons left %s = ",__LINE__, m_uiNodeId, m_uiLevel, m_lsObjsLeftQuadrant.size() > 0 ? "SOME" : "NONE");
    for(Octree3dNode<Vorton>::objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        if(first) {
            first = false;
            printf("%d",(*it)->getId());
        } else {
            printf(", %d", (*it)->getId());
        }
    }
    printf("\n");
#else

    //Display each vorton
    if(s_bDisplayBounds) {
        for(objmap_iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
            Point ptPos = it->second->getPosition();
            D3RE::get()->drawCircle(ptPos, 0.1f, Color(128,255,128));
        }
    }
#endif

    //In addition, all children's aggregate vortons will be aggregated into my vorton.
}

void
FluidOctreeNode::handleChildUpdateResults(Octree3dNode<Vorton> *node, int q) {
    FluidOctreeNode *child = (FluidOctreeNode*)node;

    //The child's aggregate vorticity has been calculated, but the aggregate
    // vorton's position has not been divided by the child's total vorton
    // magnitude yet.  The root will have to have this done specially.
    child->m_vrtAggregate.dividePosition(child->m_fTotalVortMag);

    //Now that's done, it's safe to aggregate the child's vorton
    Vec3f v3ChildVorticity = child->m_vrtAggregate.getVorticity();
    float fChildVortMag = v3ChildVorticity.magnitude();

    m_fTotalVortMag += fChildVortMag;                                           //Increase the total vorticity represented here
    m_vrtAggregate.moveBy(child->m_vrtAggregate.getPosition() * fChildVortMag); //Add to the position (will be normalized later)
    m_vrtAggregate.accumVorticity(v3ChildVorticity);                            //Add to the vorticity
}

Octree3dNode<Vorton> *
FluidOctreeNode::createChild(uint childId, const Box &bxBounds) const {
    //Calculate the node id
    uint uiNewLevel = m_uiLevel + 1;
    uint uiNewNodeId = (childId << (uiNewLevel * 4)) | m_uiNodeId;

    FluidOctreeNode *node;
    //if(childIsLeafNode(bxBounds)) {
        node = new FluidOctreeNode(
            m_pRoot,
            uiNewNodeId,
            uiNewLevel,
            bxBounds,
            m_fMinResolution
        );
        printf(__FILE__" %d: Created child!\n", __LINE__);
    /*
    } else {
        node = new FluidOctreeLeaf(
            m_pRoot,
            uiNewNodeId,
            uiNewLevel,
            bxBounds,
            m_fMinResolution
        );
    }
    */
    return node;
}

/*
void
FluidOctreeNode::onAdd(Vorton *obj) {
}

void
FluidOctreeNode::onRemove(Vorton *obj) {
}

void
FluidOctreeNode::onErase(Vorton *obj) {
}
*/

void
FluidOctreeNode::positionSearchForVelocity(const Point &ptPosition, Vec3f &v3Velocity) {
    //Velocity is calculated from vorticities.  To reduce the calculation
    // complexity, we use the aggregate vorticities of nodes where the selected
    // point is not inside.  Since the point is only going to be inside one
    // child, we do not have to use dfs to follow the point down, but must
    // simply remember which point is next.
    uint dirs = ptOutOfBounds(ptPosition, m_bxBounds);
    if(dirs) {
        //Not in the octree at all, use the aggregate vorton
        v3Velocity += m_vrtAggregate.velocityAt(ptPosition);
#if DEBUG_VORTONS
printf(__FILE__" %d: (%f,%f,%f)\n", __LINE__, v3Velocity.x, v3Velocity.y, v3Velocity.z);
#endif
    } else {
        //In the octree, descend and find the appropriate nodes
        FluidOctreeNode *curNode = this;
        do {
            //Get the total velocity due to the contents of this node
            for(objmap_iter_t it = curNode->m_mContents.begin(); it != curNode->m_mContents.end(); ++it) {
                v3Velocity += it->second->velocityAt(ptPosition);
#if DEBUG_VORTONS
printf(__FILE__" %d: (%f,%f,%f)\n", __LINE__, v3Velocity.x, v3Velocity.y, v3Velocity.z);
#endif
            }

            //Get the total velocity due to the vortons that are children of this node
            FluidOctreeNode *nextNode = NULL;
            for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
                FluidOctreeNode *child = (FluidOctreeNode*)curNode->m_apChildren[q];
                if(child != NULL) {
                    //Is the point inside this child?
                    uint dirs = ptOutOfBounds(ptPosition, child->m_bxBounds);
                    if(dirs) {
                        //Not in this area, use aggregate vorton
                        v3Velocity += child->m_vrtAggregate.velocityAt(ptPosition);
#if DEBUG_VORTONS
printf(__FILE__" %d: (%f,%f,%f)\n", __LINE__, v3Velocity.x, v3Velocity.y, v3Velocity.z);
#endif
                    } else {
                        //We are in this node.  We will search the contents
                        nextNode = child;
#if DEBUG_VORTONS
printf(__FILE__" %d: (%f,%f,%f)\n", __LINE__, v3Velocity.x, v3Velocity.y, v3Velocity.z);
#endif
                    }
                }
            }
#if DEBUG_VORTONS
printf(__FILE__" %d: (%f,%f,%f)\n", __LINE__, v3Velocity.x, v3Velocity.y, v3Velocity.z);
#endif
            curNode = nextNode;
        } while(curNode != NULL);
#if DEBUG_VORTONS
printf(__FILE__" %d: (%f,%f,%f)\n", __LINE__, v3Velocity.x, v3Velocity.y, v3Velocity.z);
#endif
    }
}


/*
 * FluidOctree
 */
FluidOctree::FluidOctree(Positionable *parent, uint uiNodeId, const Box &bxBounds, float fOctreeMinRes, float fVelocityMinRes, float fJacobianMinRes)
    :   FluidOctreeNode(this, uiNodeId, 0, bxBounds, fOctreeMinRes),
        m_igVelocities(parent, bxBounds, fVelocityMinRes),
        m_igJacobians(parent, bxBounds, fJacobianMinRes),
        m_pParent(parent),
        m_uiVelocityUpdateCount(0),
        m_uiJacobianUpdateCount(0),
        m_mxUpdateCounts(SDL_CreateMutex()),
        m_cdWaitVelocities(SDL_CreateCond()),
        m_cdWaitJacobians(SDL_CreateCond())
{
    //ctor
}

FluidOctree::~FluidOctree()
{
    //dtor
}

void
FluidOctree::update(float fTime) {
    //Perform a normal update
    FluidOctreeNode::update(fTime);

    //Add vortons back to contents
    for(objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        m_mContents[(*it)->getId()] = (*it);
    }

    //Clean lists and update aggregate vorton position
    m_vrtAggregate.dividePosition(m_fTotalVortMag);
    cleanResults();

    //Velocities and vorticities are calculated on fluid updates.
    // Note that to avoid concurrent read/write accesses, the following
    // operations cannot happen concurrently:
    // (1) Velocity calculation
    // (2) Jacobian calculation
    // (3) Octree update (vortons shared, updated, aggregated)
}

void
FluidOctree::computeVelocityAt(int x, int y, int z) {
    Vec3f &v3Velocity = m_igVelocities.at(x,y,z);
    Point ptPosition = m_igVelocities.toPosition(x,y,z);
    v3Velocity = Point();

    //This had to be moved to a separate function in order to deal with
    // inheritence of protected member problems
    positionSearchForVelocity(ptPosition, v3Velocity);
}

void
FluidOctree::computeJacobianAt(int x, int y, int z) {
    //Jacobians can't be calculated entirely locally because they require information from surrounding grid cells
    // Also note that this results in redundant calculations due to threading
    Box bxBounds = m_igJacobians.getBounds();
    int iSizeX = m_igJacobians.getSizeX();
    int iSizeY = m_igJacobians.getSizeY();
    int iSizeZ = m_igJacobians.getSizeZ();
    float fCellW = bxBounds.w / iSizeX;
    float fCellH = bxBounds.h / iSizeY;
    float fCellL = bxBounds.l / iSizeZ;

    //This changes at the cell boundaries
    float fCellWDivisor;
    if(0 < x && x < iSizeX - 1) {
        fCellWDivisor = 2 * fCellW;
    } else {
        fCellWDivisor = fCellW;
    }
    float fCellHDivisor;
    if(0 < y && y < iSizeY - 1) {
        fCellHDivisor = 2 * fCellH;
    } else {
        fCellHDivisor = fCellH;
    }
    float fCellLDivisor;
    if(0 < z && z < iSizeZ - 1) {
        fCellLDivisor = 2 * fCellL;
    } else {
        fCellLDivisor = fCellL;
    }

    //Calculate the jacobian at this point
    Mat33 &matJacobian = m_igJacobians.at(x,y,z);
    const Vec3f &v3VelocityXMY0Z0 = m_igVelocities.at(x-1,y,  z);
    const Vec3f &v3VelocityXPY0Z0 = m_igVelocities.at(x+1,y,  z);

    const Vec3f &v3VelocityX0YMZ0 = m_igVelocities.at(x,  y-1,z);
    const Vec3f &v3VelocityX0YPZ0 = m_igVelocities.at(x,  y+1,z);

    const Vec3f &v3VelocityX0Y0ZM = m_igVelocities.at(x,  y,  z-1);
    const Vec3f &v3VelocityX0Y0ZP = m_igVelocities.at(x,  y,  z+1);

#if DEBUG_VORTONS
SDL_LockMutex(s_mxDebugPrint);
printf(__FILE__" %d:[[%2.2f,%2.2f,%2.2f]\n", __LINE__,
    matJacobian[0][0], matJacobian[0][1], matJacobian[0][2]
);
printf(__FILE__" %d: [%2.2f,%2.2f,%2.2f]\n", __LINE__,
    matJacobian[1][0], matJacobian[1][1], matJacobian[1][2]
);
printf(__FILE__" %d: [%2.2f,%2.2f,%2.2f]]\n", __LINE__,
    matJacobian[2][0], matJacobian[2][1], matJacobian[2][2]
);
printf("Some factors: w/h/l %f/%f/%f; xm=%f, xp=%f, ym=%f, yp=%f, zm=%f, zp=%f\n",
    fCellWDivisor, fCellLDivisor, fCellHDivisor,
    v3VelocityXMY0Z0.magnitude(),
    v3VelocityXPY0Z0.magnitude(),
    v3VelocityX0YMZ0.magnitude(),
    v3VelocityX0YPZ0.magnitude(),
    v3VelocityX0Y0ZM.magnitude(),
    v3VelocityX0Y0ZP.magnitude()
);
SDL_UnlockMutex(s_mxDebugPrint);
#endif
    //Each row is made by combining x, y, z velocities
    #define X 0
    #define Y 1
    #define Z 2
    matJacobian[X][X] = (v3VelocityXPY0Z0.x - v3VelocityXMY0Z0.x) / fCellWDivisor;
    matJacobian[X][Y] = (v3VelocityXPY0Z0.y - v3VelocityXMY0Z0.y) / fCellWDivisor;
    matJacobian[X][Z] = (v3VelocityXPY0Z0.z - v3VelocityXMY0Z0.z) / fCellWDivisor;

    matJacobian[Y][X] = (v3VelocityX0YPZ0.x - v3VelocityX0YMZ0.x) / fCellHDivisor;
    matJacobian[Y][Y] = (v3VelocityX0YPZ0.y - v3VelocityX0YMZ0.y) / fCellHDivisor;
    matJacobian[Y][Z] = (v3VelocityX0YPZ0.z - v3VelocityX0YMZ0.z) / fCellHDivisor;

    matJacobian[Z][X] = (v3VelocityX0Y0ZP.x - v3VelocityX0Y0ZM.x) / fCellLDivisor;
    matJacobian[Z][Y] = (v3VelocityX0Y0ZP.y - v3VelocityX0Y0ZM.y) / fCellLDivisor;
    matJacobian[Z][Z] = (v3VelocityX0Y0ZP.z - v3VelocityX0Y0ZM.z) / fCellLDivisor;

#if DEBUG_VORTONS
SDL_LockMutex(s_mxDebugPrint);
printf(__FILE__" %d:[[%2.2f,%2.2f,%2.2f]\n", __LINE__,
    matJacobian[0][0], matJacobian[0][1], matJacobian[0][2]
);
printf(__FILE__" %d: [%2.2f,%2.2f,%2.2f]\n", __LINE__,
    matJacobian[1][0], matJacobian[1][1], matJacobian[1][2]
);
printf(__FILE__" %d: [%2.2f,%2.2f,%2.2f]]\n", __LINE__,
    matJacobian[2][0], matJacobian[2][1], matJacobian[2][2]
);
SDL_UnlockMutex(s_mxDebugPrint);
#endif
}
