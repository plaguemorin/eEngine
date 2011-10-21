/*
 * renderer_gl.c
 *
 *  Created on: 2011-09-29
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#if defined(__APPLE__)
#	include <OpenGLES/ES1/gl.h>
#	include <OpenGLES/ES1/glext.h>
#else
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#endif

#include "global.h"
#include "3d_math.h"
#include "engine.h"
#include "filesystem.h"

#undef GL_DEBUG_RENDER

#ifdef GL_DEBUG_RENDER
#define glCheckErrors(a)		engine->isRunning = CheckErrorsF(__PRETTY_FUNCTION__, a)
#else
#define glCheckErrors(a)
#endif

#define DEG_TO_RAD (2.0f*3.14159265f/360.0f)

typedef enum prog_location {
    OBJECT_MEMLOC_VRAM, OBJECT_MEMLOC_RAM,
} prog_location;

typedef struct simple_shader_var_t {
    GLuint index;

    GLsizei length;
    GLint size;
    GLenum type;
    GLchar* name;

    BOOL isUniform;
} shader_var_t;

typedef struct simple_shader_t {
    GLuint prog;

    GLuint num_vars;
    shader_var_t * vars;
} shader_prog_t;

typedef struct renderer_prog_object_t {
    GLuint vertexVboId;
    GLuint indiciesVboId;
    prog_location memory_location;
    shader_prog_t * shader;
} renderer_prog_object_t;

matrix_t projectionMatrix;
matrix_t modelViewMatrix;
matrix_t modelViewProjectionMatrix;

matrix_t* matrixStackPointer;
matrix_t matrixStack[16];

/* Default Object Shader */
static shader_prog_t * defaultObjectShader;

/* Text shader */
static shader_prog_t * textObjectShader;

/* The shader currently binded */
static shader_prog_t * currentShader;

typedef struct renderer_prog_texture_t {
    GLuint texId;
} renderer_prog_texture_t;


GLuint shadowFBOId;
GLuint shadowMapTextureId;

float shadowMapRation = 1.5f;
int tWidth;
int tHeight;

/**
 * Check for OpenGL errors
 */
static BOOL CheckErrorsF(const char* step, const char* details) {
    glFlush();
    GLenum err = glGetError();
    switch (err) {
        case GL_INVALID_ENUM:
            printf("[GLES2] Error GL_INVALID_ENUM %s, %s\n", step, details);
            break;
        case GL_INVALID_VALUE:
            printf("[GLES2] Error GL_INVALID_VALUE  %s, %s\n", step, details);
            break;
        case GL_INVALID_OPERATION:
            printf("[GLES2] Error GL_INVALID_OPERATION  %s, %s\n", step, details);
            break;
        case GL_OUT_OF_MEMORY:
            printf("[GLES2] Error GL_OUT_OF_MEMORY  %s, %s\n", step, details);
            break;
        case GL_NO_ERROR:
            return YES;
            break;
        default:
            printf("[GLES2] Error UNKNOWN  %s, %s \n", step, details);
            break;
    }

    return NO;
}

static void FreeShader(shader_prog_t * shaderProg) {
    unsigned int i;
    shader_var_t * v;

    /* Free vars */
    for (i = 0; i < shaderProg->num_vars; i++) {
        v = &shaderProg->vars[i];
        free(v->name);
    }

    glDeleteProgram(shaderProg->prog);

    free(shaderProg->vars);
    free(shaderProg);
}

/**
 * Loads a shader
 */
