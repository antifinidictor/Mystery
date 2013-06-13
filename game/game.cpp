
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
//#include "tpe/TimePhysicsEngine.h"
#include "ore/OrderedRenderEngine.h"
#include "bae/BasicAudioEngine.h"
#include "tpe/TimePhysicsEngine.h"

//Game includes
#include "game/Player.h"
#include "game/PhysicsSurface.h"
#include "game/Wall.h"
#include "game/MazeGenerator.h"
#include "game/SimplePhysicsObject.h"
#include "game/FXSprite.h"
#include "game/spells/Spells.h"
#include "game/GameDefs.h"
#include "game/TextRenderer.h"
#include "game/Button.h"
#include "game/GameManager.h"
#include "game/Decorative.h"
#include "game/Npc.h"
#include "game/TextBubble.h"
#include "game/Elevator.h"

//Test includes
#include "ore/EdgeRenderModel.h"
#include "game/StateButton.h"
#include "game/ClickableItem.h"
#include "ore/StripRenderModel.h"

//Function prototypes
void buildMaze(WorldEngine *we, Point pos, int roomsWide, int roomsLong);
void buildRoom(WorldEngine *we, Rect rcArea, uint uiArea);
int getRoom(int iDirection, int iWidth, int iIndex);
Rect getRoomBounds(int iRoomsWide, int iWidth, int iRoom);
void buildMainGameArea();


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
    //Perform last-minute initialization
    PartitionedWorldEngine *we = PWE::get();
    we->setPhysicsEngine(TimePhysicsEngine::get());
    we->setRenderEngine(OrderedRenderEngine::get());

    GameManager::init();


    //Map inputs
    ModularEngine *mge = ModularEngine::get();
    //mge->mapInput(SDLK_UP,    IN_NORTH);
    //mge->mapInput(SDLK_RIGHT, IN_EAST);
    //mge->mapInput(SDLK_DOWN,  IN_SOUTH);
    //mge->mapInput(SDLK_LEFT,  IN_WEST);
    mge->mapInput(SDLK_w,     IN_NORTH);
    mge->mapInput(SDLK_d,     IN_EAST);
    mge->mapInput(SDLK_s,     IN_SOUTH);
    mge->mapInput(SDLK_a,     IN_WEST);
    mge->mapInput(SDLK_SPACE, IN_CAST);
    mge->mapInput(SDL_BUTTON_LEFT, IN_SELECT);
    mge->mapInput(SDLK_h,     IN_BREAK);

    //Build images
    ORE *ore = ORE::get();
    Image *imge = ore->createImage("res/MGE_Large_Dark.png", 1, 1),
          *imgMagus = ore->createImage("res/Magus.png", 8, 4),
          *imgBlock = ore->createImage("res/Block.png"),
          *imgFX = ore->createImage("res/FX.png", 5, 3),
          *imgSpell = ore->createImage("res/Spells.png", 4, 4),
          *imgFont = ore->createImage("res/font.png", 26, 3),
          *imgButton = ore->createImage("res/button.png", 3, 1),
          *imgShadow = ore->createImage("res/shadow.png", 1, 1),
          *imgNPC = ore->createImage("res/npc.png", 5, 4),
          *imgTextBubble = ore->createImage("res/speechBubble.png", 3, 1),
          *imgHorizEdges = ore->createImage("res/hcliffs.png", 5, 1),
          *imgVertEdges = ore->createImage("res/vcliffs.png", 1, 4),
          *imgCorners = ore->createImage("res/corners.png", 6, 2),
          *imgTB = ore->createImage("res/textBubble.png", 5, 1);

    //Spell::setImage(imgSpell);
    TextRenderer::init();
    TextRenderer::get()->setFont(imgFont);

    //Map standard images
    ore->mapStandardImage(IMG_FX, imgFX->m_uiID);
    ore->mapStandardImage(IMG_SPELLS, imgSpell->m_uiID);
    ore->mapStandardImage(IMG_FONT_BASIC, imgFont->m_uiID);
    ore->mapStandardImage(IMG_SHADOW, imgShadow->m_uiID);
    ore->mapStandardImage(IMG_PLAYER, imgMagus->m_uiID);
    ore->mapStandardImage(IMG_NPC, imgNPC->m_uiID);
    ore->mapStandardImage(IMG_TEXT_BUBBLE, imgTextBubble->m_uiID);
    ore->mapStandardImage(IMG_BLOCK, imgBlock->m_uiID);
    ore->mapStandardImage(IMG_BUTTON, imgButton->m_uiID);
    ore->mapStandardImage(IMG_H_EDGES, imgHorizEdges->m_uiID);
    ore->mapStandardImage(IMG_V_EDGES, imgVertEdges->m_uiID);
    ore->mapStandardImage(IMG_CORNERS, imgCorners->m_uiID);
    ore->mapStandardImage(IMG_TB, imgTB->m_uiID);


    //Print all flags for the used engines
    printf("WORLD_FLAGS_BEGIN = %2x\n",WORLD_FLAGS_BEGIN);
    printf("PHYSICS_FLAGS_BEGIN = %2x\n",PHYSICS_FLAGS_BEGIN);
    printf("RENDER_FLAGS_BEGIN = %2x\n",RENDER_FLAGS_BEGIN);
    printf("GAME_FLAGS_BEGIN = %2x\n",GAME_FLAGS_BEGIN);
    printf("\n");

    printf("TPE_STATIC = %2x\n",TPE_STATIC);
    printf("TPE_PASSABLE = %2x\n",TPE_PASSABLE);
    printf("TPE_FLOATING = %2x\n",TPE_FLOATING);
    printf("TPE_FALLING = %2x\n",TPE_FALLING);
    printf("TPE_NUM_FLAGS = %2x\n",TPE_NUM_FLAGS);
    printf("\n");

    printf("ORE_ON_SCREEN = %2x\n",ORE_ON_SCREEN);
    printf("ORE_INVISIBLE = %2x\n",ORE_INVISIBLE);
    printf("ORE_NUM_FLAGS = %2x\n",ORE_NUM_FLAGS);
    printf("\n");

    printf("GAM_SPELLABLE = %2x\n",GAM_SPELLABLE);
    printf("GAM_ACTIVE = %2x\n",GAM_ACTIVE);
    printf("GAM_NUM_FLAGS = %2x\n",GAM_NUM_FLAGS);
    printf("\n");




    //Prepare to generate areas
    we->setCurrentArea(GM_MAIN_GAME);

    //buildMainGameArea();
