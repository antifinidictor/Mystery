/*
 * PageManager
 * Handles special instructions for specific pages.
 */

#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

class PageManager : public GameObject {
public:
    //Constructor(s)/Destructor
    PageManager(uint id);
    virtual ~PageManager(); 
    
    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }
    
    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }

private:
    uint m_uiID;
    uint m_uiFlags;
    BasicPhysicsModel   *m_pPhysicsModel;
    OrderedRenderModel  *m_pRenderModel;

};

#endif
