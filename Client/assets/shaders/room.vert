uniform vec3 eyePos;
uniform mat4 mMatrix;
uniform mat4 mvpMatrix;
//uniform mat4 fViewport;
uniform float audioLevel;

varying vec3 eyeDir;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec2 vTexCoord0;

void main()
{
	vVertex			= mMatrix * vec4( gl_Vertex );
	vNormal			= normalize( vec3( mMatrix * vec4( gl_Normal, 0.0 ) ) );
	eyeDir			= normalize( eyePos - vVertex.xyz );
	vTexCoord0 = gl_MultiTexCoord0.xy;
	
	float level = (audioLevel/200.0);
	gl_Position		= mvpMatrix * vVertex;// * vec4(level, level, level, 1.0);
	gl_TexCoord[0]	= gl_MultiTexCoord0;
}