/*
    Player *plyr = new Player(we->genID(), magus, Point(-100, 64, 32));
    Npc *npc = new Npc(we->genID(), imgNPC, Point(-200, 200, 0));
    TextBubble *testText = new TextBubble(we->genID(), imgTextBubble, "This is an extremely long test message, wouldn't you agree?\nIt may even take two bubbles!", we);
    SimplePhysicsObject *block;
    for(int i = 0; i < 5; ++i) {
        block = new SimplePhysicsObject(we->genID(), imgBlock, Box(-64 - 32 * i, 64, 0, 32, 25, 32));
        block->setFlag(GAM_SPELLABLE, true);
        block->setFlag(TPE_STATIC, true);
        we->add(block);
    }
    buildRoom(we, Rect(-1000, 0, 1000, 1000));
    we->add(plyr);
    we->add(npc);
    we->add(testText);
*/
    //Decorative *shadow = new Decorative(we->genID(), imgShadow, Point(-100, 100, 0.f), ORE_LAYER_LOW_FX);
    //we->add(shadow);

    we->setCurrentArea(GM_START_PAGE);

    //Decorative *decTestWallRender = new Decorative(we->genID(), new EdgeRenderModel(NULL, Rect(0, 0, 32*30, 24), 0, ORE_LAYER_OBJECTS));
    StripRenderModel *mdl = new StripRenderModel(imgTB, Rect(0, 0, 32*10, 32*2), 4+2, ORE_LAYER_OBJECTS);
    //            s  x  y  f  r
    mdl->setStrip(0, 0, 0, 1, 1);
    mdl->setStrip(1, 0, 1, 2, 1);
    mdl->setStrip(2, 9, 0, 3, 1);
    mdl->setStrip(3, 9, 1, 4, 1);
    mdl->setStrip(4, 1, 0, 0, 10-2);
    mdl->setStrip(5, 1, 1, 0, 10-2);
    Decorative *decTestTextBubble = new Decorative(we->genID(), mdl);

    Decorative *dec = new Decorative(we->genID(), imge, Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0.f), ORE_LAYER_OBJECTS);
    Button *btnNew = new Button(we->genID(), imgButton, Point(SCREEN_WIDTH / 2 - imgButton->w / 2, imge->h, 0), "New");
    Button *btnLoad = new Button(we->genID(), imgButton, Point(SCREEN_WIDTH / 2 - imgButton->w / 2, imge->h + imgButton->h / imgButton->m_iNumFramesH, 0), "Load");
    btnNew->addListener(resetGamePage);
    btnLoad->addListener(goToGamePage);
    we->add(btnNew);
    we->add(btnLoad);
    we->add(dec);
    //we->add(decTestWallRender);
    we->add(decTestTextBubble);

