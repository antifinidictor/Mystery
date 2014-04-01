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
    TYPE_GENERAL,
    TYPE_PLAYER,
    TYPE_MANAGER,
    TYPE_ITEM,
    TYPE_ELEMENTAL_VOLUME,
    TYPE_GUI,
    NUM_TYPES
};

enum IMAGE_TYPES {
    IMG_NONE,   //Reserved: No image
    IMG_FONT,
    IMG_BUTTON,
    IMG_MOUSE,
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
    AUD_PICKUP,
    AUD_LIFT,
    AUD_DRAG,
    AUD_POPUP,
    AUD_POPDOWN,
    AUD_CITY_MUSIC,
    AUD_SCHOOL_MUSIC,
    AUD_UNDERGROUND_MUSIC,
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
    IN_TOGGLE_DEBUG_MODE,
    IN_SHIFT,   //Modify key #1
    IN_CTRL,    //Modify key #2
    IN_CAST,    //Cast spell
    IN_HOME,    //Home screen
    IN_BREAK,   //Escape key
    IN_NUM_BOOL_INPUTS
};

enum GameEvent {
    ON_AREA_FADE_OUT = GAME_EVENTS_BEGIN,    //When an area starts fading out
    ON_AREA_FADE_IN,
    ON_SPELL_DIVIDE_CAST,   //Spell that divides an element amount BETWEEN two selected volumes
    ON_SPELL_SOURCE_CAST,   //Spell that causes an element to flow from one point to another WITHIN a volume (called once for source, once for sink)
    ON_SPELL_FLOW_CAST,     //Spell that causes a flow within the specified volume
    ON_SPELL_CANCEL,        //Spell cancelled
    NUM_GAME_EVENTS
};


enum GameManagerState {
    GM_START,
    GM_NORMAL,
    GM_FADE_OUT,    //Fading between areas
    GM_FADE_IN,
    GM_NUM_STATES
};

enum HudContainers {
    HUD_TOPBAR,
    HUD_BOTTOMBAR,
    NUM_HUD_ELEMENTS
};

enum TopBarHudElements {
    MGHUD_BACKDROP,
    MGHUD_AREA_NAME,
    MGHUD_HEALTH_CONTAINER,
    MGHUD_ITEMBAR_CONTAINER,
    MGHUD_INVENTORY_CONTAINER,
    NUM_MGHUD_TOP_BAR_ELEMENTS
};

enum HealthContainerElements {
    MGHUD_HEALTH_BACKDROP_LEFT_EDGE,
    MGHUD_HEALTH_BACKDROP_MIDDLE,
    MGHUD_HEALTH_BACKDROP_RIGHT_EDGE,
    MGHUD_HEALTH_BAR,
    MGHUD_HEALTH_VALUE
};

enum InventoryElements {
    MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT,
    MGHUD_ELEMENT_ITEMBAR_CUR_SPELL,
    MGHUD_ELEMENT_ITEMBAR_CUR_ITEM,
    MGHUD_ELEMENT_THUMBNAIL_START= 10   //Max 10 items in item bar
};

enum GameFlags {
    GAM_CAN_LINK = GAME_FLAGS_BEGIN,
    NUM_GAME_FLAGS
};

#endif