static GLuint LoadShader(const char *shaderSrcPath, GLenum type) {
    GLuint shader;
    GLint infoLen = 0;
    GLint compiled;
    filehandle_t* shaderFile;

    // Create the shader object
    shader = glCreateShader(type);
    if (shader == 0) {
        printf("Failed to created GL shader for '%s'\n", shaderSrcPath);
        return 0;
    }

    shaderFile = FS_OpenFileRead(shaderSrcPath);
    if (!shaderFile) {
        printf("Could not load shader: %s\n", shaderSrcPath);
        glDeleteShader(shader);
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, (const GLchar**) &shaderFile->ptrStart, NULL);
    CheckErrorsF(__FUNCTION__, "glShaderSource");

    // Compile the shader
    glCompileShader(shader);
    CheckErrorsF(__FUNCTION__, "glCompileShader");

    // Release the file
    FS_Close(shaderFile);

    // Check the compile status
    glFlush();
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1) {
        char* infoLog = malloc(sizeof(char) * infoLen);
        if (infoLog) {
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("[GLES2] Log for '%s' compiling shader:\n%s\n", shaderSrcPath, infoLog);
            free(infoLog);
        } else {
            printf("[GLES2] Unable to allocate %d bytes for log\n", infoLen);
        }
    }

    if (!compiled) {
        printf("[GLES2] Error, unloading shader\n");
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLint CurrentShaderFind(const char * name) {
    unsigned int i;
    shader_var_t * v;

    /* Free vars */
    for (i = 0; i < currentShader->num_vars; i++) {
        v = &currentShader->vars[i];

        if (strcmp(name, v->name) == 0) {
            return v->index;
        }
    }

    return -1;
}

static void BindVariables(shader_prog_t * shaderProg) {
    shader_var_t * currentVar;
    GLint numActiveAttributes;
    GLint numActiveUniforms;
    GLint maxLen;
    GLint attributeMaxLen;
    GLint uniformMaxLen;
    GLint i;

    char * buffer;

    glGetProgramiv(shaderProg->prog, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);
    glGetProgramiv(shaderProg->prog, GL_ACTIVE_UNIFORMS, &numActiveUniforms);

    shaderProg->num_vars = numActiveAttributes + numActiveUniforms;

    if (numActiveAttributes > 0 || numActiveUniforms > 0) {
        glGetProgramiv(shaderProg->prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeMaxLen);
        glGetProgramiv(shaderProg->prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLen);

        maxLen = MAX(attributeMaxLen, uniformMaxLen);
        buffer = (char*) malloc(maxLen);

        shaderProg->vars = (shader_var_t *) malloc(shaderProg->num_vars * sizeof(shader_var_t));

        for (i = 0; i < numActiveAttributes; i++) {
            currentVar = &shaderProg->vars[i];
            currentVar->isUniform = NO;
            glGetActiveAttrib(shaderProg->prog, i, maxLen, &currentVar->length, &currentVar->size, &currentVar->type, buffer);

            currentVar->name = (char *) malloc(sizeof(char) * (currentVar->length + 1));
            memset(currentVar->name, 0, sizeof(char) * (currentVar->length + 1));
            strncpy(currentVar->name, buffer, currentVar->length + 1);

            currentVar->index = glGetAttribLocation(shaderProg->prog, currentVar->name);
            printf("[GLES2] Shader has variable %s with location %d\n", currentVar->name, currentVar->index);
        }

        for (i = 0; i < numActiveUniforms; i++) {
            currentVar = &shaderProg->vars[i + numActiveAttributes];
            currentVar->isUniform = YES;
            glGetActiveUniform(shaderProg->prog, i, maxLen, &currentVar->length, &currentVar->size, &currentVar->type, buffer);

            currentVar->name = (char *) malloc(sizeof(char) * (currentVar->length + 1));
            memset(currentVar->name, 0, sizeof(char) * (currentVar->length + 1));
            strncpy(currentVar->name, buffer, currentVar->length + 1);

            currentVar->index = glGetUniformLocation(shaderProg->prog, currentVar->name);
            printf("[GLES2] Shader has variable %s with location %d\n", currentVar->name, currentVar->index);
        }

        free(buffer);
    }
}

static void LoadProgram(shader_prog_t* shaderProg, const char* vertexShaderPath, const char* fragmentShaderPath) {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;
    GLint infoLen = 0;

    printf("[GLES2] Loading vertex shader %s and fragment shader %s\n", vertexShaderPath, fragmentShaderPath);

    //Load simple shader
    vertexShader = LoadShader(vertexShaderPath, GL_VERTEX_SHADER);
    fragmentShader = LoadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0) {
        printf("[GLES2] Unable to load shaders\n");

        if (vertexShader) {
            glDeleteShader(vertexShader);
        }

        if (fragmentShader) {
            glDeleteShader(fragmentShader);
        }

        return;
    }

    memset(shaderProg, 0, sizeof(shader_prog_t));

    // Create the program object
    shaderProg->prog = glCreateProgram();
    if (shaderProg->prog == 0) {
        CheckErrorsF(__FUNCTION__, "glCreateProgram");
        printf("[GLES2] Could not create GL program.");
        return;
    }

    // Attach them
    glAttachShader(shaderProg->prog, vertexShader);
    glAttachShader(shaderProg->prog, fragmentShader);

    // Link the program
    glLinkProgram(shaderProg->prog);

    // Check the link status
    glFlush();
    glGetProgramiv(shaderProg->prog, GL_LINK_STATUS, &linked);

    // Print out the logs
    glGetProgramiv(shaderProg->prog, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) {
        char* infoLog = malloc(sizeof(char) * infoLen);
        if (infoLog) {
            glGetProgramInfoLog(shaderProg->prog, infoLen, NULL, infoLog);
            printf("[GLES2] Linking program log:\n%s\n", infoLog);
            free(infoLog);
        } else {
            printf("[GLES2] Unable to allocate %d bytes for program linker log\n", infoLen);
        }
    }

    if (glIsProgram(shaderProg->prog) == GL_FALSE) {
        printf("[GLES2] Error loading program!\n");
    }

    if (!linked) {
        printf("[GLES2] Unable to link\n");
        glDeleteProgram(shaderProg->prog);
        return;
    }

    /* Dump info about the shaders */
    glGetProgramiv(shaderProg->prog, GL_ATTACHED_SHADERS, &infoLen);
    printf("[GLES2] Shader has %d programs linked to it\n", infoLen);

    BindVariables(shaderProg);

    printf("[GLES2] Shader is ready\n");
}

