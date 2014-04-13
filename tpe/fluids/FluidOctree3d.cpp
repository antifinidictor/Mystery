#include "FluidOctree3d.h"

FluidOctree3d::FluidOctree3d(const Box &bxBounds, float fMinResolution) {
    m_bEmpty = true;
    m_bxBounds = bxBounds;
    m_fMinResolution = fMinResolution;

    //Children are allocated as needed when objects are added
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        m_aChildren[q] = NULL;
    }
}

FluidOctree3d::~FluidOctree3d() {
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_aChildren[q] != NULL) {
            delete m_aChildren[q];
            m_aChildren[q] = NULL;
        }
    }
    iter_t it;
    for(it = m_mObjs.begin(); it != m_mObjs.end(); ++it) {
        delete it->second;
    }
}

//Adds object to the appropriate list
bool
FluidOctree3d::add(Vorton *obj, bool bForce) {
    Box bxObjBounds = obj->getBounds();
    uint uiObjId = obj->getId();
    bool bAdded = false;

    char dirs = bxOutOfBounds(bxObjBounds, m_bxBounds);
    if(dirs) {
        //It doesn't fit in me, but if forced I will add it to me
        // Only the top-level can be forced to add
        if(bForce) {
            m_mObjs[uiObjId] = obj;
            m_bEmpty = false;
            bAdded = true;
        }

    } else {
        //Try recursively adding to children
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_aChildren[q] == NULL) {
                //We may want to create a new octree child here
                Box bxChildBounds;
                //If we could make a valid child here and it would be inside this child
                if(getChildBounds(q, bxChildBounds) && bxOutOfBounds(bxObjBounds, bxChildBounds) == 0) {
                    //Create a new child octree here
                    m_aChildren[q] = new FluidOctree3d(bxChildBounds, m_fMinResolution);
                    m_aChildren[q]->add(obj, false);    //TODO: If this returns false, we have a massive error
                    m_bEmpty = false;
                    bAdded = true;
                    break;
                }
            } else if(m_aChildren[q]->add(obj, false)) {
                m_bEmpty = false;
                bAdded = true;
                break;
            }
        }
        if(!bAdded) {
            //We add it to us if none of our children could accept it, but it was within our bounds
            m_mObjs[uiObjId] = obj;
            m_bEmpty = false;
            bAdded = true;
        }
    }
    return bAdded;
}

//Removes from the list but does not call delete
bool
FluidOctree3d::remove(uint uiObjId) {
    iter_t obj = m_mObjs.find(uiObjId);
    if(obj != m_mObjs.end()) {
        m_mObjs.erase(obj);
        updateEmptiness();
        return true;
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_aChildren[q] != NULL && !m_aChildren[q]->empty()) {
            if(m_aChildren[q]->remove(uiObjId)) {
                //Successfully erased from child
                return true;
            }
        }
    }

    return false;
}

//Removes from the list and calls delete
bool
FluidOctree3d::erase(uint uiObjId) {
    iter_t obj = m_mObjs.find(uiObjId);
    if(obj != m_mObjs.end()) {
        delete obj->second;
        m_mObjs.erase(obj);
        updateEmptiness();
        return true;
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_aChildren[q] != NULL && !m_aChildren[q]->empty()) {
            if(m_aChildren[q]->erase(uiObjId)) {
                //Successfully erased from child
                return true;
            }
        }
    }

    return false;
}

//Returns a reference to the appropriate object
Vorton *
FluidOctree3d::find(uint uiObjId) {
    iter_t obj = m_mObjs.find(uiObjId);
    if(obj != m_mObjs.end()) {
        return obj->second;
    }

    //Otherwise, search children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_aChildren[q] != NULL && !m_aChildren[q]->empty()) {
            Vorton *obj = m_aChildren[q]->find(uiObjId);
            if(obj != NULL) {
                return obj;
            }
        }
    }

    return NULL;
}

bool
FluidOctree3d::getChildBounds(int iQuadName, Box &bx) {
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

void
FluidOctree3d::updateEmptiness() {
    m_bEmpty = m_mObjs.size() == 0;
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_aChildren[q] != NULL) {
            //Only empty if we have nothing and children are empty too
            m_bEmpty = m_bEmpty && m_aChildren[q]->empty();
        }
    }
}