#if 0
    //Build maze in area 1
    GravityGameObject *mgeImage = new GravityGameObject(we->genID(), imge, Point(0,-imge->h,0), false);
    Button *btn = new Button(we->genID(), imgButton, Point(-50, -50, 0));
    buildMaze(we, Point(), 15, 15);
    we->add(mgeImage);
    we->add(btn);

    //Build everything else in area 2
    we->setCurrentArea(area2ID);
    Player *plyr = new Player(we->genID(), magus, Point(-100, 48, 0));
    SimplePhysicsObject *block = new SimplePhysicsObject(we->genID(), imgBlock, Box(-64 - 32, 64, 0, 32, 32, 32));
    block->setFlag(GAM_SPELLABLE, true);
    block->setFlag(TPE_STATIC, true);
    printf("Block id: %d\n", block->getID());

    buildRoom(we, Rect(-1000, 0, 1000, 1000));

    we->add(plyr);
    we->add(block);
#endif


    ///////////////////////////////////////////
    //Audio Engine Test
    BasicAudioEngine *ae = BasicAudioEngine::get();
    uint uiMusic = ae->loadMusic("res/Secret_of_Mana_NightTime_Evolution_OC_ReMix.mp3");
    //ae->playMusic(uiMusic);
    uint uiSound = ae->loadSound("res/pop.wav");
    ae->playSound(uiSound);
    ///////////////////////////////////////////

}

void buildRoom(PartitionedWorldEngine *we, Rect rcArea, uint uiArea) {
    Image *imgTile         = ORE::get()->createImage("res/tiles.png", 1, 1),
          *imgWallStraight = ORE::get()->createImage("res/walls.png", 1, 1),
          *imgWallCorner   = ORE::get()->createImage("res/wallcorners.png", 6, 2);
#define BLOCK_SIZE 32
    Box bxFloor = Box(rcArea.x, rcArea.y, -10, rcArea.w, rcArea.l, 10);
    Box bxWallNorth = Box(rcArea.x + BLOCK_SIZE, rcArea.y, 0, rcArea.w - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE),
        bxWallEast  = Box(rcArea.x + rcArea.w - BLOCK_SIZE, rcArea.y + BLOCK_SIZE, 0, BLOCK_SIZE, rcArea.l - BLOCK_SIZE, BLOCK_SIZE),
        bxWallSouth = Box(rcArea.x, rcArea.y + rcArea.l - BLOCK_SIZE, 0, rcArea.w - BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE),
        bxWallWest  = Box(rcArea.x, rcArea.y, 0, BLOCK_SIZE, rcArea.l - BLOCK_SIZE, BLOCK_SIZE);
    PhysicsSurface *floor = new PhysicsSurface(we->genID(), imgTile, bxFloor);
    Wall *northWall = new Wall(we->genID(), imgWallStraight, imgWallCorner, bxWallNorth, WALL_NORTH),
         *eastWall  = new Wall(we->genID(), imgWallStraight, imgWallCorner, bxWallEast,  WALL_EAST),
         *southWall = new Wall(we->genID(), imgWallStraight, imgWallCorner, bxWallSouth, WALL_SOUTH),
         *westWall  = new Wall(we->genID(), imgWallStraight, imgWallCorner, bxWallWest,  WALL_WEST);
    northWall->setNextWall(eastWall);
    eastWall->setNextWall(southWall);
    southWall->setNextWall(westWall);
    westWall->setNextWall(northWall);
    we->addTo(floor, uiArea);
    we->addTo(northWall, uiArea);
    we->addTo(eastWall, uiArea);
    we->addTo(southWall, uiArea);
    we->addTo(westWall, uiArea);
}

