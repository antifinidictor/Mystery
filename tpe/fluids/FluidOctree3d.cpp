#include "FluidOctree3d.h"
#include "mge/GameObject.h"
#include "tpe/tpe.h"

using namespace std;

#define DEFAULT_VORTON_RADIUS 0.1

FluidOctreeNode::FluidOctreeNode(const Box &bxBounds, float fMinResolution)
    : m_vrtAggregate(0, bxCenter(bxBounds), DEFAULT_VORTON_RADIUS, Point())
{
    m_bEmpty = true;
    m_bxBounds = bxBounds;
    m_fMinResolution = fMinResolution;

    //Children are allocated as needed when objects are added
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        m_apChildren[q] = NULL;
    }
}

FluidOctreeNode::~FluidOctreeNode() {
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL) {
            delete m_apChildren[q];
            m_apChildren[q] = NULL;
        }
    }
    iter_t it;
    for(it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        delete it->second;
    }
}

//Adds object to the appropriate list
bool
FluidOctreeNode::add(GameObject *obj, bool bForce) {
    Box bxObjBounds = obj->getPhysicsModel()->getCollisionVolume();
    bool bCanAdd = false;

    char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);
    if(dirs) {
        //It doesn't fit in me, but if forced I will add it to me
        // Only the top-level may be forced to add
        if(bForce) {
            m_lsObjsToAdd.push_back(obj);
            bCanAdd = true;
        }

    } else {
        //Try recursively adding to children
        bCanAdd = addToChildren(obj);

        //We add it to us if none of our children could accept it, but it was within our bounds
        if(!bCanAdd) {
            m_lsObjsToAdd.push_back(obj);
            bCanAdd = true;
        }
    }
    return bCanAdd;
}

//Removes from the list but does not call delete
bool
FluidOctreeNode::remove(uint uiObjId) {
    iter_t obj = m_mContents.find(uiObjId);
    if(obj != m_mContents.end()) {
        //m_mContents.erase(obj);
        //updateEmptiness();
        m_lsObjsToRemove.push_back(uiObjId);
        return true;
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            if(m_apChildren[q]->remove(uiObjId)) {
                //Successfully erased from child
                return true;
            }
        }
    }

    return false;
}

//Removes from the list and calls delete
bool
FluidOctreeNode::erase(uint uiObjId) {
    iter_t obj = m_mContents.find(uiObjId);
    if(obj != m_mContents.end()) {
        //delete obj->second;
        //m_mContents.erase(obj);
        //updateEmptiness();
        m_lsObjsToErase.push_back(uiObjId);
        return true;
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            if(m_apChildren[q]->erase(uiObjId)) {
                //Child contains this object and it will be erased
                return true;
            }
        }
    }

    return false;
}

//Returns a reference to the appropriate object
GameObject *
FluidOctreeNode::find(uint uiObjId) {
    iter_t obj = m_mContents.find(uiObjId);
    if(obj != m_mContents.end()) {
        return obj->second;
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            GameObject *obj = m_apChildren[q]->find(uiObjId);
            if(obj != NULL) {
                return obj;
            }
        }
    }

    return NULL;
}



void
FluidOctreeNode::update(float fTime) {
    //Update internal container elements
    updateContents(fTime);

    //Deal with childrens' update-results
    handleChildrenUpdateResults();

    //Erase queued objects from the container
    for(list<uint>::iterator itObjId = m_lsObjsToErase.begin(); itObjId != m_lsObjsToErase.end(); ++itObjId) {
        eraseNow(*itObjId);
    }
    m_lsObjsToErase.clear();

    //Remove queued objects from the container
    for(list<uint>::iterator itObjId = m_lsObjsToRemove.begin(); itObjId != m_lsObjsToRemove.end(); ++itObjId) {
        removeNow(*itObjId);
    }
    m_lsObjsToRemove.clear();

    //Add queued objects to the container
    for(list<GameObject*>::iterator itObj = m_lsObjsToAdd.begin(); itObj != m_lsObjsToAdd.end(); ++itObj) {
        addNow(*itObj);
    }
    m_lsObjsToAdd.clear();
    updateEmptiness();
}


