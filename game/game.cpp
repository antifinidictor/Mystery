
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
//#include "ore/OrderedRenderEngine.h"
#include "d3re/d3re.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"

//Game includes
#include "game/game_defs.h"
#include "game/gui/TextRenderer.h"
#include "game/Player.h"
#include "game/SimplePhysicsObject.h"

//Function prototypes
void buildWorld();

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
    D3RE::init();
    return D3RE::get();
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
    D3RE::clean();
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
        buildWorld();
        we->setCurrentArea(GM_MAIN_GAME);
    }
}


void initWorld() {
    //Perform last-minute setup of the world engine
    PartitionedWorldEngine *we = PWE::get();
    we->setPhysicsEngine(TimePhysicsEngine::get());
    we->setRenderEngine(D3RE::get());

    ModularEngine *mge = ModularEngine::get();

    //Map inputs
    mge->mapInput(SDLK_w,     IN_NORTH);
    mge->mapInput(SDLK_d,     IN_EAST);
    mge->mapInput(SDLK_s,     IN_SOUTH);
    mge->mapInput(SDLK_a,     IN_WEST);
    mge->mapInput(SDLK_SPACE, IN_CAST);
    mge->mapInput(SDL_BUTTON_LEFT, IN_SELECT);
    mge->mapInput(SDLK_h,     IN_BREAK);

    D3RE::get()->createImage(IMG_PLAYER, "res/Magus.png", 8, 4);
    D3RE::get()->createImage(IMG_FONT,   "res/gui/font.png", 26, 3);
    D3RE::get()->createImage(IMG_BLOCK,  "res/BlockTexture.png");

    //Other singleton initializations
    TextRenderer::init();

    //Initialize world areas
    for(uint ui = 0; ui < GM_NUM_AREAS; ++ui) {
        we->generateArea(ui);
    }

    buildWorld();
}

void buildWorld() {
    PartitionedWorldEngine *we = PWE::get();

    //Initialize the main game area
    we->setCurrentArea(GM_MAIN_GAME);
    we->setEffectiveArea(GM_MAIN_GAME); //Make sure objects actually get added here

    Player *player = new Player(we->genID(), Point());
    we->add(player);

    SimplePhysicsObject *block = new SimplePhysicsObject(we->genID(), D3RE::get()->getImage(IMG_BLOCK), Box(-32,0,0,32,32,32));
    we->add(block);
}
