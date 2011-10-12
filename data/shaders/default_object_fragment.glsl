varying vec2 v_texcoord;
varying vec3 v_normal;

uniform sampler2D s_baseMap;

void main(void)
{
    vec4 colorSample  = texture2D(s_baseMap, v_texcoord);
    gl_FragColor = vec4(colorSample.r, 1.0, 1.0, 1.0);
}