static void UseShader(shader_prog_t * shader) {
    currentShader = shader;
    glUseProgram(shader->prog);
}

static BOOL currentShaderBindVertexAttribPointer(const char * name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) {
    GLint indx;

    indx = CurrentShaderFind(name);
    if (indx >= 0) {
        glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
        glEnableVertexAttribArray(indx);
        return TRUE;
    }

    return FALSE;
}

static BOOL currentShaderUniformMatrix(const char * name, GLsizei count, GLboolean transpose, matrix_t value) {
    GLint indx;

    indx = CurrentShaderFind(name);
    if (indx >= 0) {
        glUniformMatrix4fv(indx, count, transpose, value);
        return TRUE;
    }

    return FALSE;
}

static void Perspective(float fovy, float aspect, float zNear, float zFar, matrix_t projectionMatrix) {
    float f = (float) (1 / tan(fovy * DEG_TO_RAD / 2));

    projectionMatrix[0] = f / aspect;
    projectionMatrix[4] = 0;
    projectionMatrix[8] = 0;
    projectionMatrix[12] = 0;
    projectionMatrix[1] = 0;
    projectionMatrix[5] = f;
    projectionMatrix[9] = 0;
    projectionMatrix[13] = 0;
    projectionMatrix[2] = 0;
    projectionMatrix[6] = 0;
    projectionMatrix[10] = (zFar + zNear) / (zNear - zFar);
    projectionMatrix[14] = 2 * (zFar * zNear) / (zNear - zFar);
    projectionMatrix[3] = 0;
    projectionMatrix[7] = 0;
    projectionMatrix[11] = -1;
    projectionMatrix[15] = 0;
}