void
FluidOctreeNode::updateContents(float fTime) {
    TimePhysicsEngine *pe = TimePhysicsEngine::get();
    for(iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        //Update the object's logic
        if(it->second->update(fTime)) {
            //Object requested deletion
            m_lsObjsToErase.push_back(it->first);
            continue;
        }

        //Update physics
        bool bHasMoved = pe->applyPhysics(it->second);

        //If the object has moved, add it to the appropriate list so collision
        //checks can be applied later
        if(bHasMoved) {
            //Object has moved.  We will deal with the case where it can be pushed lower in the tree later
            m_lsDynamicObjs.push_back(it->second);

            //If the object has left the bounds, then some additional collision
            // processing must be done against its neighbors.  Notice that objs
            // that left the quadrant are also referenced as dynamic objects
            Box bxObjBounds = it->second->getPhysicsModel()->getCollisionVolume();
            char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);

            if(dirs) {
                //Leaves bounds in at least one direction
                m_lsObjsToRemove.push_back(it->first);
                m_lsObjsLeftQuadrant.push_back(it->second);
            } else {
                //If the object can be added to a child's list, then it should be removed
                if(addToChildren(it->second)) {
                    m_lsObjsToRemove.push_back(it->first);
                }
            }
        } else {
            //All dynamic objects need to be checked against it
            // (all children's, my own, my parents)
            m_lsStaticObjs.push_back(it->second);

            //Collision check against my dynamic objs
            for(objlist_iter_t mv = m_lsDynamicObjs.begin(); mv != m_lsDynamicObjs.end(); ++mv) {
                pe->applyPhysics(it->second, (*mv));
            }
        }
    }

    //Collision check the dynamic objects against contents they had not yet been checked against
    for(objlist_iter_t mv = m_lsDynamicObjs.begin(); mv != m_lsDynamicObjs.end(); ++mv) {

        for(iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
            if(it->second->getId() == (*mv)->getId()) {
                break;
            }
            pe->applyPhysics(it->second, *mv);
        }
    }
}

void
FluidOctreeNode::handleChildrenUpdateResults() {
    TimePhysicsEngine *pe = TimePhysicsEngine::get();
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL) {
            //To get here, the child must have already been updated
            //Perform collision checks on relevant lists
            //Dynamic objects from child must be checked against my static objects
            pe->applyCollisionPhysics(m_apChildren[q]->m_lsDynamicObjs, m_lsStaticObjs);

            //Static objects from child must be checked against my dynamic objects
            pe->applyCollisionPhysics(m_apChildren[q]->m_lsStaticObjs, m_lsDynamicObjs);

            //Dynamic objects from child must be checked against my dynamic objects
            pe->applyCollisionPhysics(m_apChildren[q]->m_lsDynamicObjs, m_lsDynamicObjs);

            //Objects that left the child's quadrant need to be checked against
            // the other children
            for(int q2 = q + 1; q2 < QUAD_NUM_QUADS; ++q2) {
                if(m_apChildren[q2] != NULL) {
                    pe->applyCollisionPhysics(
                        m_apChildren[q]->m_lsObjsLeftQuadrant,
                        m_apChildren[q]->m_lsDynamicObjs
                    );
                    pe->applyCollisionPhysics(
                        m_apChildren[q]->m_lsObjsLeftQuadrant,
                        m_apChildren[q]->m_lsStaticObjs
                    );
                }
            }

            //Augment my lists by children's lists
            //Objects that left the child's quadrant need to be added to other lists
            for(objlist_iter_t itObjLeftChild = m_apChildren[q]->m_lsObjsLeftQuadrant.begin();
                    itObjLeftChild != m_apChildren[q]->m_lsObjsLeftQuadrant.end();
                    ++itObjLeftChild)
            {

                Box bxObjBounds = (*itObjLeftChild)->getPhysicsModel()->getCollisionVolume();
                char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);

                if(dirs) {
                    //Object left my quadrant too
                    m_lsObjsLeftQuadrant.push_back(*itObjLeftChild);
                } else {
                    //Object is in my quadrant, so add it
                    m_lsObjsToAdd.push_back(*itObjLeftChild);
                }
            }

            //Copy child lists into my lists
            m_lsDynamicObjs.insert(
                m_lsDynamicObjs.end(),
                m_apChildren[q]->m_lsDynamicObjs.begin(),
                m_apChildren[q]->m_lsDynamicObjs.end()
            );
            m_lsStaticObjs.insert(
                m_lsStaticObjs.end(),
                m_apChildren[q]->m_lsStaticObjs.begin(),
                m_apChildren[q]->m_lsStaticObjs.end()
            );

            //Clear child lists
            m_apChildren[q]->cleanResults();
        }
    }
}


bool
FluidOctreeNode::addToChildren(GameObject *obj) {
    //Try adding this object to my children.  Note that they won't actually get
    // added to the contents here, only scheduled for addition.
    Box bxObjBounds = obj->getPhysicsModel()->getCollisionVolume();

    bool bSomeChildCanAdd = false;
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        //Two cases: Child exists, and child does not exist.
        //If the child does not exist we may want to create it.
        if(m_apChildren[q] == NULL) {
            //See if we want to make a new child (can create child bounds & obj is inside bounds)
            Box bxChildBounds;
            //If we could make a valid child here and it would be inside this child
            if(getChildBounds(q, bxChildBounds) && bxOutOfBounds(bxObjBounds, bxChildBounds) == 0) {
                //Create a new child octree here
                if(childIsLeafNode(bxChildBounds)) {
                    //Leaf octree node
                    m_apChildren[q] = new FluidOctreeLeaf(bxChildBounds, m_fMinResolution);

                } else {
                    //Generic octree node
                    m_apChildren[q] = new FluidOctreeNode(bxChildBounds, m_fMinResolution);
                }

                //Add to child
                bSomeChildCanAdd = m_apChildren[q]->add(obj, false);
                break;
            }
        } else if(m_apChildren[q]->add(obj, false)) {
            //Child does exist
            bSomeChildCanAdd = true;
            break;
        }
    }
    return bSomeChildCanAdd;
}


