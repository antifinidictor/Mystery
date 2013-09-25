
#include "game/game.h"

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
#include "editor/editor_defs.h"
#include "editor/EditorObject.h"
#include "editor/EditorManager.h"

using namespace std;

//Engine initialization, cleanup
WorldEngine   *createWorldEngine() {
    PWE::init();
    EditorManager::init();
    PWE::get()->setManager(EditorManager::get());
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
    EditorManager::clean();
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


void initWorld() {
    //Perform last-minute setup of the world engine
    PartitionedWorldEngine *we = PWE::get();
    we->setPhysicsEngine(TimePhysicsEngine::get());
    we->setRenderEngine(D3RE::get());

    ModularEngine *mge = ModularEngine::get();

    //Map inputs
    // Typing
    mge->mapInput(SDLK_PERIOD, ED_IN_PERIOD);
    mge->mapInput(SDLK_MINUS, ED_IN_UNDERSCORE);
    mge->mapInput(SDLK_SPACE, ED_IN_SPACE);
    mge->mapInput(SDLK_BACKSPACE, ED_IN_BACKSPACE);
    mge->mapInput(SDLK_RETURN, ED_IN_ENTER);
    mge->mapInput(SDLK_LSHIFT, IN_SHIFT);
    mge->mapInput(SDLK_LCTRL, IN_CTRL);

    // Navigation
    mge->mapInput(SDLK_UP,    IN_NORTH);
    mge->mapInput(SDLK_RIGHT, IN_EAST);
    mge->mapInput(SDLK_DOWN,  IN_SOUTH);
    mge->mapInput(SDLK_LEFT,  IN_WEST);

    //Load image resources
    D3RE::get()->createImage(IMG_PLAYER,   "res/Magus.png", 8, 4);
    D3RE::get()->createImage(IMG_FONT,     "res/gui/font.png", 26, 3);
    D3RE::get()->createImage(IMG_BLOCK,    "res/world/block.png");
    D3RE::get()->createImage(IMG_WALL_TOP, "res/world/wallTop.png");
    D3RE::get()->createImage(IMG_WALL_BOTTOM, "res/world/wallBottom.png");
    D3RE::get()->createImage(IMG_WALL_SIDE, "res/world/wallSide.png");

    //Load audio resources
    BAE::get()->loadSound(AUD_STEP, "res/audio/step.wav");

    //Other singleton initializations
    TextRenderer::init();

    //Initialize starting area
    we->generateArea(ED_AREA_0);
    we->setCurrentArea(ED_AREA_0);

    D3RE::get()->setBackgroundColor(Color(0x0,0x0,0x0));

    //Create first editor object for initial area
    EditorObject *ed = new EditorObject(we->genID(), ED_AREA_0, Point());
    EditorManager::get()->setEditorObject(ed);
    we->add(ed);
}

