/*
 * scene_node.c
 *
 *  Created on: 2011-10-27
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "scene_node.h"

void SCENE_InitializeNode(scene_node_t * new_scene_node, const char * name, scene_node_type_t type) {
    memset(new_scene_node, 0, sizeof(scene_node_t));
    new_scene_node->is_active = TRUE;

    if (name) {
        new_scene_node->name = (char *) malloc(sizeof(char) * (strlen(name) + 1));
        if (new_scene_node->name) {
            strcpy(new_scene_node->name, name);
        }
    }

    new_scene_node->type = type;
    switch (new_scene_node->type) {
        case NODE_TYPE_STATIC_MESH:
            new_scene_node->object.mesh = (mesh_t *) malloc(sizeof(mesh_t));
            break;
    }
}

scene_node_t * SCENE_NewNode(const char * name, scene_node_type_t type) {
    scene_node_t * new_scene_node;

    new_scene_node = (scene_node_t*) malloc(sizeof(scene_node_t));

    if (new_scene_node) {
        SCENE_InitializeNode(new_scene_node, name, type);
    } else {
        /* CRASH */

    }

    return new_scene_node;
}

scene_node_t * SCENE_NewNodeWithParent(scene_node_t * parent, const char * name, scene_node_type_t type) {
    scene_node_t * node;

    node = SCENE_NewNode(name, type);
    SCENE_AttachNodeAsChildOf(parent, node);

    return node;
}

scene_node_t * SCENE_DeepCopy(scene_node_t * node) {
    scene_node_t * copy;

    copy = (scene_node_t*) malloc(sizeof(scene_node_t));
    memset(copy, 0, sizeof(scene_node_t));

    copy->is_active = node->is_active;
    memcpy(&copy->bounding_box, &node->bounding_box, sizeof(box_t));

    vectorCopy(node->position, copy->position);
    vectorCopy(node->rotation, copy->rotation);

    if (node->name) {
        copy->name = (char *) malloc(sizeof(char) * (strlen(node->name) + 1));
        if (copy->name) {
            strcpy(copy->name, node->name);
        }
    }

    copy->type = node->type;
    copy->object = node->object;

    SCENE_AttachNodeAsChildOf(node->parent, copy);

    while (copy->num_children != node->num_children) {
        SCENE_DeepCopy(node->child[copy->num_children]);
    }

    return copy;
}

void SCENE_AttachNodeAsChildOf(scene_node_t * parent, scene_node_t * child) {
    parent->num_children++;
    parent->child = (scene_node_t **) realloc(parent->child, parent->num_children * sizeof(scene_node_t*));
    parent->child[parent->num_children - 1] = child;

    child->parent = parent;
}

void SCENE_FreeNode(scene_node_t * node) {
    free(node->name);
    free(node->script_data);

    while (node->num_children > 0) {
        SCENE_FreeNode(node->child[node->num_children - 1]);
    }

    free(node->child);
}

void SCENE_DumpNode(scene_node_t * node, unsigned int level) {
    unsigned int i, j;
    i = level;
    while (i--)
        putchar(' ');

    printf("Node '%s' ", node->name);
    printf("is of type %d and has %d children\n", node->type, node->num_children);

    for (j = 0; j < node->num_children; j++) {
        i = level;
        while (i--)
            putchar(' ');

        printf("Child %d:\n", j);
        SCENE_DumpNode(node->child[j], level + 1);
    }
}
