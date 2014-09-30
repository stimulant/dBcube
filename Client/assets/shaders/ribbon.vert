uniform mat4 mvpMatrix;
uniform vec3 eyePos;
uniform mat4 matrix;

varying vec3 vEyeDir;
varying vec4 vVertex;
varying vec3 vNormal;

void main()
{
	vNormal			= normalize( gl_NormalMatrix * vec3( matrix * vec4( gl_Normal, 0.0 ) ) );
	vVertex			= matrix * vec4( gl_Vertex );
	vEyeDir			= normalize( eyePos - vVertex.xyz );
	
	gl_Position		= mvpMatrix * vVertex;
	gl_FrontColor 	= gl_Color;
	gl_TexCoord[0]	= gl_MultiTexCoord0;
}