void buildMainGameArea() {
    PartitionedWorldEngine *we = PWE::get();
    we->setEffectiveArea(GM_MAIN_GAME);

    Image *imgMagus      = ORE::get()->getMappedImage(IMG_PLAYER),
          *imgNPC        = ORE::get()->getMappedImage(IMG_NPC),
          *imgTextBubble = ORE::get()->getMappedImage(IMG_TEXT_BUBBLE),
          *imgBlock      = ORE::get()->getMappedImage(IMG_BLOCK),
          *imgButton     = ORE::get()->getMappedImage(IMG_BUTTON),
          *imgTB = ORE::get()->getMappedImage(IMG_TB);


    Player *plyr = new Player(we->genID(), imgMagus, Point(-100, 64, 32));
    Npc *npc = new Npc(we->genID(), imgNPC, Point(-200, 200, 0));
    TextBubble *testText = new TextBubble(we->genID(), imgTextBubble, "This is an extremely long test message, wouldn't you agree?\nIt may even take two bubbles!  Of course, it didn't.  Not right away, at least.  But with these last few additions, it should take up extra just fine!", we);
    SimplePhysicsObject *block;
    for(int x = 0; x < 5; ++x) {
        for(int y = 0; y < 1; ++y) {
            block = new SimplePhysicsObject(we->genID(), imgBlock, Box(-96 - 32 * x, 64 + 25 * y, 0, 32, 25, 32));
            block->setFlag(GAM_SPELLABLE, true);
            block->setFlag(TPE_STATIC, true);
            we->add(block);
        }
    }
    Elevator *ev = new Elevator(we->genID(), imgBlock, Box(-64, 64, 0, 32, 25, 32));
    we->addTo(ev, GM_MAIN_GAME);
    buildRoom(we, Rect(-1000, 0, 1000, 1000), GM_MAIN_GAME);
    we->addTo(plyr, GM_MAIN_GAME);
    we->addTo(npc, GM_MAIN_GAME);
    we->addTo(testText, GM_MAIN_GAME);
    
    StripRenderModel *mdl = new StripRenderModel(imgTB, Rect(-500, 200, 32*10, 32*2), 4+2, ORE_LAYER_OBJECTS);
    //            s  x  y  r  f
    mdl->setStrip(0, 0, 0, 1, 1);
    mdl->setStrip(1, 0, 1, 1, 2);
    mdl->setStrip(2, 9, 0, 1, 3);
    mdl->setStrip(3, 9, 1, 1, 4);
    mdl->setStrip(4, 1, 0, 10-2, 0);
    mdl->setStrip(5, 1, 1, 10-2, 0);
    Decorative *decTestTextBubble = new Decorative(we->genID(), mdl);
    decTestTextBubble->setFlag(TPE_STATIC, true);
    decTestTextBubble->setFlag(TPE_PASSABLE, true);
    we->add(decTestTextBubble);

    Button *btnBack = new Button(we->genID(), imgButton, Point(-SCREEN_WIDTH / 2 - imgButton->w / 2, imgButton->h / imgButton->m_iNumFramesH, 0), "Back");
    btnBack->addListener(goToHomePage);
    we->add(btnBack);

    we->restoreEffectiveArea();
}


