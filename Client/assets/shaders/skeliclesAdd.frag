//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform vec3 pos;
uniform vec3 vel;

uniform float skeletonIdx;
uniform float jointType1;
uniform float jointType2;
uniform float jointLerp;

uniform vec3 offset;
uniform float rotation;
uniform float scale;

uniform float life;
uniform float elapsed;
uniform vec4 color;

void main(void)
{
	// positions
	gl_FragData[0] = vec4( pos, life );

	// veloctiy
	gl_FragData[1] = vec4( pos, life );

    // joints
	gl_FragData[2] = vec4( skeletonIdx, jointType1, jointType2, jointLerp );

    // offsets
    gl_FragData[3] = vec4(offset.x, offset.y, offset.z, scale);

    // colors
    gl_FragData[4] = color;
}

