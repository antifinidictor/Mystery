#ifndef OCTREE3D_H
#define OCTREE3D_H

/*
 * Class T must have the following properties:
 *      getId()     Returns a unique identifier for the object
 *      getBounds() Returns an AABB indicating the position of the object
 *
 * Note that octrees dynamically allocate more space, but they do not
 * deallocate space until the entire tree is deleted
 */
#include "defs.h"
#include <map>

template<class T>
class Octree3d {
public:

    Octree3d(const Box &bxBounds, float fMinResolution = 1.f) {
        m_bEmpty = true;
        m_bxBounds = bxBounds;
        m_fMinResolution = fMinResolution;

        //Children are allocated as needed when objects are added
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            m_aChildren[q] = NULL;
        }
    }

    virtual ~Octree3d() {
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
    bool add(T *obj, bool bForce = true) {
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
                        m_aChildren[q] = new Octree3d(bxChildBounds, m_fMinResolution);
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
    bool remove(uint uiObjId) {
        std::map<uint,T>::iterator obj = m_mObjs.find(uiObjId);
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
    bool erase(uint uiObjId) {
        std::map<uint,T>::iterator obj = m_mObjs.find(uiObjId);
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
    T *find(uint uiObjId) {
        std::map<uint,T>::iterator obj = m_mObjs.find(uiObjId);
        if(obj != m_mObjs.end()) {
            return obj->second;
        }

        //Otherwise, search children
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_aChildren[q] != NULL && !m_aChildren[q]->empty()) {
                T *obj = m_aChildren[q]->find(uiObjId);
                if(obj != NULL) {
                    return obj;
                }
            }
        }

        return NULL;
    }

    //The Octree is essentially incomplete, you need to implement your own searching functions & other ops

#if 0
    //Applies a functor to each object.  Functor must take in the object id,
    // the object, and return true if done looping
    template<class Functor>
    void forEach(Functor &f) {
        //TODO: Finish.  Also, can we use this for collision checks?

        std::map<uint,T>::iterator it;
        for(it = m_mObjs.begin(); it != m_mObjs.end(); ++it) {
            bool bFinished = f(it->first, it->second);

            //Make sure this object still fits in this part of the octree

            //Check if this object can be put in a child

            if(bFinished) {
                break;
            }
        }
    }
#endif
protected:
//private:
    enum QuadrantNames {
        QUAD_FIRST = 0,                     //Used for iterating
        QUAD_POSX_POSY_POSZ = QUAD_FIRST,
        QUAD_POSX_POSY_NEGZ,
        QUAD_POSX_NEGY_POSZ,
        QUAD_POSX_NEGY_NEGZ,
        QUAD_NEGX_POSY_POSZ,
        QUAD_NEGX_POSY_NEGZ,
        QUAD_NEGX_NEGY_POSZ,
        QUAD_NEGX_NEGY_NEGZ,
        QUAD_NUM_QUADS      //Used for iterating
    };
    #define QUAD_X_MASK 0x4
    #define QUAD_Y_MASK 0x2
    #define QUAD_Z_MASK 0x1

    bool getChildBounds(int iQuadName, Box &bx) {
        //Half-widths
        float hw = bxBounds.w / 2;
        float hh = bxBounds.h / 2;
        float hl = bxBounds.l / 2;

        if(hw < m_fMinResolution && hh < m_fMinResolution && hl < m_fMinResolution) {
            //The parent of this 'child' is really a leaf node
            return false;
        }

        if(iQuadName & QUAD_X_MASK != 0) {  //negative
            bx.x = bxBounds.x;
            bx.w = (hw < m_fMinResolution) ? bxBounds.w : hw;
        } else {                            //positive
            if(hw < m_fMinResolution) {
                //Invalid child: Positive
                return false;
            } else {
                bx.x = bxBounds.x + hw;
                bx.w = hw;
            }
        }

        if(iQuadName & QUAD_Y_MASK != 0) {  //negative
            bx.y = bxBounds.y;
            bx.h = (hh < m_fMinResolution) ? bxBounds.h : hh;
        } else {                            //positive
            if(hh < m_fMinResolution) {
                //Invalid child: Positive
                return false;
            } else {
                bx.y = bxBounds.y + hh;
                bx.h = hh;
            }
        }

        if(iQuadName & QUAD_Z_MASK != 0) {  //negative
            bx.z = bxBounds.z;
            bx.l = (hl < m_fMinResolution) ? bxBounds.l : hl;
        } else {                            //positive
            if(hl < m_fMinResolution) {
                //Invalid child: Positive
                return false;
            } else {
                bx.z = bxBounds.z + hl;
                bx.l = hl;
            }
        }

        //If we make it here, the child is valid
        return true;
    }

    void updateEmptiness() {
        m_bEmpty = m_mObjs.size() == 0;
        for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
            if(m_aChildren[q] != NULL) {
                //Only empty if we have nothing and children are empty too
                m_bEmpty = m_bEmpty && m_aChildren[q]->empty();
            }
        }
    }

    bool empty() {
        return m_bEmpty;
    }

    //Fellow octree information
    //Octree3d *m_pParent;
    Octree3d<T> *m_aChildren[QUAD_NUM_QUADS];

    //Object information
    std::map<uint, T*> m_mObjs;
    bool m_bEmpty;

    //Boundary information
    Box m_bxBounds;
    float m_fMinResolution;
};

#endif // OCTREE3D_H
