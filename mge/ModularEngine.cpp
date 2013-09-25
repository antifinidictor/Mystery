#include "ModularEngine.h"
#include "defs.h"

using namespace std;

ModularEngine *ModularEngine::mge;

//Static methods
void ModularEngine::init(int iSDLVideoFlags) {
    mge = new ModularEngine(iSDLVideoFlags);
}

void ModularEngine::clean() {
    delete mge;
    mge = NULL;
}

//Constructor/Destructor
ModularEngine::ModularEngine(int iSDLVideoFlags) {
	m_sInputData.clear();

	//Initialize SDL
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		exit(-1);
	atexit(SDL_Quit);

	//If something goes wrong on another computer or results are inconsistent,
	// swap out SDL_HWSURFACE with SDL_SWSURFACE.
	SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, iSDLVideoFlags);


    //Set window title and icon
    SDL_WM_SetCaption("Modular Game Engine", "MGE");
    SDL_WM_SetIcon(SDL_LoadBMP("res/icon.bmp"), NULL);

	/* Initialize image loading */
	if(IMG_Init(IMG_INIT_PNG) == 0) {
		printf("Failed to initialize image loader.  Error: %s\n", IMG_GetError());
		exit(-1);
	}

    m_bIsRunning = true;
    ck = new Clock();
}

ModularEngine::~ModularEngine() {
    delete ck;
    IMG_Quit();
	m_mInputMap.clear();
	SDL_Quit();
}

//General methods
void ModularEngine::run() {
    SDL_Event event;

    m_uiLastTime = 0;

	/* Main game loop */
	while(m_bIsRunning) {
	    ck->update();

		m_sInputData.clearChanged();
		while(SDL_PollEvent(&event)) {
			//Manage each event individually
			handleInput(&event);
		}

		//Handle input listeners
		if( (m_sInputData.mouseHasMoved()) ) {
			informListeners(ON_MOUSE_MOVE);
		}
		if( (m_sInputData.inputHasChanged()) ){
			informListeners(ON_BUTTON_INPUT);
		}

        //Update the world.
        we->update(ck->getTime());

        //Render the scene.
        re->render();

        //Blit the buffer onto the screen
//        SDL_GL_SwapBuffers();   //Should probably be done by the render engine
		SDL_Delay(5);
	}
}



void ModularEngine::handleInput(SDL_Event *pEvent) {
	switch( pEvent->type )
	{
	case SDL_KEYDOWN:			//Key pressed
		handleKey(pEvent, true);
		break;
	case SDL_KEYUP:
		handleKey(pEvent, false);
		break;
	case SDL_MOUSEBUTTONDOWN:
		handleButton(pEvent, true);
		break;
	case SDL_MOUSEBUTTONUP:
		handleButton(pEvent, false);
		break;
	case SDL_MOUSEMOTION:
		m_sInputData.setInputState(MIN_MOUSE_X, pEvent->motion.x);
		m_sInputData.setInputState(MIN_MOUSE_Y, pEvent->motion.y);
		m_sInputData.setInputState(MIN_MOUSE_REL_X, pEvent->motion.xrel);
		m_sInputData.setInputState(MIN_MOUSE_REL_Y, pEvent->motion.yrel);
		break;
	case SDL_QUIT:
		m_bIsRunning = false;
		break;
	default:
		break;
	}
}

/*
 * handleKey()
 * Handles key press
 */
void ModularEngine::handleKey( SDL_Event *pEvent, bool bDown) {
	//Full list of keys under SDL_keysym.h
	static map<int, int>::iterator key;
	switch( pEvent->key.keysym.sym )
	{
	case SDLK_ESCAPE:
		//Stop the game.
		m_bIsRunning = false;
		break;
	default:
        uint sdlKeyId = pEvent->key.keysym.sym;
		key = m_mInputMap.find(sdlKeyId);
		//If the key mapping exists
		if( key != m_mInputMap.end() ) {
			m_sInputData.setInputState((*key).second, (int)bDown);
		}

        //If the key is a letter (may or may not already be mapped)
        if((sdlKeyId >= SDLK_a) && (sdlKeyId <= SDLK_z)) {
            m_sInputData.setLetter((sdlKeyId - SDLK_a), bDown);
            m_sInputData.setInputState(LKIN_KEY_PRESSED, (int)bDown);
        }
	}
}

/*
 * handleButton()
 * Handles mouse button presses
 */
void ModularEngine::handleButton(SDL_Event *pEvent, bool bDown) {
	static map<int, int>::iterator itrButton;
    itrButton = m_mInputMap.find(pEvent->button.button);
    if( itrButton != m_mInputMap.end() ) {
        m_sInputData.setInputState((*itrButton).second, bDown);
    }
}

void ModularEngine::informListeners(uint id) {
	list<Listener*>::iterator iter;
	switch(id) {
	case ON_MOUSE_MOVE:
		for( iter = m_lsMouseMoveList.begin();
			iter != m_lsMouseMoveList.end();
			++iter ) {
			(*iter)->callBack(ID_MODULAR_ENGINE, &m_sInputData, id);
		}
		break;
	case ON_BUTTON_INPUT:
		for( iter = m_lsButtonInputList.begin();
			iter != m_lsButtonInputList.end();
			++iter ) {
			(*iter)->callBack(ID_MODULAR_ENGINE, &m_sInputData, id);
		}
		break;
	default:
		cout << "Unsupported event handle " << id << ".\n";
	}
}

void ModularEngine::addListener(Listener *pListener, uint id, char* triggerData) {
	switch(id) {
	case ON_MOUSE_MOVE:
		m_lsMouseMoveList.push_back(pListener);
		break;
	case ON_BUTTON_INPUT:
		m_lsButtonInputList.push_back(pListener);
		break;
	default:
		cout << "Unsupported event handle " << id << ".\n";
	}
}

bool ModularEngine::removeListener(uint uiListenerID, uint id) {
	list<Listener*>::iterator iter;
	switch(id) {
	case ON_MOUSE_MOVE:
		for( iter = m_lsMouseMoveList.begin();
			iter != m_lsMouseMoveList.end();
			++iter ) {
			if( (*iter)->getID() == uiListenerID ) {
				m_lsMouseMoveList.erase(iter);
				return true;
			}
		}
		break;
	case ON_BUTTON_INPUT:
		for( iter = m_lsButtonInputList.begin();
			iter != m_lsButtonInputList.end();
			++iter ) {
			if( (*iter)->getID() == uiListenerID ) {
				m_lsButtonInputList.erase(iter);
				return true;
			}
		}
		break;
	default:
		cout << "Unsupported event handle " << id << ".\n";
		return false;
	}
	return false;
}

void ModularEngine::mapInput(int iSdlInputName, int iGameInputName) {
    m_mInputMap.insert(pair<int,int>(iSdlInputName, iGameInputName));
}
