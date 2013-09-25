/*
 * editor_defs.h
 * Definitions specific to the editor
 */
#ifndef EDITOR_DEFS_H
#define EDITOR_DEFS_H

#include "game/game_defs.h"

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
    ED_STATE_ENTER_TEXT,
    ED_NUM_STATES
};

enum EditorHudIds {
    ED_HUD_CURSOR_POS,
    ED_NUM_HUDS
};

#endif //EDITOR_DEFS_H
