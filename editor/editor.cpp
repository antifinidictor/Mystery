
#include "game/game.h"

//SDL includes
#include <SDL.h>
#include "pgl.h"

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
#include "editor/EditorCursor.h"
#include "editor/EditorManager.h"
#include "game/ObjectFactory.h"
#include "game/GameManager.h"
#include "game/gui/GuiButton.h"

using namespace std;

/*
namespace game {
//TODO: Horrendous hack
#include "game/game.cpp"
}
*/

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
    return 0;//Rendered useless by SDL 2.0 SDL_OPENGL | SDL_HWSURFACE;// | SDL_NOFRAME | SDL_FULLSCREEN;
}

void initWorld() {
    printf("Initializing editor\n");
    //Perform last-minute setup of the world engine
    PartitionedWorldEngine *we = PWE::get();
    we->setPhysicsEngine(TimePhysicsEngine::get());
    we->setRenderEngine(D3RE::get());

    ObjectFactory::init();
    EditorManager::init();
    GameManager::init();
    we->setManager(EditorManager::get());

    registerClasses();

    ModularEngine *mge = ModularEngine::get();

    //Map inputs
    // Typing
    mge->mapInput(SDLK_PERIOD, ED_IN_PERIOD);
    mge->mapInput(SDLK_MINUS, ED_IN_UNDERSCORE);
    mge->mapInput(SDLK_SPACE, ED_IN_SPACE);
    mge->mapInput(SDLK_BACKSPACE, ED_IN_BACKSPACE);
    mge->mapInput(SDLK_RETURN, ED_IN_ENTER);
    mge->mapInput(SDLK_SLASH, ED_IN_SLASH);
    mge->mapInput(SDLK_COLON, ED_IN_COLON);
    mge->mapInput(SDLK_LSHIFT, IN_SHIFT);
    mge->mapInput(SDLK_LCTRL, IN_CTRL);

    // Selection
    mge->mapInput(SDL_BUTTON_LEFT, IN_SELECT);
    mge->mapInput(SDLK_F1, IN_TOGGLE_DEBUG_MODE);

    // Navigation
    mge->mapInput(SDLK_w,     IN_NORTH);
    mge->mapInput(SDLK_d,     IN_EAST);
    mge->mapInput(SDLK_s,     IN_SOUTH);
    mge->mapInput(SDLK_a,     IN_WEST);
    mge->mapInput(SDLK_q,     IN_ROTATE_LEFT);
    mge->mapInput(SDLK_e,     IN_ROTATE_RIGHT);

    mge->mapInput(SDLK_UP,    IN_NORTH);
    mge->mapInput(SDLK_RIGHT, IN_EAST);
    mge->mapInput(SDLK_DOWN,  IN_SOUTH);
    mge->mapInput(SDLK_LEFT,  IN_WEST);
    mge->mapInput(SDLK_RSHIFT,IN_ROTATE_LEFT);
    mge->mapInput(SDLK_END,   IN_ROTATE_RIGHT);
/*
    mge->mapInput(SDLK_SPACE, IN_CAST);
    mge->mapInput(SDLK_LSHIFT, IN_SHIFT);
    mge->mapInput(SDLK_LCTRL, IN_CTRL);
    mge->mapInput(SDL_BUTTON_LEFT, IN_SELECT);
    mge->mapInput(SDL_BUTTON_RIGHT, IN_RCLICK);
    mge->mapInput(SDLK_h,     IN_BREAK);
    mge->mapInput(SDLK_F1, IN_TOGGLE_DEBUG_MODE);
*/

    //Load image resources (required by editor)
    D3RE::get()->createImage(IMG_NONE,     "res/gui/noImage.png");
    D3RE::get()->createImage(IMG_FONT,     "res/gui/font.png", 26, 3, true);
    D3RE::get()->createImage(IMG_BUTTON,   "res/gui/button.png", 3, 1);

    //These could be loaded from a file
    /*
    D3RE::get()->createImage(IMG_PLAYER,   "res/Magus.png", 8, 4);
    D3RE::get()->createImage(IMG_BLOCK,    "res/world/block.png");
    D3RE::get()->createImage(IMG_WALL_TOP, "res/world/wallTop.png");
    D3RE::get()->createImage(IMG_WALL_BOTTOM, "res/world/wallBottom.png");
    D3RE::get()->createImage(IMG_WALL_SIDE, "res/world/wallSide.png");
    */

    //Load audio resources
    //BAE::get()->loadSound(AUD_STEP, "res/audio/step.wav");

    //Other singleton initializations
    TextRenderer::init();

    //Initialize starting area
    we->generateArea(ED_AREA_0);
    we->setCurrentArea(ED_AREA_0);

    D3RE::get()->setBackgroundColor(Color(0x0,0x0,0x0));

    //Create first editor object for initial area
    EditorCursor *ed = new EditorCursor(we->genId(), ED_AREA_0, Point());
    EditorManager::get()->setEditorCursor(ed);
    we->add(ed);
}

void cleanWorld() {
    ObjectFactory::clean();
    EditorManager::clean();
}
