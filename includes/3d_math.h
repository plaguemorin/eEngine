/**
 * Common 3D Math stuff
 * 
 * Philippe Lague-Morin
 */
#ifndef TD_MATH_H__
#define TD_MATH_H__

#define MAX_JOINT_PER_VERTEX 4

#define X 0
#define Y 1
#define Z 2
#define W 3

/**
 * Vectors
 */
typedef float vec4_t[4];
typedef float vec3_t[3];
typedef float vec2_t[2];
typedef short vec4s_t[4];
typedef short vec3s_t[3];
typedef short vec2s_t[2];

typedef float rgb[3];
typedef float rgba[4];

typedef float matrix_t[16];
typedef float matrix3x3_t[9];

typedef float quat4_t[4];

/**
 * Basic VERTEX; the base of triangle
 */
typedef struct {
    vec3_t position;
    vec3_t normal;
    vec2s_t textureCoord;
    vec3_t tangent;

    /* The blend factor for each bone/offset matrix */
    float weights[MAX_JOINT_PER_VERTEX];

    /* Index into the bone/offset matrix array */
    unsigned short boneId[MAX_JOINT_PER_VERTEX];
} vertex_t;

/**
 * Offset of things
 */
#define VERTEX_OFFSET_OF_POSITION       offsetof(vertex_t, position)
#define VERTEX_OFFSET_OF_NORMAL         offsetof(vertex_t, normal)
#define VERTEX_OFFSET_OF_TANGENT        offsetof(vertex_t, tangent)
#define VERTEX_OFFSET_OF_TEXTURECOORD   offsetof(vertex_t, textureCoord)
#define VERTEX_OFFSET_OF_WEIGHTS        offsetof(vertex_t, weights)
#define VERTEX_OFFSET_OF_BONEID         offsetof(vertex_t, boneId)

/**
 * 2D Texture Types
 */
typedef enum td_texture_type {
    TEXTURE_TYPE_UNKNOWN, TEXTURE_TYPE_RGB, TEXTURE_TYPE_RGBA, TEXTURE_TYPE_BGR, TEXTURE_TYPE_BGRA
} texture_type_t;

/**
 * 2D Texture
 */
typedef struct td_texture_t {
    /* Loaded Data as `type` (RGB or RGBA) */
    unsigned char * data;

    /* Size of loaded data */
    unsigned int data_length;

    /* Image information */
    unsigned int width;
    unsigned int height;

    /* Bits per pixel, 24 or 32 */
    unsigned int bpp;

    /* Loaded texture type: RGB, RGBA */
    texture_type_t type;

    /* Renderer Private Data */
    void * renderer_data;
} texture_t;

/**
 * Rendering properties
 */
typedef union td_properties_t {
    unsigned char props;
    struct {
        char has_bumpmapping :1;
        char has_specular :1;
        char has_diffuse :1;
        char has_UNDEF1 :1;
        char has_UNDEF2 :1;
        char has_UNDEF3 :1;
        char has_alpha :1;
        char has_shadows :1;
    };
} props_t;

/**
 * Material is the look of an object
 */
typedef struct td_material_t {
    /* Name of this material */
    char * name;

    /* Material Properties */
    props_t properties;

    /* */
    float shininess;

    /* */
    rgb specular_color;

    /* Textures */
    texture_t * texture_diffuse;
    texture_t * texture_bump;
    texture_t * texture_specular;

    /* Renderer Private Data */
    void * renderer_data;
} material_t;

/**
 * The minimum or smallest bounding or enclosing box for a 
 * point set in N dimensions is the box with the smallest 
 * measure within which all the points lie.
 */
typedef struct td_box_t {
    vec3_t min;
    vec3_t max;
} box_t;

/**
 *
 */
typedef struct td_joint_t {
    char *name;

    int parent;
    struct td_object_t * parent_joint;

    vec3_t position;
    quat4_t orientation;
} object_joint_t;

/**
 * A 3D object is simply the mesh and the material to render
 * this does not have any position as many instance of this 
 * object may exist
 */