void
FluidOctreeNode::addNow(GameObject *obj) {
    m_mContents[obj->getId()] = obj;
}

void
FluidOctreeNode::removeNow(uint uiObjId) {
    iter_t itFoundObj = m_mContents.find(uiObjId);
    if(itFoundObj != m_mContents.end()) {
        m_mContents.erase(itFoundObj);
    } else {
        printf(__FILE__" %d ERROR: Failed to remove object %d from octree; obj does not exist\n", __LINE__, uiObjId);
    }
}

void
FluidOctreeNode::eraseNow(uint uiObjId) {
    iter_t itFoundObj = m_mContents.find(uiObjId);
    if(itFoundObj != m_mContents.end()) {
        delete itFoundObj->second;
        m_mContents.erase(itFoundObj);
    } else {
        printf(__FILE__" %d ERROR: Failed to erase object %d from octree; obj does not exist\n", __LINE__, uiObjId);
    }
}

bool
FluidOctreeNode::getChildBounds(int iQuadName, Box &bx) {
    //Half-widths
    float hw = m_bxBounds.w / 2;
    float hh = m_bxBounds.h / 2;
    float hl = m_bxBounds.l / 2;

    if(hw < m_fMinResolution && hh < m_fMinResolution && hl < m_fMinResolution) {
        //The parent of this 'child' is really a leaf node
        return false;
    }

    if((iQuadName & QUAD_X_MASK) != 0) {  //negative
        bx.x = m_bxBounds.x;
        bx.w = (hw < m_fMinResolution) ? m_bxBounds.w : hw;
    } else {                            //positive
        if(hw < m_fMinResolution) {
            //Invalid child: Positive
            return false;
        } else {
            bx.x = m_bxBounds.x + hw;
            bx.w = hw;
        }
    }

    if((iQuadName & QUAD_Y_MASK) != 0) {  //negative
        bx.y = m_bxBounds.y;
        bx.h = (hh < m_fMinResolution) ? m_bxBounds.h : hh;
    } else {                            //positive
        if(hh < m_fMinResolution) {
            //Invalid child: Positive
            return false;
        } else {
            bx.y = m_bxBounds.y + hh;
            bx.h = hh;
        }
    }

    if((iQuadName & QUAD_Z_MASK) != 0) {  //negative
        bx.z = m_bxBounds.z;
        bx.l = (hl < m_fMinResolution) ? m_bxBounds.l : hl;
    } else {                            //positive
        if(hl < m_fMinResolution) {
            //Invalid child: Positive
            return false;
        } else {
            bx.z = m_bxBounds.z + hl;
            bx.l = hl;
        }
    }

    //If we make it here, the child is valid
    return true;
}


bool
FluidOctreeNode::childIsLeafNode(const Box &bxChildBounds) {
    float hw = bxChildBounds.w / 2;
    float hh = bxChildBounds.h / 2;
    float hl = bxChildBounds.l / 2;
    return (hw < m_fMinResolution && hh < m_fMinResolution && hl < m_fMinResolution);
}

void
FluidOctreeNode::updateEmptiness() {
    m_bEmpty = m_mContents.size() == 0;
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL) {
            //Only empty if we have nothing and children are empty too
            m_bEmpty = m_bEmpty && m_apChildren[q]->empty();
        }
    }
}


/*
 * FluidOctreeLeafNode
 */

FluidOctreeLeaf::FluidOctreeLeaf(const Box &bxBounds, float fMinResolution)
    : FluidOctreeNode(bxBounds, fMinResolution)
{
}


bool
FluidOctreeLeaf::add(GameObject *obj, bool bForce) {
    Box bxObjBounds = obj->getPhysicsModel()->getCollisionVolume();
    bool bCanAdd = bForce;

    char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);
    if(dirs) {
        //It doesn't fit in me, but if forced I will add it to me
        // Only the top-level can be forced to add
        if(bForce) {
            m_lsObjsToAdd.push_back(obj);
        }

    } else {
        //Since it was in our bounds, we could add it to our list
        m_lsObjsToAdd.push_back(obj);
        bCanAdd = true;
    }
    return bCanAdd;
}
