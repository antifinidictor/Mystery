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

enum EditorState {
    ED_STATE_NORMAL,
    ED_STATE_LOAD_FILE,
    ED_STATE_SAVE_FILE,
    ED_STATE_SELECT,
    ED_NUM_STATES
};

enum EditorHudIds {
    ED_HUD_CURSOR_POS,
    ED_HUD_NEW,
    ED_HUD_CREATE,
    ED_HUD_CANCEL,
    ED_LOAD,
    ED_LOAD_FILE,
    ED_SAVE,
    ED_SAVE_FILE,
    ED_TEXT,
    ED_MESSAGE,
    ED_NUM_HUDS
};

#endif //EDITOR_DEFS_H
