#include "WorldOctree.h"
#include "tpe/tpe.h"
#include "d3re/d3re.h"
#include "pwe/PartitionedWorldEngine.h"
#include <iostream>
#include "mge/ConfigManager.h"
using namespace std;
static SDL_mutex *s_mxRenderEngine = SDL_CreateMutex();
static bool s_bDisplayBounds;

WorldOctreeNode::WorldOctreeNode(uint uiNodeId, uint uiLevel, uint uiAreaId, const Box &bxBounds, float fMinResolution)
    :   Octree3dNode<GameObject>(uiNodeId, uiLevel, bxBounds, fMinResolution),
        m_uiAreaId(uiAreaId)
{
    printf(__FILE__" %d\n",__LINE__);
    s_bDisplayBounds = ConfigManager::get()->get("pwe.drawBounds", false);
}

WorldOctreeNode::~WorldOctreeNode()
{
    m_lsDynamicObjs.clear();
    m_lsStaticObjs.clear();
}


void
WorldOctreeNode::write(boost::property_tree::ptree &pt, const std::string &keyBase, bool bIsSaveFile) {
    //Write contents
    for(objmap_iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
        GameObject *obj = it->second;

        //Two different kinds of passes: Write-to-world file (editor only) and write-to-save-file
        bool bIsSaveFileObj = obj->getFlag(PWE_SAVE_FILE_OBJ);
        bool bCanWrite = (bIsSaveFile && bIsSaveFileObj) ||
                        (!bIsSaveFile && !bIsSaveFileObj);
        if(bCanWrite) {
            obj->write(pt, keyBase + "." + obj->getClass() + "." + obj->getName());
        }
    }

    //Write children
    for(int q = QUAD_FIRST; q < QUAD_NUM_QUADS; ++q) {
        if(m_apChildren[q] != NULL && !m_apChildren[q]->empty()) {
            WorldOctreeNode *child = (WorldOctreeNode*)m_apChildren[q];
            child->write(pt, keyBase, bIsSaveFile);
        }
    }
}

void
WorldOctreeNode::updateContents(float fTime) {
    TimePhysicsEngine *pe = TimePhysicsEngine::get();
    if(s_bDisplayBounds) {
        D3RE::get()->drawBox(m_bxBounds, Color(m_uiNodeId, m_uiLevel * 85, m_uiAreaId * 85));
    }

    //printf(__FILE__" %d: Octree node %5x updated at time %5d by thread 0x%8x\n", __LINE__, m_uiNodeId, Clock::get()->getTime(), SDL_ThreadID());

    for(objmap_iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
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

        for(objmap_iter_t it = m_mContents.begin(); it != m_mContents.end(); ++it) {
            if(it->second->getId() == (*mv)->getId()) {
                break;
            }
            pe->applyPhysics(it->second, *mv);
        }
    }

    //this->print(cout, __LINE__);
}

void
WorldOctreeNode::handleChildUpdateResults(Octree3dNode<GameObject> *node, int q) {
    WorldOctreeNode *child = (WorldOctreeNode*)(node);
    TimePhysicsEngine *pe = TimePhysicsEngine::get();
    //Perform collision checks on relevant lists
    //Dynamic objects from child must be checked against my static objects
    pe->applyCollisionPhysics(child->m_lsDynamicObjs, m_lsStaticObjs);

    //Static objects from child must be checked against my dynamic objects
    pe->applyCollisionPhysics(child->m_lsStaticObjs, m_lsDynamicObjs);

    //Dynamic objects from child must be checked against my dynamic objects
    pe->applyCollisionPhysics(child->m_lsDynamicObjs, m_lsDynamicObjs);

    //Objects that left the child's quadrant need to be checked against
    // the other children
    for(int q2 = q + 1; q2 < QUAD_NUM_QUADS; ++q2) {
        if(m_apChildren[q2] != NULL) {
            pe->applyCollisionPhysics(
                child->m_lsObjsLeftQuadrant,
                child->m_lsDynamicObjs
            );
            pe->applyCollisionPhysics(
                child->m_lsObjsLeftQuadrant,
                child->m_lsStaticObjs
            );
        }
    }

    //Copy child lists into my lists
    m_lsDynamicObjs.insert(
        m_lsDynamicObjs.end(),
        child->m_lsDynamicObjs.begin(),
        child->m_lsDynamicObjs.end()
    );

    m_lsStaticObjs.insert(
        m_lsStaticObjs.end(),
        child->m_lsStaticObjs.begin(),
        child->m_lsStaticObjs.end()
    );
}