static void LookAt(vec3_t vEye, vec3_t vLookat, vec3_t vUp, matrix_t fModelView) {
    vec3_t vN, vU, vV;

    // determine the new n
    vectorSubtract(vEye, vLookat, vN);

    // determine the new u by crossing with the up vector
    vector_cross_product(vUp, vN, vU);

    // normalize both the u and n vectors
    normalize(vU);
    normalize(vN);

    // determine v by crossing n and u
    vector_cross_product(vN, vU, vV);

    // create a model view matrix
    fModelView[0] = vU[0];
    fModelView[4] = vU[1];
    fModelView[8] = vU[2];
    fModelView[12] = -dotProduct(vEye, vU);
    fModelView[1] = vV[0];
    fModelView[5] = vV[1];
    fModelView[9] = vV[2];
    fModelView[13] = -dotProduct(vEye, vV);
    fModelView[2] = vN[0];
    fModelView[6] = vN[1];
    fModelView[10] = vN[2];
    fModelView[14] = -dotProduct(vEye, vN);
    fModelView[3] = 0.0f;
    fModelView[7] = 0.0f;
    fModelView[11] = 0.0f;
    fModelView[15] = 1.0f;
}

/**
 * Check for framebuffer errors
 */
static BOOL CheckFBStatus() {
    GLenum status;

    glFlush();
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            printf("[GLES2] GL_FRAMEBUFFER_COMPLETE\n");
            return YES;
            break;
        case 0x8CDB:
            printf("[GLES2] GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            printf("[GLES2] GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            printf("[GLES2] GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
            break;
            //case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:            printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS\n");break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf("[GLES2] GL_FRAMEBUFFER_UNSUPPORTED\n");
            break;
        default:
            printf("[GLES2] Unknown issue (%x).\n", status);
            break;
    }

    return NO;
}

static BOOL init(int w, int h) {
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    //glEnable(GL_DEPTH_TEST);
    /*
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(TRUE);
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFFFFFFFF);
    glStencilFunc(GL_EQUAL, 0x00000000, 0x00000001);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearStencil(0);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glActiveTexture(GL_TEXTURE0);
    */

    defaultObjectShader = (shader_prog_t *) malloc(sizeof(shader_prog_t));
    if (defaultObjectShader) {
        LoadProgram(defaultObjectShader, "data/shaders/default_object_vertex.glsl", "data/shaders/default_object_fragment.glsl");
    }

    textObjectShader = (shader_prog_t *) malloc(sizeof(textObjectShader));
    if (textObjectShader) {
        LoadProgram(textObjectShader, "data/shaders/v_text.glsl", "data/shaders/f_text.glsl");
    }
/*
    GLuint   depthRenderbuffer;

    glGenFramebuffers(1, &shadowFBOId);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBOId);

    glGenTextures(1, &shadowMapTextureId);
    glBindTexture(GL_TEXTURE_2D, shadowMapTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tWidth, tHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMapTextureId,0);
    glBindTexture(GL_TEXTURE_2D, -1);

    glGenRenderbuffers(1, &depthRenderbuffer);
    SCR_CheckErrorsF("CreateFBOandShadowMap", "glGenRenderbuffers");

    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    SCR_CheckErrorsF("CreateFBOandShadowMap", "glBindRenderbuffer");

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tWidth, tHeight);
    SCR_CheckErrorsF("CreateFBOandShadowMap", "glRenderbufferStorage");

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    SCR_CheckErrorsF("CreateFBOandShadowMap", "glFramebufferRenderbuffer");
    */
    if (!CheckErrorsF("init", "no details")) {
        engine->isRunning = NO;
    }

    CheckFBStatus();
    return YES;
}

static BOOL unregisterObject(object_t * object) {
    renderer_prog_object_t * objData;

    if (object && object->renderer_data) {
        objData = (renderer_prog_object_t *) object->renderer_data;

        if (objData->vertexVboId > 0) {
            glDeleteBuffers(1, (const GLuint *) &objData->vertexVboId);
            objData->vertexVboId = 0;
        }

        if (objData->indiciesVboId > 0) {
            glDeleteBuffers(1, (const GLuint *) &objData->indiciesVboId);
            objData->indiciesVboId = 0;
        }

        free(objData);
        object->renderer_data = NULL;
    }

    return TRUE;
}

static BOOL registerObject(object_t * object) {
    renderer_prog_object_t * objData;

    if (!object) {
        printf("[GLES2] Object data was NULL nothing to upload\n");
        return NO;
    }

    if (object->renderer_data) {
        printf("[GLES2] Object %s seems already uploaded\n", object->name);
        return NO;
    }

    objData = malloc(sizeof(renderer_prog_object_t));
    if (!objData) {
        printf("[GLES2] Unable to allocate memory for object data\n");
        return NO;
    }

    object->renderer_data = objData;
    objData->memory_location = OBJECT_MEMLOC_VRAM;
    objData->shader = defaultObjectShader;

    glGenBuffers(1, &objData->vertexVboId);
    glGenBuffers(1, &objData->indiciesVboId);

    UseShader(objData->shader);

    if (objData->vertexVboId > 0 && objData->indiciesVboId > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, objData->vertexVboId);
        glBufferData(GL_ARRAY_BUFFER, object->num_verticies * sizeof(vertex_t), object->vertices, GL_STATIC_DRAW);
        glCheckErrors("Array Buffer");

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objData->indiciesVboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, object->num_indices * sizeof(unsigned short), object->indices, GL_STATIC_DRAW);
        glCheckErrors("Buffer Data Indices");

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return YES;
    }

    printf("[GLES2] Unable to create a buffer on the video card for vertex information\n");
    unregisterObject(object);

    return NO;
}

