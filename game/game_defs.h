/*
 * game_defs.h
 * Definitions specific to the game
 */
#ifndef GAME_DEFS_H
#define GAME_DEFS_H

#include "mge/defs.h"

enum GAME_AREA {
    GM_START_PAGE,
    GM_MAIN_GAME,
    GM_NUM_AREAS
};

enum OBJECT_TYPES {
    TYPE_PLAYER,
    TYPE_GENERAL,
    NUM_TYPES
};

enum IMAGE_TYPES {
    IMG_NONE,   //Reserved: No image
    IMG_FONT,
    IMG_PLAYER,
    IMG_BLOCK,
    IMG_WALL_TOP,
    IMG_WALL_BOTTOM,
    IMG_WALL_SIDE,
    NUM_IMAGES
};

enum SOUND_TYPES {
    AUD_NONE,   //Reserved: No sound
    AUD_STEP,
    NUM_SOUNDS
};

enum RENDER_ORDER {
    ORDER_FLOOR,
    ORDER_LOW_FX,
    ORDER_OBJECTS,
    ORDER_HIGH_FX,
    ORDER_CEILING
};

enum InputID {
    IN_NORTH = MIN_NUM_MOUSE_INPUTS,
    IN_EAST,
    IN_SOUTH,
    IN_WEST,
    IN_SELECT,
    IN_SHIFT,   //Modify key #1
    IN_CTRL,    //Modify key #2
    IN_CAST,    //Cast spell
    IN_HOME,    //Home screen
    IN_BREAK,   //Escape key
    IN_NUM_BOOL_INPUTS
};

enum WorldEvent {
    ON_WORLD_CHANGE = IN_NUM_BOOL_INPUTS,    //When an area-of-effect event occurs
    NUM_WORLD_EVENTS
};

#endif
