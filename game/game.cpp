
#include "game.h"

//SDL includes
#include <SDL.h>
#include <gl/gl.h>
#include <gl/glu.h>
//#include <SDL_mixer.h>
//#include <SDL_opengl.h>
//#include "SDL_image.h"
#include <fstream>

//Engine includes
#include "mge/ModularEngine.h"
#include "pwe/PartitionedWorldEngine.h"
#include "ore/OrderedRenderEngine.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"

//Game includes

//Function prototypes

//Engine initialization, cleanup
WorldEngine   *createWorldEngine() {
    PWE::init();
    return PWE::get();
}

PhysicsEngine *createPhysicsEngine() {
    //TimePhysicsEngine::init();
    TimePhysicsEngine::init();
    return TimePhysicsEngine::get();

}

RenderEngine  *createRenderEngine() {
    OrderedRenderEngine::init();
    return OrderedRenderEngine::get();
}

AudioEngine   *createAudioEngine() {
    BasicAudioEngine::init();
    return BasicAudioEngine::get();
}

void cleanWorldEngine() {
    PWE::clean();
}

void cleanPhysicsEngine() {
    TimePhysicsEngine::clean();
}

void cleanRenderEngine() {
    OrderedRenderEngine::clean();
}

void cleanAudioEngine() {
    BasicAudioEngine::clean();
}

//SDL video flags
int getSDLVideoFlags() {
    return SDL_OPENGL | SDL_HWSURFACE;// | SDL_NOFRAME | SDL_FULLSCREEN;
}

void goToGamePage(uint buttonID, uint eventID) {
    if(eventID == ON_ACTIVATE) {
        PWE::get()->setCurrentArea(GM_MAIN_GAME);
    }
}

void goToHomePage(uint buttonID, uint eventID) {
    if(eventID == ON_ACTIVATE) {
        PWE::get()->setCurrentArea(GM_START_PAGE);
    }
}

void resetGamePage(uint buttonID, uint eventID) {
    if(eventID == ON_ACTIVATE) {
        PWE *we = PWE::get();
        we->cleanArea(GM_MAIN_GAME);
        buildMainGameArea();
        we->setCurrentArea(GM_MAIN_GAME);
    }
}

void buildWorld() {

}
