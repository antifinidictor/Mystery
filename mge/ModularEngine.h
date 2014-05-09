#ifndef MODULARENGINE_H
#define MODULARENGINE_H

#include <stdio.h>
#include <iostream>

#include "SDL.h"
#include "pgl.h"

//#include <gl/gl.h>
//#include <gl/glu.h>
//#include <SDL_mixer.h>
//#include <SDL_opengl.h>
#include "SDL_image.h"

#include <list>
#include <map>

#include "mge/RenderEngine.h"
#include "mge/WorldEngine.h"
#include "mge/PhysicsEngine.h"
#include "mge/AudioEngine.h"
#include "mge/Clock.h"

#include "mge/Event.h"

class ModularEngine : public EventHandler {
public:

    //Static methods
    static void init(int iSDLVideoFlags);
    static void clean();
    static ModularEngine *get() { return mge; }

    //Less static methods
    void run();
    void stop() { m_bIsRunning = false; }
    bool isRunning() { return m_bIsRunning; }

    //From EventHandler
	virtual void addListener(Listener *pListener, uint id, char* triggerData = NULL);
	virtual bool removeListener(uint uiListenerID, uint eventID);	//Returns true if object found
	virtual uint getId() { return 0; }
	virtual void informListeners(uint id);

	//Getter/setter methods
	void setWorldEngine(WorldEngine *we)        { this->we = we; }
	void setPhysicsEngine(PhysicsEngine *pe)    { this->pe = pe; }
	void setRenderEngine(RenderEngine *re)      { this->re = re; }
	void setAudioEngine(AudioEngine *ae)        { this->ae = ae; }
	WorldEngine   *getWorldEngine()  { return we; }
	PhysicsEngine *getPhysicsEngine() { return pe; }
	RenderEngine  *getRenderEngine() { return re; }
	AudioEngine   *getAudioEngine()  { return ae; }

    //Input adjustment
    void mapInput(int iSdlInputName, int iGameInputName);

    //Polling input
    InputData *getInputState() { return &m_sInputData; }

protected:
private:
    ModularEngine(int iSDLVideoFlags);
    virtual ~ModularEngine();

    //Static members
    static ModularEngine *mge;

    //Manager objects
    Clock         *ck;
    RenderEngine  *re;
    WorldEngine   *we;
    PhysicsEngine *pe;
    AudioEngine   *ae;

    //Input handling
    InputData m_sInputData;
	std::map<int, int> m_mInputMap;	//Maps SDL constants to InputIDs
	//std::map<uint, Listener*> m_mButtonInputListeners;
	//std::map<uint, Listener*> m_mMouseMoveListeners;
	std::list<Listener*>m_lsButtonInputListeners;
	std::list<Listener*>m_lsMouseMoveListeners;

    //General
	bool m_bIsRunning;
    uint m_uiLastTime;

    //Helper methods
    void handleInput(SDL_Event *pEvent);
    void handleKey( SDL_Event *pEvent, bool bDown);
    void handleButton(SDL_Event *pEvent, bool bDown);
};

typedef ModularEngine MGE;

#endif // MODULARENGINE_H
