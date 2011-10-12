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
   v_texcoord = a_texcoord0.xy;
   gl_Position = modelViewProjectionMatrix * vec4(a_vertex, 1.0);
   //gl_Position = vec4(a_vertex.x, a_vertex.y, a_vertex.z - 50.0, 1.0);
}
