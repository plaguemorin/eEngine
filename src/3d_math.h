/**
 * Common 3D Math stuff
 * 
 * Philippe Lague-Morin
 */
#ifndef TD_MATH
#define TD_MATH

#include "global.h"

/**
 * Vectors
 */
typedef float[4] vec4_t;
typedef float[3] vec3_t;
typedef float[2] vec2_t;
typedef short[3] vec3s_t;
typedef short[2] vec2s_t;

typedef float[3] rgb;
typedef float[4] rgba;

typedef float matrix_t[16];
typedef float matrix3x3_t[9];

/**
 * Basic VERTEX; the base of triangle
 */
typedef struct {
    vec3_t          position;
    vec3_t          normal;
    vec3_t          tangent;
    vec2_t          textureCoord;
} vertex_t;

/**
 * Offset of things
 */
#define VERTEX_OFFSET_OF_POSITION       offsetof(vertex_t, position)
#define VERTEX_OFFSET_OF_NORMAL         offsetof(vertex_t, normal)
#define VERTEX_OFFSET_OF_TANGENT        offsetof(vertex_t, tangent)
#define VERTEX_OFFSET_OF_TEXTURECOORD   offsetof(vertex_t, textureCoord)

/**
 * 2D Texture Types
 */
typedef enum td_texture_type {
    TEXTURE_TYPE_UNKNOWN,
    TEXTURE_TYPE_RGB,
    TEXTURE_TYPE_RGBA,
} texture_type_t;

/**
 * 2D Texture
 */
typedef struct td_texture_t {
    unsigned char   *               data;
    unsigned int                    data_length;
    
    unsigned int                    width;
    unsigned int                    height;
    unsigned int                    bpp;
    texture_type_t                  type;
    
    /* Renderer Private Data */
    void                *           renderer_data;
} texture_t;

/**
 * Rendering properties
 */
typedef union td_properties_t {
    unsigned char props;
    struct {
        char        has_bumpmapping : 1;
        char        has_specular    : 1;
        char        has_diffuse     : 1;
        char        has_UNDEF1      : 1;
        char        has_UNDEF2      : 1;
        char        has_UNDEF3      : 1;
        char        has_alpha       : 1;
        char        has_shadows     : 1;
    };
} props_t;

/**
 * Material is the look of an object
 */
typedef struct td_material_t {
    /* Name of this material */
    char                *          name;
    
    /* Material Properties */
    props_t                        properties;
    
    /* */
    float                          shininess;
    
    /* */
    rgb                            specular_color;
    
    /* Textures */
    texture_t           *          texture_diffuse;
    texture_t           *          texture_bump;
    texture_t           *          texture_specular;
    
    /* Renderer Private Data */
    void                *          renderer_data;
} material_t;

/**
 * The minimum or smallest bounding or enclosing box for a 
 * point set in N dimensions is the box with the smallest 
 * measure within which all the points lie.
 */
typedef struct td_box_t {
    vec3_t      min;
    vec3_t      max;
} box_t;

/**
 * A 3D object is simply the mesh and the material to render
 * this does not have any position as many instance of this 
 * object may exist
 */
typedef struct td_object_t {
    /* Object name this should be unique */
    char                *           name;
    
    /* The look of this object */
    material_t          *           material;
   
    /* Object's vertex */
    vertex_t        *               vertices;

    /* Number of verticies */
    unsigned short                  num_verticies;
    
    /* Indicies pointing to the verticies */
    unsigned short      *           indices;

    /* Number of indicies */
    unsigned short                  num_indices;
    
    /* Bounding box */
    box_t                           bounding_box;

    /* Renderer Private Data */
    void                *           renderer_data;

} object_t;

/**
 * A light source
 */
typedef struct td_light_t {
    /* Light Position */
    vec4_t position;
    
    /* Where the light points to */
    vec3_t lookAt;
    
    /* The "UP" vector for the light */
    vec3_t upVector;
    
    /* Field-of-view of the light */
    float fov;
    
    /* Ambiant color */
    vec4_t ambient;
    
    /* Diffuse color */
    vec4_t diffuse;
    
    /* Specular color */
    vec4_t specula;
    
    /* Constant Attenuation */
    float constantAttenuation;
    
    /* Linear Attenuation */
    float linearAttenuation;
} light_t;

/**
 * A Camera view point
 * Normally there should be only one
 */
typedef struct td_camera_t {
    /* Position of the camera */
    vec4_t position;

    /* Forward direction */
    vec3_t forward;
    
    /* Right direction */
    vec3_t right;
    
    /* Up vector */
    vec3_t up;

    float pitch;
    float head;

    float aspect;
    float fov;
    float zNear;
    float zFar;
} camera_t;

#define vectorClear( a )                ((a)[0] = (a)[1] = (a)[2] = 0)
#define vectorNegate( a, b )            ((b)[0] = -(a)[0], (b)[1] = -(a)[1], (b)[2] = -(a)[2])
#define vectorSet( v, x, y, z )         ((v)[0] = (x), (v)[1] = (y), (v)[2] = (z))
#define vectorInverse( a )              ((a)[0] = (-a)[0], (a)[1] = (-a)[1], (a)[2] = (-a)[2])
#define vectorSubtract( a, b, c )       ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1], (c)[2] = (a)[2] - (b)[2])
#define vector2Subtract( a, b, c )      ((c)[0] = (a)[0] - (b)[0], (c)[1] = (a)[1] - (b)[1])
#define vectorAdd( a, b, c )            ((c)[0] = (a)[0] + (b)[0], (c)[1] = (a)[1] + (b)[1], (c)[2] = (a)[2] + (b)[2])
#define vectorCopy( a, b )              ((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
#define vector2Copy( a, b )             ((b)[0] = (a)[0], (b)[1] = (a)[1])
#define vectorScale( v, s, o )          ((o)[0] = (v)[0] * (s), (o)[1] = (v)[1] * (s), (o)[2] = (v)[2] * (s))
#define vectorMA( v, s, b, o )          ((o)[0] = (v)[0] + (b)[0]*(s), (o)[1] = (v)[1] + (b)[1] * (s), (o)[2] = (v)[2] + (b)[2] * (s))

#define dotProduct( x, y )              ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2])

/***************************************************************************************************************************
 * World Structures 
 **************************************************************************************************************************/

/**
 * Instance of an object
 */
typedef struct world_object_instance_t {
    /* The Object */
    struct td_object_t * object;
    
    /* This instance's transformation */
    matrix_t            transformation; 
    
    /* Should this object be in the collision system ? */
    BOOL                has_collision;
    
    /* Should we actually render this object ? */
    BOOL                is_active;
    
    /* The next instance */
    struct world_object_instance_t  * next;
} world_object_instance_t;

/**
 * THE World
 */
typedef struct world_t {
    unsigned int                        num_objects;
    world_object_instance_t     *       objects;
} world_t;


#endif