static BOOL registerTexture(texture_t * tex) {
    renderer_prog_texture_t * texData;

    if (!tex) {
        printf("[GLES2] Texture was null\n");
        return NO;
    }

    if (tex->renderer_data) {
        printf("[GLES2] Texture is already registered\n");
        return NO;
    }

    texData = malloc(sizeof(renderer_prog_texture_t));
    if (!texData) {
        printf("[GLES2] Unable to allocate memory for texture data\n");
        return NO;
    }

    tex->renderer_data = texData;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texData->texId);
    glBindTexture(GL_TEXTURE_2D, texData->texId);

    switch (tex->type) {
        case TEXTURE_TYPE_RGB:
        case TEXTURE_TYPE_BGR:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex->width, tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex->data);
            break;

        case TEXTURE_TYPE_RGBA:
        case TEXTURE_TYPE_BGRA:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
            break;

        default:
            printf("[GLES2] Unknown texture format \n");
            return NO;
            break;
    }

    /* Using mipMapping to reduce bandwidth consumption */
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return YES;
}

static void UseTexture(texture_t * tex) {
    renderer_prog_texture_t * texData;
    if (tex) {
        texData = (renderer_prog_texture_t *) tex->renderer_data;

        if (texData) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texData->texId);
            glUniform1i(CurrentShaderFind("s_baseMap"), 0);
        }
    }
}

static void setup3d(camera_t * camera) {
    vec3_t vLookat;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);

    vectorAdd(camera->position, camera->forward, vLookat);
    LookAt(camera->position, vLookat, camera->up, modelViewMatrix);
    Perspective(camera->fov, camera->aspect, camera->zNear, camera->zFar, projectionMatrix);
}

static void render(object_t * object, matrix_t mat) {
    matrix_t tmp;
    renderer_prog_object_t * objData;

    objData = object->renderer_data;

    if (!objData) {
        return;
    }

    UseShader(objData->shader);
    if (object->material) {
        if (object->material->texture_diffuse) {
            UseTexture(object->material->texture_diffuse);
        }
    }

    matrix_multiply(modelViewMatrix, mat, tmp);
    matrix_multiply(projectionMatrix, tmp, modelViewProjectionMatrix);
    currentShaderUniformMatrix("modelViewProjectionMatrix", 1, GL_FALSE, modelViewProjectionMatrix);

    if (objData->memory_location == OBJECT_MEMLOC_VRAM) {
        glBindBuffer(GL_ARRAY_BUFFER, objData->vertexVboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objData->indiciesVboId);

        currentShaderBindVertexAttribPointer("a_vertex", 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(VERTEX_OFFSET_OF_POSITION));
        currentShaderBindVertexAttribPointer("a_normal", 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(VERTEX_OFFSET_OF_NORMAL));
        currentShaderBindVertexAttribPointer("a_texcoord0", 2, GL_SHORT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(VERTEX_OFFSET_OF_TEXTURECOORD));
        //currentShaderBindVertexAttribPointer("a_weight0", 1, GL_FLOAT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(VERTEX_OFFSET_OF_WEIGHTS));
        //currentShaderBindVertexAttribPointer("a_boneId0", 1, GL_SHORT, GL_FALSE, sizeof(vertex_t), BUFFER_OFFSET(VERTEX_OFFSET_OF_BONEID));

        glDrawElements(GL_TRIANGLES, object->num_indices, GL_UNSIGNED_SHORT, 0);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        currentShaderBindVertexAttribPointer("a_vertex", 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &object->vertices[0].position[0]);
        currentShaderBindVertexAttribPointer("a_normal", 2, GL_FLOAT, GL_TRUE, sizeof(vertex_t), object->vertices->normal);
        currentShaderBindVertexAttribPointer("a_texcoord0", 2, GL_SHORT, GL_FALSE, sizeof(vertex_t), object->vertices->textureCoord);

        glDrawElements(GL_TRIANGLES, object->num_indices, GL_UNSIGNED_SHORT, object->indices);
    }
}