void
WorldOctreeNode::onAdd(GameObject *obj) {
    if(obj->getFlag(PWE_INFORM_OBJ_ADD)) {
        obj->setFlag(PWE_INFORM_OBJ_ADD, false);
        obj->callBack(m_uiNodeId, &m_uiAreaId, PWE_ON_ADDED_TO_AREA);
    }
    SDL_LockMutex(s_mxRenderEngine);
    D3RE::get()->manageObjOnScreen(obj);
    SDL_UnlockMutex(s_mxRenderEngine);
}

void
WorldOctreeNode::onRemove(GameObject *obj) {
    if(obj->getFlag(PWE_INFORM_OBJ_REMOVE)) {
        obj->setFlag(PWE_INFORM_OBJ_REMOVE, false);
        //obj->callBack(m_uiNodeId, &m_uiAreaId, PWE_ON_REMOVED_FROM_AREA);
        SDL_LockMutex(s_mxRenderEngine);
        D3RE::get()->remove(obj);
        SDL_UnlockMutex(s_mxRenderEngine);
    }
}

void
WorldOctreeNode::onErase(GameObject *obj) {
    obj->callBack(m_uiNodeId, &m_uiAreaId, PWE_ON_ERASED_FROM_AREA);
}

Octree3dNode<GameObject> *
WorldOctreeNode::createChild(uint childId, const Box &bxBounds) const {
    uint uiNewLevel = m_uiLevel + 1;
    uint uiNewNodeId = (childId << (uiNewLevel * 4)) | m_uiNodeId;
    WorldOctreeNode *node = new WorldOctreeNode(
        uiNewNodeId,
        uiNewLevel,
        m_uiAreaId,
        bxBounds,
        m_fMinResolution
    );
    return node;
}

/*
 * WorldOctree
 */

WorldOctree::WorldOctree(uint uiNodeId, uint uiAreaId, const Box &bxBounds, float fMinResolution)
    :   WorldOctreeNode(uiNodeId, 0, uiAreaId, bxBounds, fMinResolution)
{
}

WorldOctree::~WorldOctree() {
}

/*
void
WorldOctree::update(float fTime) {
int preSemNum = SDL_SemValue(m_sem);
printf(__FILE__" %d: %d/?\n",__LINE__, preSemNum);
    Octree3dNode<GameObject>::update(fTime);
int postSemNum = SDL_SemValue(m_sem);
printf(__FILE__" %d: %d/%d\n", __LINE__, preSemNum, postSemNum);
}
*/

void
WorldOctree::postUpdate(float fTime) {
    //Post-processing after a normal update
    //Make sure processing is complete
//int preSemNum = SDL_SemValue(m_sem);
//printf(__FILE__" %d: %d/?\n",__LINE__, preSemNum);
    SDL_SemWait(m_sem);
//int postSemNum = SDL_SemValue(m_sem);
//printf(__FILE__" %d: %d/%d\n", __LINE__, preSemNum, postSemNum);
    /*
    SDL_LockMutex(m_mutex);
    while(m_bIsFinished) {
        SDL_CondWait(m_cond, m_mutex);
    }
    */

    //Add objects back to contents
    for(objlist_iter_t it = m_lsObjsLeftQuadrant.begin(); it != m_lsObjsLeftQuadrant.end(); ++it) {
        m_mContents[(*it)->getId()] = (*it);
    }

    SDL_LockMutex(s_mxRenderEngine);

    for(objlist_iter_t it = m_lsDynamicObjs.begin(); it != m_lsDynamicObjs.end(); ++it) {
        D3RE::get()->manageObjOnScreen(*it);
    }

    SDL_UnlockMutex(s_mxRenderEngine);

    cleanResults();
    //SDL_UnlockMutex(m_mutex);
}
