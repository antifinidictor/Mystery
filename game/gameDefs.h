/*
 * gameDefs.h
 * Useful definitions specific to the game in question
 */

#ifndef GAME_DEFS_H
#define GAME_DEFS_H

#include "mge/defs.h"

enum GameFlags {
    GAM_SPELLABLE = GAME_FLAGS_BEGIN,   //True if spells can affect object
    GAM_ACTIVE,                         //True if object should respond to input
    GAM_NUM_FLAGS
};

enum InputID {
    IN_NORTH = MIN_NUM_MOUSE_INPUTS,
    IN_EAST,
    IN_SOUTH,
    IN_WEST,
    IN_SELECT,
    IN_CAST,    //Cast spell
    IN_HOME,    //Home screen
    IN_BREAK,   //Escape key
    IN_NUM_BOOL_INPUTS
};

enum StdImgs {
    IMG_FX,
    IMG_SPELLS,
    IMG_SHADOW,
    IMG_FONT_BASIC,
    IMG_PLAYER,
    IMG_NPC,
    IMG_TEXT_BUBBLE,
    IMG_BLOCK,
    IMG_BUTTON,
    IMG_H_EDGES,
    IMG_V_EDGES,
    IMG_CORNERS,
    IMG_TB,
    IMG_NUM_IMGS
};

//Each class of game object will have an associated ID
enum ObjTypes {
    OBJ_PLAYER,
    OBJ_WALL,
    OBJ_ITEM,
    OBJ_GUI,
    OBJ_DISPLAY,
    OBJ_NPC,
    OBJ_SPELL,
    OBJ_SURFACE,
    OBJ_SIMPLE_PHYSICS,
    OBJ_NUM_TYPES
};

#endif
