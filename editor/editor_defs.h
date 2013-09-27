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
    ED_IN_SPACE = NUM_WORLD_EVENTS,
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
    ED_STATE_MAIN,
    ED_STATE_LOAD_FILE,
    ED_STATE_LOADING_FILE,
    ED_STATE_SAVE_FILE,
    ED_STATE_SELECT,
    ED_STATE_LIST_OBJECTS,
    ED_NUM_STATES
};

enum EditorCursorState {
    EDC_STATE_STATIC,
    EDC_STATE_MOVE,
    EDC_STATE_SELECT_VOL,   //Select a volume
    EDC_STATE_SELECT_RECT,
    EDC_STATE_TYPE,         //Looks for ED_HUD_TEXT objs
    EDC_NUM_STATES
};

//First container layer huds
enum EditorHudBaseIds {
    ED_HUD_CURSOR_POS,  //Created and accessed by EditorCursor
    ED_HUD_TEXT_FIELD,  //Accessed by EditorCursor
    ED_HUD_RIGHT_PANE,  //Right pane: contains possible actions
    ED_HUD_MIDDLE_PANE, //Middle pane: contains fields during field entry
    ED_HUD_LEFT_PANE,   //Left pane: contains list of areas
    ED_HUD_NUM_BASE_IDS
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
    ED_HUD_NEW_OBJ_UP,
    ED_HUD_NEW_OBJ_DOWN,
    ED_HUD_NEW_OBJ_LIST_START
};

//Texture loading panel

//Operations of buttons on the hud
enum EditorHudOps {
    ED_HUD_OP_CANCEL,       //Undo current input/back up a state
    ED_HUD_OP_FINALIZE,     //Opposite of cancel: accept current input
    ED_HUD_OP_NEW_OBJ,      //Create new object
    ED_HUD_OP_NEW_AREA,     //Create new area
    ED_HUD_OP_NEW_TEXTURE,  //Create new texture
    ED_HUD_OP_GO_TO_AREA,   //Go to existing area
    ED_HUD_OP_UP,           //Go up one HUD display section    
    ED_HUD_OP_DOWN,         //Go down one HUD display section
    ED_HUD_OP_NEW_AREA,     //New HUD area
    ED_HUD_NUM_OPS
};

#endif //EDITOR_DEFS_H
