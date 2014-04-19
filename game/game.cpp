
#include "game.h"

//SDL includes
#include <SDL.h>
#include "pgl.h"

//#include <SDL_mixer.h>
//#include <SDL_opengl.h>
//#include "SDL_image.h"
#include <fstream>
#include <stdlib.h>

//Engine includes
#include "mge/ModularEngine.h"
#include "pwe/PartitionedWorldEngine.h"
//#include "ore/OrderedRenderEngine.h"
#include "d3re/d3re.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"

//Game includes
#include "game/game_defs.h"
#include "game/Player.h"
#include "game/world/SimplePhysicsObject.h"
#include "game/world/Wall.h"
#include "game/ObjectFactory.h"
#include "game/GameManager.h"

//Test includes
#include "mge/PixelMap.h"
#include "game/gui/TextDisplay.h"
#include "game/gui/DraggableHud.h"
#include "tpe/fluids/mgeMath.h"
#include "tpe/fluids/BruteForceFluidTest.h"

using namespace std;

void testTextCb(uint id);
void testTextCb2(uint id);

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
    return 0;// Obsolete with SDL 2.0     SDL_OPENGL | SDL_HWSURFACE;// | SDL_NOFRAME | SDL_FULLSCREEN;
}

void initWorld() {
    srand(time(NULL));

    //Perform last-minute setup of the world engine
    PartitionedWorldEngine *we = PWE::get();
    we->setPhysicsEngine(TimePhysicsEngine::get());
    we->setRenderEngine(D3RE::get());
    ObjectFactory::init();

    registerClasses();

    ModularEngine *mge = ModularEngine::get();

    //Map inputs
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

    mge->mapInput(SDLK_SPACE, IN_CAST);
    mge->mapInput(SDLK_LSHIFT, IN_SHIFT);
    mge->mapInput(SDLK_LCTRL, IN_CTRL);
    mge->mapInput(SDL_BUTTON_LEFT, IN_SELECT);
    mge->mapInput(SDL_BUTTON_RIGHT, IN_RCLICK);
    mge->mapInput(SDLK_h,     IN_BREAK);
    mge->mapInput(SDLK_F1, IN_TOGGLE_DEBUG_MODE);

    //Initialize the text renderer
    TextRenderer::init();

    //Initialize the game manager
    GameManager::init();
    we->setManager(GameManager::get());

    //Read in the world from the file
    ObjectFactory::get()->read("res/game.info");

    PWE::get()->setCurrentArea(0);

    //Load audio resources
    BAE::get()->loadSound(AUD_STEP, "res/audio/step.wav");
    BAE::get()->loadSound(AUD_PICKUP, "res/audio/pickup.wav");
    BAE::get()->loadSound(AUD_LIFT, "res/audio/liftup.wav");
    BAE::get()->loadSound(AUD_DRAG, "res/audio/drag.wav");
    BAE::get()->loadSound(AUD_POPUP, "res/audio/popup.wav");
    BAE::get()->loadSound(AUD_POPDOWN, "res/audio/popdown.wav");
    BAE::get()->loadSound(AUD_CASTING, "res/audio/spellCastMode.wav");
    BAE::get()->loadSound(AUD_SPELL_POINT, "res/audio/spellPoint.wav");

    BAE::get()->loadMusic(AUD_UNDERGROUND_MUSIC, "res/audio/OfBlackAndWhite.wav");

    //BAE::get()->playMusic(AUD_UNDERGROUND_MUSIC);

    D3RE::get()->setBackgroundColor(Color(0x9a,0xd7,0xfb));
    //D3RE::get()->hideRealMouse();

    //Test text
    //testTextCb(0);
    //testTextCb2(0);
#if 1
    BruteForceFluidTest *bfft = new BruteForceFluidTest(
        NULL,                               //Pixel map
        Box(1.f, 1.f, 5.f, 5.f, 1.f, 5.f),  //Bounds
        5,                                 //Num vortons
        0.1f,                               //Cell size
        0.01f                                 //Viscocity
    );
    PWE::get()->add(bfft);
#endif
}


void cleanWorld() {
    ObjectFactory::clean();
    GameManager::clean();
    TextRenderer::clean();
}