static void end3d() {
    glCheckErrors("end3D");
}

static void orthographic(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far, matrix_t mat) {
    mat[0] = 2.0f / (right - left);
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    mat[3] = 0.0f;

    mat[4] = 0.0f;
    mat[5] = 2.0f / (top - bottom);
    mat[6] = 0.0f;
    mat[7] = 0.0f;

    mat[8] = 0.0f;
    mat[9] = 0.0f;
    mat[10] = (-2.0f) / (far - near);
    mat[11] = 0.0f;

    mat[12] = -(right + left) / (right - left);
    mat[13] = -(top + bottom) / (top - bottom);
    mat[14] = -(far + near) / (far - near);
    mat[15] = 1.0f;
}

static void start2D(int renderWidth, int renderHeight) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    UseShader(textObjectShader);
    orthographic(0, 800, 0, 600, -1, 1, modelViewProjectionMatrix);
    currentShaderUniformMatrix("modelViewProjectionMatrix", 1, GL_FALSE, modelViewProjectionMatrix);
}

static void end2D() {
    glCheckErrors("end2D");
}

static void printString(int x, int y, font_t * font, const char * txt) {
    char * p = (char *) txt;
    float cx, cy;

    UseTexture(font->texture);

    unsigned short vertices[] = { 0, 0, 16, 0, 16, 16, 0, 16 };
    unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };

    while (*p) {
        cx = (float) (*p % 16) / 16.0f;
        cy = (float) (*p / 16) / 16.0f;

        vec2_t texcoord[] = { { cx, 1 - cy - 0.0625f }, { cx + 0.0625f, 1 - cy - 0.0625f }, { cx + 0.0625f, 1 - cy }, { cx, 1 - cy } };

        currentShaderBindVertexAttribPointer("a_vertex", 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, vertices);
        currentShaderBindVertexAttribPointer("a_texcoord0", 2, GL_FLOAT, GL_FALSE, 0, texcoord);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);

        vertices[0] += font->nCharWidth[(int) *p];
        vertices[2] += font->nCharWidth[(int) *p];
        vertices[4] += font->nCharWidth[(int) *p];
        vertices[6] += font->nCharWidth[(int) *p];

        p++;
    }
}

static BOOL shutdown() {
    FreeShader(defaultObjectShader);
    FreeShader(textObjectShader);

    return YES;
}

renderer_t * rendererInitProg() {
    renderer_t * renderer;
    renderer = malloc(sizeof(renderer_t));

    if (renderer) {
        renderer->name = "OpenGL Prog Path";

        renderer->init = &init;
        renderer->shutdown = &shutdown;
        renderer->register_object = &registerObject;
        renderer->unregister_object = &unregisterObject;
        renderer->register_texture = &registerTexture;

        renderer->start_3D = &setup3d;
        renderer->render_object = &render;
        renderer->end_3D = &end3d;

        renderer->start_2D = &start2D;
        renderer->printString = &printString;
        renderer->end_2D = &end2D;
    }

    return renderer;
}
