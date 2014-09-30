//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform vec3 pos;
uniform float life;
uniform float elapsed;
uniform vec4 color;

uniform vec3 vel;

void main(void)
{
    // Render to positions texture
	gl_FragData[0] = vec4(pos, elapsed);

    // Render to velocities texture
    gl_FragData[1] = vec4(vel, life);

    // Render to color texture
    gl_FragData[2] = color;
}

