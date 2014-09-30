uniform sampler2D displacementMap;
uniform sampler2D velocityMap;

varying vec4 color;

void main()
{
    //using the displacement map to move vertices
	vec3 pos = texture2D( displacementMap, gl_MultiTexCoord0.xy ).xyz;
	float life = texture2D( displacementMap, gl_MultiTexCoord0.xy ).w;
    //color.rg = texture2D( velocityMap, gl_MultiTexCoord0.xy ).rg;
	//color.a = life;

	color = vec4(1, 1, 0, 1);
	gl_PointSize = 10.0 * life;
	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1.0) ;
}