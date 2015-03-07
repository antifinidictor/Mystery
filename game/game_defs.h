/*
 * game_defs.h
 * Definitions specific to the game
 */
#ifndef GAME_DEFS_H
#define GAME_DEFS_H

#include "mge/defs.h"
#include "mge/Event.h"

class Item;

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
    AUD_SPELL_POINT,
    AUD_CITY_MUSIC,
    AUD_SCHOOL_MUSIC,
    AUD_UNDERGROUND_MUSIC,
    AUD_CASTING,
    NUM_SOUNDS
};

//15 audio channels
enum AUDIO_CHANNELS {
    AUD_CHAN_PLAYER_SPELL_BACKGROUND,
    AUD_CHAN_ANY = -1
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
    IN_ROTATE_LEFT,     //Rotate the screen left
    IN_ROTATE_RIGHT,    //Rotate the screen right
    IN_SELECT,
    IN_RCLICK,
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
    ON_ITEM_DROPPED,        //Item dropped into a slot
    ON_UPDATE_HUD,          //HUD animations should be updated
    ON_ITEM_CLICK_USE,      //Current item receives a mouse click (button up/down, mouse location)
    ON_ITEM_DRAG_USE,       //Current item receives a mouse-drag event (mouse location)
    ON_OBJ_MOUSE_OVER,      //Object is moused over, doesn't need to be listening to receive this event
    ON_SAVE_GAME,
    ON_LOAD_GAME,
    ON_QUIT_GAME,
    ON_NEW_GAME,
    ON_OPTIONS,
    ON_ACTION_BUTTON,
    ON_INACTION_BUTTON,
    NUM_GAME_EVENTS
};

enum TypingKeys {
    TP_IN_SPACE = NUM_GAME_EVENTS,
    TP_IN_PERIOD,
    TP_IN_UNDERSCORE,
    TP_IN_BACKSPACE,
    TP_IN_ENTER,
    TP_IN_SLASH,
    TP_IN_COLON,
    TP_NUM_INPUTS
};

enum GameEventReturnCodes {
    EVENT_ITEM_CAN_DROP = NUM_EVENT_RETURN_CODES,   //Item can be dropped here
    EVENT_ITEM_CANNOT_DROP,                         //Item cannot be dropped here (should return to previous location)
    NUM_GAME_EVENT_RETURN_CODES
};

enum GameManagerState {
    GM_START,
    GM_NORMAL,
    GM_FADE_OUT,    //Fading between areas
    GM_FADE_IN,
    GM_FADE_AREA,
    GM_NEW_GAME,
    GM_LOAD_GAME,
    GM_CLEAN_GAME,
    GM_WAIT_FOR_WORLD_CLEAN,
    GM_NUM_STATES
};

enum HudContainers {
    HUD_MISC,
    HUD_TOPBAR,
    HUD_BOTTOMBAR,
    NUM_HUD_ELEMENTS
};

enum TopBarHudElements {
    MGHUD_BACKDROP,
    MGHUD_HEALTH_CONTAINER,
    MGHUD_ITEMBAR_CONTAINER,
    MGHUD_MAIN_CONTAINER,
    MGHUD_CUR_AREA,
    MGHUD_CUR_ACTION,
    MGHUD_SIDEBUTTON_CONTAINER,
    NUM_MGHUD_TOP_BAR_ELEMENTS
};

enum HealthContainerElements {
    MGHUD_HEALTH_BACKDROP_LEFT_EDGE,
    MGHUD_HEALTH_BACKDROP_MIDDLE,
    MGHUD_HEALTH_BACKDROP_RIGHT_EDGE,
    MGHUD_HEALTH_BAR,
    MGHUD_HEALTH_VALUE
};

enum ItembarElements {
    MGHUD_ELEMENT_ITEMBAR_CUR_ELEMENT,
    MGHUD_ELEMENT_ITEMBAR_CUR_SPELL,
    MGHUD_ELEMENT_ITEMBAR_CUR_ITEM
};

enum InventoryContainer {
    MGHUD_SPELL_CONTAINER,
    MGHUD_ITEM_CONTAINER,
    MGHUD_ELEMENT_CONTAINER,
};

enum SideButtonElements {
    MGHUD_SIDEBUTTON_ITEMNAME,
    MGHUD_SIDEBUTTON_ITEMDESC,
    MGHUD_SIDEBUTTON_NEWBUTTON,
    MGHUD_SIDEBUTTON_LOADBUTTON,
    MGHUD_SIDEBUTTON_SAVEBUTTON,
    MGHUD_SIDEBUTTON_QUITBUTTON,
    MGHUD_SIDEBUTTON_OPTIONSBUTTON
};

enum SideTypeElements {
    MGHUD_SIDETYPE_MESSAGE,
    MGHUD_SIDETYPE_TEXT,
    MGHUD_SIDETYPE_ACTION_BUTTON,
    MGHUD_SIDETYPE_INACTION_BUTTON
};

enum SideConfirmElements {
    MGHUD_SIDECONFIRM_MESSAGE,
    MGHUD_SIDECONFIRM_ACTION_BUTTON,
    MGHUD_SIDECONFIRM_INACTION_BUTTON
};

enum GameFlags {
    GAM_CAN_LINK = GAME_FLAGS_BEGIN,    //Can be transferred to new areas
    GAM_CAN_PICK_UP,                    //Item can be picked up by the player
//    GAM_WRITE_TO_SAVE_FILE,           //Flag replaced with PWE equivalent
    NUM_GAME_FLAGS
};

//Some useful event structures
struct ItemDropEvent {
    Item *item;
    uint itemOldIndex;
    uint itemNewIndex;
    float distance;
};

struct ItemUseEvent {
    Point m_ptMousePos;
    bool m_bMouseDown;
};

#endif
