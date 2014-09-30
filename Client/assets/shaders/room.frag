uniform sampler2D colorMap;
uniform vec3 eyePos;
uniform float power;
uniform float lightPower;
uniform vec3 roomDims;
uniform float timePer;
uniform vec4 topColor;
uniform bool useRoomMap;
uniform float audioLevel;

varying vec3 eyeDir;
varying vec4 vVertex;
varying vec3 vNormal;
varying vec4 vColor;
varying vec2 vTexCoord0;

void main()
{
	vec4 roomColor = texture2D( colorMap, vTexCoord0 );
	float aoLight		= 1.0 - length( vVertex.xyz ) * ( 0.0015 + ( power * 0.015 ) );
	float ceiling		= 0.1;
	if( vNormal.y < -0.5 ) ceiling = 1.0;
	
	float yPer = clamp( vVertex.y/roomDims.y, 0.0, 1.0 );
	float ceilingGlow	= pow( yPer, 2.0 ) * 0.5;
	ceilingGlow			+= pow( yPer, 200.0 );
	ceilingGlow			+= pow( max( yPer - 0.7, 0.0 ), 3.0 ) * 4.0;
	ceilingGlow = ceilingGlow * audioLevel;
	
	vec3 litRoomColor	= vec3( aoLight );
	if (useRoomMap && ceiling != 1.0)
		litRoomColor = roomColor.rgb * audioLevel;
	gl_FragColor.rgb = litRoomColor + ( ceiling + ceilingGlow * timePer ) * lightPower * topColor.rgb;
	gl_FragColor.a		= 1.0;
}