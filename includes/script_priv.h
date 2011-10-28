/*
 * script_priv.h
 *
 *  Created on: 2011-10-17
 *      Author: plaguemorin
 */

#ifndef SCRIPT_PRIV_H_
#define SCRIPT_PRIV_H_

typedef struct s_keybinding_t {
    unsigned long keySym;
    char * function;

    struct s_keybinding_t * next;
} keybinding_t;

typedef enum s_command_type {
    COMMAND_TYPE_KEY, COMMAND_TYPE_TOUCH, COMMAND_TYPE_MOVE
} command_type;

typedef struct s_command_t {
    command_type type;

    // for Type == KEY
    unsigned long keySym;

    // Used for both TYPE_MOVE and TYPE_TOUCH
    int x;
    int y;

    // Only for type TYPE_TOUCH
    char buttonNumber;

    struct s_command_t * next;
} command_t;

typedef struct s_worldentity_t {
    BOOL hasTick;
} world_entity;

extern char updateMethodName[256];
extern char buttonPressedMethodName[256];
extern char moveMethodName[256];
extern keybinding_t * keybindings;

void SCRIPTING_CallFunction(lua_State *L, const char *func, const char *sig, ...);

int luaopen_dengine_entity(lua_State *);
int luaopen_dengine_core(lua_State *);
int luaopen_dengine_callback(lua_State *);
int luaopen_dengine_camera(lua_State *);
int luaopen_dengine_world(lua_State *);

#endif /* SCRIPT_PRIV_H_ */
