#version 120

uniform sampler2D displacementMap;
uniform sampler2D velocityMap;
uniform sampler2D colorMap;
uniform float size;
uniform float lifeScale;

uniform vec3 eyePos;
varying vec3 vEyeDir;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec4 vColor;
varying float vAlpha;

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
	vec3 translate = texture2D( displacementMap, gl_MultiTexCoord0.xy ).xyz;
	float elapsed = texture2D( displacementMap, gl_MultiTexCoord0.xy ).w;
	float life = texture2D( velocityMap, gl_MultiTexCoord0.xy ).w;
	vColor = texture2D( colorMap, gl_MultiTexCoord0.xy );

	float rotScale = 0.05;
	mat4x4 rot_mat = rotationMat(elapsed + life * rotScale, elapsed + life * rotScale, 0.0);

	vNormal	= normalize( gl_NormalMatrix * vec3(rot_mat * vec4( gl_Normal, 1.0 ) ) );

	vAlpha = max(1.0 - life*0.002*lifeScale, 0);
	vVertex	= gl_ModelViewProjectionMatrix * (rot_mat * vec4( gl_Vertex ) * vec4(vec3(1,1,1) * size * vAlpha,1) + vec4(translate, 0));
	vEyeDir	= normalize( eyePos - vVertex.xyz );

	gl_Position = vVertex;
}