void buildMaze(WorldEngine *we, Point pos, int roomsWide, int roomsLong) {
    //Let's say rooms are 3x3 squares of 32x32 (96x96).
#define BLOCK_W 32
#define BLOCK_L 32
#define ROOM_W BLOCK_W * 3
#define ROOM_L BLOCK_L * 3
#define FLOOR_THICKNESS 1
    int width =  ROOM_W * roomsWide,
        length = ROOM_L * roomsLong;

    //Get the necessary images
    Image *imgTile         = ORE::get()->createImage("res/tiles.png", 1, 1),
          *imgWallStraight = ORE::get()->createImage("res/walls.png", 1, 1),
          *imgWallCorner   = ORE::get()->createImage("res/wallcorners.png", 6, 2);
    //Get the floor
    PhysicsSurface *floor = new PhysicsSurface(we->genID(), imgTile, Box(pos.x,pos.y,pos.z-FLOOR_THICKNESS, width,length,FLOOR_THICKNESS));
    we->add(floor);

    //Get the maze
    //tWall *aWalls = (tWall*)calloc(sizeof(tWall), roomsWide * roomsLong);
    tWall aWalls[roomsWide * roomsLong];
    generateMaze(roomsWide, roomsLong, aWalls);

//#define TILE_SIZE BLOCK_W
#define ROOM_SIZE ROOM_W
#define Z_HEIGHT BLOCK_SIZE
    const int END_ROOM = (roomsLong - 1) * roomsWide,
              END_WALL = WALL_NORTH;
    int iRoom = END_ROOM,
        iState = END_WALL,
        iDirection = END_WALL;
    Wall *pCurWall = NULL,
         *pPrevWall = NULL,
         *pFirstWall = NULL;
    Box bxArea;
    printf("Floor id: %d\n", floor->getID());

    do {
        Rect rcRoom = getRoomBounds(roomsWide, ROOM_SIZE, iRoom);
        rcRoom += pos;

        switch(iState) {
        case WALL_NORTH:
            if(aWalls[iRoom].top) {
                iDirection = WALL_NORTH;
                if(aWalls[iRoom].right) {
                    bxArea = Box(rcRoom.x + TILE_SIZE, rcRoom.y, pos.z,
                                 rcRoom.w - TILE_SIZE, TILE_SIZE, Z_HEIGHT);
                    iState = WALL_EAST;
                } else {
                    bxArea = Box(rcRoom.x + TILE_SIZE, rcRoom.y, pos.z,
                                 rcRoom.w, TILE_SIZE, Z_HEIGHT);
                    iRoom = getRoom(EAST, roomsWide, iRoom);
                }
            } else {    //Move wall outside room
                iDirection = WALL_WEST;
                bxArea = Box(rcRoom.x, rcRoom.y - TILE_SIZE, pos.z,
                             TILE_SIZE, TILE_SIZE, Z_HEIGHT);
                iState = WALL_WEST;
                iRoom = getRoom(NORTH, roomsWide, iRoom);
            }
            break;
        case WALL_EAST:
            if(aWalls[iRoom].right) {
                iDirection = WALL_EAST;
                if(aWalls[iRoom].bottom) {
                    bxArea = Box(rcRoom.x + rcRoom.w - TILE_SIZE, rcRoom.y + TILE_SIZE, pos.z,
                                 TILE_SIZE, rcRoom.l - TILE_SIZE, Z_HEIGHT);
                    iState = WALL_SOUTH;
                } else {
                    bxArea = Box(rcRoom.x + rcRoom.w - TILE_SIZE, rcRoom.y + TILE_SIZE, pos.z,
                                 TILE_SIZE, rcRoom.l, Z_HEIGHT);
                    iRoom = getRoom(SOUTH, roomsWide, iRoom);
                }
            } else {
                iDirection = WALL_NORTH;
                bxArea = Box(rcRoom.x + rcRoom.w, rcRoom.y, pos.z,
                             TILE_SIZE, TILE_SIZE, Z_HEIGHT);
                iState = WALL_NORTH;
                iRoom = getRoom(EAST, roomsWide, iRoom);
            }
            break;
        case WALL_SOUTH:
            if(aWalls[iRoom].bottom) {
                iDirection = WALL_SOUTH;
                if(aWalls[iRoom].left) {
                    bxArea = Box(rcRoom.x, rcRoom.y + rcRoom.l - TILE_SIZE, pos.z,
                                 rcRoom.w - TILE_SIZE, TILE_SIZE, Z_HEIGHT);
                    iState = WALL_WEST;
                } else {
                    bxArea = Box(rcRoom.x - TILE_SIZE, rcRoom.y + rcRoom.l - TILE_SIZE, pos.z,
                                 rcRoom.w, TILE_SIZE, Z_HEIGHT);
                    iRoom = getRoom(WEST, roomsWide, iRoom);
                }
            } else {
                iDirection = WALL_EAST;
                bxArea = Box(rcRoom.x + rcRoom.w - TILE_SIZE, rcRoom.y + rcRoom.l, pos.z,
                             TILE_SIZE, TILE_SIZE, Z_HEIGHT);
                iState = WALL_EAST;
                iRoom = getRoom(SOUTH, roomsWide, iRoom);
            }
            break;
        case WALL_WEST:
            if(aWalls[iRoom].left) {
                iDirection = WALL_WEST;
                if(aWalls[iRoom].top) {
                    bxArea = Box(rcRoom.x, rcRoom.y, pos.z,
                                 TILE_SIZE, rcRoom.l - TILE_SIZE, Z_HEIGHT);
                    iState = WALL_NORTH;
                } else {
                    bxArea = Box(rcRoom.x, rcRoom.y - TILE_SIZE, pos.z,
                                 TILE_SIZE, rcRoom.l, Z_HEIGHT);
                    iRoom = getRoom(NORTH, roomsWide, iRoom);
                }
            } else {
                iDirection = WALL_SOUTH;
                bxArea = Box(rcRoom.x - TILE_SIZE, rcRoom.y + rcRoom.l - TILE_SIZE, pos.z,
                             TILE_SIZE, TILE_SIZE, Z_HEIGHT);
                iState = WALL_SOUTH;
                iRoom = getRoom(WEST, roomsWide, iRoom);
            }
            break;
        }

        //Now here's where it gets messy: we actually build the walls
        pCurWall = new Wall(we->genID(), imgWallStraight, imgWallCorner, bxArea, iDirection);
        printf("Wall id: %d (%2.2f,%2.2f,%2.2f, %d,%d,%d)\n", pCurWall->getID(), bxArea.x, bxArea.y, bxArea.z, bxArea.w, bxArea.l, bxArea.h);
        we->add(pCurWall);

        //Connect the walls
        if(pPrevWall != NULL) {
            pPrevWall->setNextWall(pCurWall);
        } else {
            pFirstWall = pCurWall;
        }
        pPrevWall = pCurWall;
    } while(iRoom != END_ROOM || iState != END_WALL);
    pCurWall->setNextWall(pFirstWall);

    //Free the walls once we're done
//    free(aWalls);
}

int getRoom(int iDirection, int iWidth, int iIndex) {
    switch(iDirection) {
    case NORTH:
        return iIndex + iWidth;
        break;
    case SOUTH:
        return iIndex - iWidth;
        break;
    case EAST:
        return iIndex + 1;
        break;
    case WEST:
        return iIndex - 1;
        break;
    }
    return -1;
}

Rect getRoomBounds(int iRoomsWide, int iWidth, int iRoom) {
    return Rect((iRoom % iRoomsWide) * iWidth,
                (iRoomsWide - (iRoom / iRoomsWide) - 1) * iWidth,
                iWidth,
                iWidth);    //Let's just assume square rooms
}


