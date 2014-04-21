/*
 * editor_defs.h
 * Definitions specific to the editor
 */
#ifndef EDITOR_DEFS_H
#define EDITOR_DEFS_H

#include "game/game_defs.h"

#define BUTTON_WIDTH  128
#define BUTTON_HEIGHT 32

enum EditorTypes {
    ED_TYPE_EDITOR_OBJECT = NUM_TYPES,
    ED_TYPE_EDITOR_MANAGER,
    ED_NUM_TYPES
};

enum EditorKeys {
    ED_IN_SPACE = NUM_GAME_EVENTS,
    ED_IN_PERIOD,
    ED_IN_UNDERSCORE,
    ED_IN_BACKSPACE,
    ED_IN_ENTER,
    ED_IN_SLASH,
    ED_IN_COLON,
    ED_NUM_INPUTS
};

//Events caused by interacting with the editor gui
enum EditorEvents {
    ED_ON_SET_AREA,
    ED_ON_NEW_AREA,
    ED_NUM_EVENTS
};

enum EditorAreas {
    ED_AREA_0
};

//This one looks wacky because it is responsible for making everything happen
enum EditorState {
    ED_STATE_INIT,
    ED_STATE_MAIN,
    ED_STATE_LOAD_FILE,
    ED_STATE_SAVE_FILE,
    ED_STATE_LIST_OBJECTS,
    ED_STATE_CREATE_OBJECT,
    ED_STATE_NAME_AREA,
    ED_STATE_LIST_TEXTURES,
    ED_STATE_ENTER_UINT,
    ED_STATE_ENTER_INT,
    ED_STATE_ENTER_FLOAT,
    ED_STATE_ENTER_STRING,
    ED_STATE_ENTER_COLOR,
    ED_STATE_SELECT_VOLUME,
    ED_STATE_SELECT_POINT,
    ED_STATE_NEW_TEXTURE,
    ED_NUM_STATES
};

enum EditorCursorState {
    EDC_STATE_STATIC,
    EDC_STATE_MOVE,
    EDC_STATE_SELECT_VOL,   //Select a volume
    EDC_STATE_SELECT_RECT,
    EDC_STATE_TYPE,         //Looks for ED_HUD_TEXT objs
    EDC_STATE_TYPE_FIELD,
    EDC_NUM_STATES
};

//First container layer huds
enum EditorHudBaseIds {
    ED_HUD_CURSOR_POS = NUM_HUD_ELEMENTS,  //Created and accessed by EditorCursor
    ED_HUD_RIGHT_PANE,  //Right pane: contains possible actions
    ED_HUD_MIDDLE_PANE, //Middle pane: contains fields during field entry
    ED_HUD_LEFT_PANE,   //Left pane: contains list of areas
    ED_HUD_NUM_BASE_IDS
};

//Main panel
enum EditorHudMainIds {
    ED_HUD_MAIN_LOAD_WORLD,
    ED_HUD_MAIN_SAVE_WORLD,
    ED_HUD_MAIN_NEW_AREA,
    ED_HUD_MAIN_RENAME_AREA,
    ED_HUD_MAIN_GO_TO_AREA,
    ED_HUD_MAIN_NEW_OBJ,
    ED_HUD_MAIN_NEW_TEXTURE,
    ED_HUD_NUM_MAIN_IDS
};

//Loading panel
enum EditorHudLoadWorldIds {
    ED_HUD_LOAD_CANCEL,
    ED_HUD_LOAD_LOAD,
    ED_HUD_NUM_LOAD_IDS
};

//Saving panel
enum EditorHudSaveWorldIds {
    ED_HUD_SAVE_CANCEL,
    ED_HUD_SAVE_SAVE,
    ED_HUD_NUM_SAVE_IDS
};

//Object creation panel
enum EditorHudNewObjIds {
    ED_HUD_NEW_OBJ_CANCEL,
    ED_HUD_NEW_OBJ_MAKE,
    ED_HUD_NEW_OBJ_UP,
    ED_HUD_NEW_OBJ_DOWN,
    ED_HUD_NEW_OBJ_LIST_PANE,
    ED_HUD_NUM_NEW_OBJ_IDS
};

//
enum EditorHudSelectionIds {
    ED_HUD_SEL_CANCEL,
    ED_HUD_SEL_FINALIZE,
    ED_HUD_SEL_SNAP_X,
    ED_HUD_SEL_SNAP_Y,
    ED_HUD_SEL_SNAP_Z,
    ED_HUD_NUM_SEL
};

//Area panel
enum EditorHudAreaIds {
    ED_HUD_AREA_NEW,
    ED_HUD_AREA_RENAME,
    ED_HUD_AREA_UP,
    ED_HUD_AREA_DOWN,
    ED_HUD_AREA_LIST_PANE,
    ED_HUD_NUM_AREA_IDS
};

//Field panel
enum EditorHudFieldIds {
    ED_HUD_FIELD_LABEL,
    ED_HUD_FIELD_TEXT,  //Accessed by EditorCursor
    ED_HUD_NUM_FIELDS
};

//Operations of buttons on the hud
enum EditorHudOps {
    ED_HUD_OP_CANCEL,       //Undo current input/back up a state
    ED_HUD_OP_FINALIZE,     //Opposite of cancel: accept current input
    ED_HUD_OP_LOAD_WORLD,   //Loads world from file
    ED_HUD_OP_SAVE_WORLD,   //Saves world to file
    ED_HUD_OP_NEW_AREA,     //Create new area
    ED_HUD_OP_RENAME_AREA,  //New HUD area name
    ED_HUD_OP_GO_TO_AREA,   //Go to existing area
    ED_HUD_OP_NEW_OBJ,      //List new object classes
    ED_HUD_OP_CHOOSE_OBJ,   //Desired class chosen
    ED_HUD_OP_NEW_TEXTURE,  //Create new texture
    ED_HUD_OP_UP,           //Go up one HUD display section
    ED_HUD_OP_DOWN,         //Go down one HUD display section
    ED_HUD_OP_UP_AREA,      //Go up one HUD display section
    ED_HUD_OP_DOWN_AREA,    //Go down one HUD display section
    //The following ops get attributes
    ED_HUD_OP_ATTR,
    ED_HUD_OP_CHOOSE_TEXTURE,
    //The following snap the position
    ED_HUD_OP_SNAP_X,
    ED_HUD_OP_SNAP_Y,
    ED_HUD_OP_SNAP_Z,
    ED_HUD_NUM_OPS
};

#endif //EDITOR_DEFS_H
