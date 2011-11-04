/*
 * script_world.c
 *
 *  Created on: 2011-10-21
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "entities.h"
#include "renderer.h"
#include "world.h"
#include "scene_node.h"

#include "script_priv.h"

#define SCENE_NODE "scene_node_t"

static scene_node_t ** toscenenode(lua_State *L, int index) {
    scene_node_t ** bar;
    bar = (scene_node_t **) lua_touserdata(L, index);
    if (bar == NULL ) luaL_typerror(L, index, SCENE_NODE);
    return bar;
}

static scene_node_t ** checkscenenode(lua_State *L, int index) {
    scene_node_t ** bar;
    luaL_checktype(L, index, LUA_TUSERDATA);
    bar = (scene_node_t **) luaL_checkudata(L, index, SCENE_NODE);

    if (bar == NULL) {
        luaL_typerror(L, index, SCENE_NODE);
    }

    if (*bar == NULL) {
        luaL_typerror(L, index, SCENE_NODE);
    }

    return bar;
}

static scene_node_t ** pushscenenode(lua_State *L) {
    scene_node_t ** bar;
    bar = (scene_node_t **) lua_newuserdata(L, sizeof(scene_node_t*));
    luaL_getmetatable(L, SCENE_NODE);
    lua_setmetatable(L, -2);
    return bar;
}

static void putscenenode(lua_State *L, scene_node_t * node) {
    scene_node_t ** bar;
    bar = pushscenenode(L);
    *bar = node;
}

static int SceneNode_New(lua_State *L) {
    const char * x;
    scene_node_t ** bar;

    x = luaL_optlstring(L, 1, NULL, NULL);
    bar = pushscenenode(L);

    *bar = SCENE_NewNode(x, NODE_TYPE_DUMMY);
    return 1;
}

static int SceneNode_LoadMesh(lua_State *L) {
    const char * meshName;
    BOOL result;
    scene_node_t ** self;

    self = checkscenenode(L, 1);
    meshName = luaL_checklstring(L, 2, NULL);

    result = ENTITY_LoadObject(meshName, *self);
    if (result) {
        result = REN_MakeNodeAvailable(*self);
    }

    lua_pushboolean(L, result);
    return 1;
}

static int SceneNode_Attach(lua_State * L) {
    scene_node_t ** self;
    scene_node_t ** parent;

    self = checkscenenode(L, 1);

    if (lua_isnil(L, 2)) {
        parent = &engine->world->root_node;
    } else {
        parent = checkscenenode(L, 2);
    }

    SCENE_AttachNodeAsChildOf(*parent, *self);
    return 0;
}

static int SceneNode_Move(lua_State * L) {
    scene_node_t ** self;
    float x, y, z;

    self = checkscenenode(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    z = luaL_checknumber(L, 4);

    vectorSet((*self)->position, x, y, z);
    return 0;
}

static int SceneNode_Rotate(lua_State * L) {
    scene_node_t ** self;
    float x, y, z;

    self = checkscenenode(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    z = luaL_checknumber(L, 4);

    vectorSet((*self)->rotation, x, y, z);
    return 0;
}

static int SceneNode_Dump(lua_State * L) {
    scene_node_t ** self;
    self = checkscenenode(L, 1);

    SCENE_DumpNode(*self, 0);
    return 0;
}

static scene_node_t * child_by_name(scene_node_t * node, const char * name) {
    scene_node_t * i;
    unsigned int j;

    if (strcmp(node->name, name) == 0) {
        return node;
    }

    for (j = 0; j < node->num_children; j++) {
        i = child_by_name(node->child[j], name);
        if (i) {
            return i;
        }
    }

    return NULL;
}

static int SceneNode_ChildByName(lua_State * L) {
    scene_node_t ** self;
    scene_node_t * child;
    const char * name;

    self = checkscenenode(L, 1);
    name = luaL_checklstring(L, 2, NULL);

    child = child_by_name(*self, name);
    if (child) {
        putscenenode(L, child);
        return 1;
    }

    return 0;
}

static int SceneNode__gc(lua_State *L) {
    printf("Trashing node %p\n", checkscenenode(L, 1));
    return 0;
}

static int SceneNode__tostring(lua_State *L) {
    scene_node_t ** self;
    char buff[32];

    self = checkscenenode(L, 1);

    sprintf(buff, "%p %p %s", self, *self, (*self)->name);
    lua_pushfstring(L, "SceneNode (%s)", buff);
    return 1;
}

static const luaL_Reg entityMethodsLib[] = {
/* New Dummy Node */
{ "new", SceneNode_New },
/* New Mesh node */
{ "loadMesh", SceneNode_LoadMesh },
/* Attach node to another node */
{ "attachTo", SceneNode_Attach },
/* Move a node */
{ "move", SceneNode_Move },
/* Rotate a node */
{ "rotate", SceneNode_Rotate },
/* Find child by name */
{ "child", SceneNode_ChildByName },
/* Debug */
{ "dump", SceneNode_Dump },
/* end */
{ NULL, NULL } };

static const luaL_Reg entityMetaLib[] = { { "__gc", SceneNode__gc }, { "__tostring", SceneNode__tostring }, { NULL, NULL } };

int luaopen_dengine_world(lua_State *L) {
    /* create methods table, add it to the globals */
    luaL_openlib(L, SCENE_NODE, entityMethodsLib, 0);

    /* create metatable for SCENE_NODE, and add it to the Lua registry */
    luaL_newmetatable(L, SCENE_NODE);

    /* fill metatable */
    luaL_openlib(L, 0, entityMetaLib, 0);
    lua_pushliteral(L, "__index");

    /* dup methods table*/
    lua_pushvalue(L, -3);

    /* metatable.__index = methods */
    lua_rawset(L, -3);

    /* dup methods table*/
    lua_pushliteral(L, "__metatable");

    /* hide metatable:     metatable.__metatable = methods */
    lua_pushvalue(L, -3);

    /* drop metatable */
    lua_rawset(L, -3);

    /* return methods on the stack */
    lua_pop(L, 1);

    /* After SCENE_NODE register the methods are still on the stack, remove them. */
    lua_pop(L, 1);
    return 1;
}