void testTextCb(uint id) {
    static int curId = 0;
    std::string s = "?";
    switch(curId) {
    case 0:
        s = "Hello there!";
        break;
    case 1:
        s = "My name is bob!";
        break;
    case 2:
        s = "So is mine!";
        break;
    case 3:
        s = "This message brought to you by Alchemy Industries.  Bringing you the science of the future!";
        break;
    default:
        s = "This is dumb...";
    }
    TextDisplay::get()->registerText(s, &testTextCb);
    curId = (curId + 1) % 5;
}

void testTextCb2(uint id) {
    static int curId = 0;
    std::string s = "?";
    switch(curId) {
    case 0:
        s = "How many conversations does it take to screw in a light bulb?";
        break;
    case 1:
        s = "Uh, one?";
        break;
    case 2:
        s = "Nope!  You're an idiot!";
        break;
    case 3:
        s = "That's not very nice...";
        break;
    default:
        s = "Suck it.";
    }
    TextDisplay::get()->registerText(s, &testTextCb2, 0.9f);
    curId = (curId + 1) % 5;
}

#if 0
void buildRoom(Box bxVol) {
    PartitionedWorldEngine *we = PWE::get();

    //"Mins" are inside the wall, "maxes" are outside the wall
    float minEast = bxVol.x,
          maxEast = minEast - WORLD_TILE_SIZE;
    float minWest = bxVol.x + bxVol.w,
          maxWest = minWest + WORLD_TILE_SIZE;
    float minNorth = bxVol.z,
          maxNorth = minNorth - WORLD_TILE_SIZE;
    float minSouth = bxVol.z + bxVol.l,
          maxSouth = minSouth + WORLD_TILE_SIZE;
    float minDown = bxVol.y,
          maxDown = minDown - WORLD_TILE_SIZE;
    float height = bxVol.h,
          width = WORLD_TILE_SIZE;


    Wall *wallNorth = new Wall(we->genId(), IMG_NONE, IMG_NONE, IMG_WALL_SIDE,
                               Box(minEast, minDown, maxNorth, minWest - minEast, height, width), WALL_SOUTH);
    Wall *wallWest  = new Wall(we->genId(), IMG_NONE, IMG_NONE, IMG_WALL_SIDE,
                               Box(maxEast, minDown, minNorth, width, height, minSouth - minNorth), WALL_EAST);
    Wall *wallSouth = new Wall(we->genId(), IMG_NONE, IMG_NONE, IMG_WALL_SIDE,
                               Box(minEast, minDown, minSouth, minWest - minEast, height, width), WALL_NORTH);
    Wall *wallEast  = new Wall(we->genId(), IMG_NONE, IMG_NONE, IMG_WALL_SIDE,
                               Box(minWest, minDown, minNorth, width, height, minSouth - minNorth), WALL_WEST);
    Wall *wallFloor = new Wall(we->genId(), IMG_WALL_TOP, IMG_NONE, IMG_NONE,
                               Box(minEast, maxDown, minNorth, minWest - minEast, minDown - maxDown, minSouth - minNorth), WALL_UP);
    we->add(wallNorth);
    we->add(wallSouth);
    we->add(wallEast);
    we->add(wallWest);
    we->add(wallFloor);
}

void buildWorld() {
    PartitionedWorldEngine *we = PWE::get();

    //Initialize the main game area
    we->setCurrentArea(GM_MAIN_GAME);
    we->setEffectiveArea(GM_MAIN_GAME); //Make sure objects actually get added here

    buildRoom(Box(-128, 0, -256, 256, WORLD_TILE_SIZE * 2, 512));

    SimplePhysicsObject *block = new SimplePhysicsObject(we->genId(), IMG_BLOCK, Box(-32,0,0,32,32,32));
    we->add(block);

/*
    Wall *wall0 = new Wall(we->genId(), IMG_WALL_TOP, IMG_WALL_BOTTOM, IMG_WALL_SIDE,
                          Box(50, -32, 50, 128, 32, 128), WALL_SOUTH | WALL_NORTH | WALL_WEST | WALL_EAST | WALL_DOWN);
    wall0->setColor(Color(0, 0, 0xFF));
    we->add(wall0);
*/

    Player *player = new Player(we->genId(), Point());
    we->add(player);
}
#endif
