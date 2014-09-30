#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;
varying vec4 texCoord;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main(void)
{
	gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
    if (texture2D( tex, texCoord.st ).rgb == vec3(1.0, 1.0, 1.0))
    	gl_FragColor.a      = 0.0;
    else
    	gl_FragColor.a      = 0.2;//0.33 * gl_FragColor.x + 0.33 * gl_FragColor.y + 0.33 *gl_FragColor.z;;
}