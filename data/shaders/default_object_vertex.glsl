uniform mat4 modelViewProjectionMatrix;

attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_texcoord0;
//attribute vec4 a_weight0;
//attribute vec4 a_boneId0;

varying vec2 v_texcoord;
varying vec3 v_normal;

void main(void)
{
   v_texcoord = vec2(a_texcoord0.x / 32767.0, a_texcoord0.y / 32767.0);
   gl_Position = modelViewProjectionMatrix * vec4(a_vertex, 1.0);
}
