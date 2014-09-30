#version 110
varying float depth;

void main()
{
    float d = depth * 0.1;
    if( d > 0.28) discard;

    gl_FragColor.rgb    = 1.0 - vec3( d );
    gl_FragColor.a      = 1.0;
}

