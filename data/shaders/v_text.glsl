#ifdef GL_ES
precision highp float;
#endif

uniform mat4 modelViewProjectionMatrix;

attribute vec3 a_vertex;
attribute vec2 a_texcoord0;

varying vec2 v_texcoord;

void main(void) {
   // Transform output position
   gl_Position = modelViewProjectionMatrix * vec4(a_vertex.x, a_vertex.y, a_vertex.z, 1.0);

   // Pass through texture coordinate
   v_texcoord = a_texcoord0.xy;
}
