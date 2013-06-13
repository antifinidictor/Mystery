/*
 * Text Bubble
 * Renders the generic text bubble at the bottom of the screen
 */
#ifndef TEXT_BUBBLE_H
#define TEXT_BUBBLE_H
enum TB_States {
    TB_INVISIBLE,
    TB_NEXT_TEXT,
    TB_PREV_TEXT,
    TB_STATIC_TEXT,
    TB_NUM_STATES
};

class TextBubble : public GameObject, public StateDevice {
public:
    //Constructor(s)/Destructor
    TextBubble(uint id);
    virtual ~Player();
    
    //General
    virtual uint getID()                        { return m_uiID; }
    virtual bool getFlag(uint flag)             { return GET_FLAG(m_uiFlags, flag); }
    virtual void setFlag(uint flag, bool value) { m_uiFlags = SET_FLAG(m_uiFlags, flag, value); }

    virtual bool update(uint time);
    virtual uint getType() { return OBJ_PLAYER; }

    //Models
    virtual RenderModel  *getRenderModel()  { return m_pRenderModel; }
    virtual PhysicsModel *getPhysicsModel() { return m_pPhysicsModel; }
    
    //StateDevice
    virtual int getState() { return m_eState; }
    virtual void setState(int iState, void *data);

private:
    uint m_uiID;
    uint m_uiFlags;
    
    //States
    TB_States m_eState;
    bool m_bDone;
    
    //Text states
    int m_iTextStart, m_iTextEnd;
    int m_iNumDisplayLines;
    
    TimePhysicsModel *m_pPhysicsModel;
    CompositeRenderModel *m_pRenderModel
    StripRenderModel *m_pBubbleRM;
    TextRenderModel  *m_pTextRM;
};



#endif