typedef struct td_object_t {
    /* Object name this should be unique */
    char * name;

    /* The look of this object */
    material_t * material;

    /* Object's vertex */
    vertex_t * vertices;

    /* Number of verticies */
    unsigned short num_verticies;

    /* Indicies pointing to the verticies */
    unsigned short * indices;

    /* Number of indicies */
    unsigned short num_indices;

    /* Bounding box */
    box_t bounding_box;

    /* Renderer Private Data */
    void * renderer_data;
} object_t;

/**
 * Entities are complete objects with animations
 */
typedef struct td_entity_t {
    /* Name of this entity */
    char * name;

    /* Number of time this entity is in a world object */
    unsigned short num_references;

    /* Number of meshes (objects) this entity has */
    unsigned short num_objects;

    /* Objects of this entity */
    object_t * objects;

    /* Number of joints */
    unsigned short num_joints;

    /* Joints */
    object_joint_t * joints;

    /* Bounding box */
    box_t bounding_box;
} entity_t;

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

/**
 * Font for 2D HUD
 */
typedef struct td_font_t {
    unsigned char nCharWidth[128]; // width of each character
    unsigned char nMaxWidth; // box width
    unsigned char nMaxHeight; // box height
    unsigned int size;
    float wFrac;
    float hFrac;

    texture_t * texture;
} font_t;

/***************************************************************************************************************************
 * Operations
 **************************************************************************************************************************/
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

void vector_cross_product(const vec3_t v1, const vec3_t v2, vec3_t cross);
void normalize(vec3_t v);
void vector_interpolate(const vec3_t v1, const vec3_t v2, float f, vec3_t dest);

#define matrixValue(m, i, j)			m[  j + (i*4)  ]
#define matrixTranslate(m, x, y, z) 	(m[12] = (x), m[13] = (y), m[14] = (z))

void matrix_multiply_vertex_by_matrix(vec3_t pos, matrix_t modelViewProjectionMatrix, vec3_t dest);
void matrix_multiply(const matrix_t m1, const matrix_t m2, matrix_t dest);
void matrix_transform_vec4t(const matrix_t m1, const vec4_t vect, vec4_t dest);
void matrix_print(matrix_t m);
void matrix_load_identity(matrix_t m);
void matrix_copy(matrix_t from, matrix_t to);

void matrix_multiply3x3(const matrix3x3_t m1, const matrix3x3_t m2, matrix3x3_t dest);
void matrix_print3x3(matrix3x3_t m);
void matrix_transform_vec3t(const matrix3x3_t m1, const vec3_t vect, vec3_t dest);

void Quat_fromEuler(float pitch, float yaw, float roll, quat4_t dest);
void Quat_to_matrix(quat4_t q, matrix_t matrix);
void Quat_computeW(quat4_t q);
void Quat_normalize(quat4_t q);
void Quat_mult_quat(const quat4_t qa, const quat4_t qb, quat4_t out);
void Quat_mult_vec(const quat4_t q, const vec3_t v, quat4_t out);
void multiply_by_invert_quaternion(const vec3_t v1, const quat4_t quat, vec3_t dest);
void Quat_rotate_point(const quat4_t q, const vec3_t in, vec3_t out);
void Quat_rotate_short_point(const quat4_t q, const vec3s_t in, vec3_t out);

float Quat_dot_product(const quat4_t qa, const quat4_t qb);
void Quat_slerp(const quat4_t qa, const quat4_t qb, float t, quat4_t out);
void Quat_create_from_mat3x3(const matrix3x3_t matrix, quat4_t out);
void Quat_convert_to_mat3x3(matrix3x3_t matrix, const quat4_t out);

/***************************************************************************************************************************
 * World Structures 
 **************************************************************************************************************************/

/**
 * Instance of an object
 */
typedef struct world_object_instance_t {
    /* The Object */
    entity_t * object;

    /* This instance's transformation */
    //matrix_t transformation;
    /* This instance's position */
    vec3_t position;

    /* This instance's rotation */
    vec3_t rotation;

    /* Should this object be in the collision system ? */
    BOOL has_collision;

    /* Should we actually render this object ? */
    BOOL is_active;

    /* Scripting Engine Data */
    void * script_data;

    /* The next instance */
    struct world_object_instance_t * next;
} world_object_instance_t;

/**
 * THE World
 */
typedef struct world_t {
    unsigned int num_objects;
    world_object_instance_t * objects;
} world_t;

#endif
