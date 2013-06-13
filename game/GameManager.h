/*
 * GameManager.h
 * Defines the game manager class
 */

#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <map>
#include "mge/defs.h"
#include "mge/Event.h"

enum GMPageName {
    GM_START_PAGE,
    GM_PAUSE_SCREEN,
    GM_MAIN_GAME,
    GM_NUM_SCREENS
};

class GameManager : public Listener {
public:
    static void init()  { gm = new GameManager(); }
    static void clean() { delete gm; }
    static GameManager *get() { return gm; }

	virtual void callBack(uint cID, void *data, EventID id);
	virtual uint getID() { return m_uiID; }
    
    void setScreen(GMPageName pn);

private:
    GameManager();
    virtual ~GameManager();
    static GameManager *gm;

    uint m_uiID;
    std::map<int, uint> m_mPageMap;
    Point m_ptLastScreenPos;
    GMPageName m_eCurScreen;
    
    void handleButton(InputData* data);
};

#endif
