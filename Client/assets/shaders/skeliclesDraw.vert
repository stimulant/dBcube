#version 120

uniform sampler2D positions;
uniform sampler2D velocity;
uniform sampler2D joints;
uniform sampler2D offsets;
uniform sampler2D colors;

uniform float size;
uniform float lifeScale;
uniform float elapsed;
uniform float audioLevel;

uniform vec3 eyePos;
varying vec3 vEyeDir;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec4 vColor;
varying float vAlpha;
varying float bug;

// Gets a transformation matrix given the rotation angles
mat4x4 rotationMat(float rx, float ry, float rz)
{
  // Pre-computes trigonometric values (mainly for better readability)
  float cx = cos(rx);
  float sx = sin(rx);
  float cy = cos(ry);
  float sy = sin(ry);
  float cz = cos(rz);
  float sz = sin(rz);
 
  return mat4x4(cy*cz, (sx*sy*cz-cx*sz), (sx*sz+cx*sy*cz), 0,
                    cy*sz, (sx*sy*sz+cx*cz), (cx*sy*sz-sx*cz), 0,
                    -sy,   sx*cy,            cx*cy,            0,
                    0,     0,                0,                1);
}

void main()
{
	vec4 positionInfo = texture2D( positions, gl_MultiTexCoord0.xy );
	vec3 pos = positionInfo.xyz;
	float life = positionInfo.w;

	vec4 offsetInfo = texture2D( offsets, gl_MultiTexCoord0.xy );
	vec3 offset = offsetInfo.xyz;
	float scale = offsetInfo.w;

	vec4 color = texture2D( colors, gl_MultiTexCoord0.xy );

	// rotate
	float rotScale = 0.04;
	float rotation = (elapsed + life + scale * 1000) * rotScale;
	mat4x4 rot_mat = rotationMat(rotation, -rotation, rotation);

	// update
	vColor = color;
	vNormal	= normalize( gl_NormalMatrix * vec3(rot_mat * vec4( gl_Normal, 1.0 ) ) );
	vVertex	= gl_ModelViewProjectionMatrix * (rot_mat * vec4( gl_Vertex ) * vec4(vec3(1,1,1) * 8.0 * scale * audioLevel, 1) + vec4(pos, 0));
	vEyeDir	= normalize( eyePos - vVertex.xyz );

	gl_Position = vVertex;
}