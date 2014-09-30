varying vec4 vVertex;
varying float depth;
uniform float time;
uniform vec3 ballPositions[100];
uniform int ballCount;
uniform float ballRadius;
uniform float nearPlane;
uniform float farPlane;
uniform int volumeType;
uniform float volumeNoiseAmountScale;
uniform float volumeNoiseTimeScale;

vec4 mod289(vec4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
 
vec4 permute(vec4 x)
{
    return mod289(((x*34.0)+1.0)*x);
}
 
vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}
 
vec2 fade(vec2 t) {
    return t*t*t*(t*(t*6.0-15.0)+10.0);
}
 
// Classic Perlin noise
float cnoise(vec2 P)
{
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod289(Pi); // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
     
    vec4 i = permute(permute(ix) + iy);
     
    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
     
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
     
    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;  
    g01 *= norm.y;  
    g10 *= norm.z;  
    g11 *= norm.w;  
     
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
     
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}
 
// Classic Perlin noise, periodic variant
float pnoise(vec2 P, vec2 rep)
{
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, rep.xyxy); // To create noise with explicit period
    Pi = mod289(Pi);        // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
     
    vec4 i = permute(permute(ix) + iy);
     
    vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
    vec4 gy = abs(gx) - 0.5 ;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
     
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
     
    vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
    g00 *= norm.x;  
    g01 *= norm.y;  
    g10 *= norm.z;  
    g11 *= norm.w;  
     
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
     
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}
 
float fbm(vec2 P, int octaves, float lacunarity, float gain)
{
    float sum = 0.0;
    float amp = 1.0;
    vec2 pp = P;
     
    int i;
     
    for(i = 0; i < octaves; i+=1)
    {
        amp *= gain; 
        sum += amp * cnoise(pp);
        pp *= lacunarity;
    }
    return sum;
}

void main()
{
	vec3 outerPos;
	vec3 innerPos;
	vVertex				= vec4( gl_Vertex );

	// new noise
	float l = 2.3;
    float g = 0.4;
    int oc = 5;
	float scale = 0.05;
	float noise = fbm( vVertex.z * scale + vec2(time,time) * volumeNoiseTimeScale, oc, l, g);

	float outerZ = nearPlane;
	float innerZ = farPlane;

	if (volumeType == 0) // square
	{
		if (vVertex.x <= 0.25)							// top
		{
			outerPos = vec3((vVertex.x * 4.0) - 0.5, 0.5, outerZ);
			innerPos = vec3((vVertex.x * 4.0) - 0.5, 0.5, innerZ);		
		}
		else if (vVertex.x > 0.25 && vVertex.x <= 0.5)		// right
		{
			outerPos = vec3(0.5, (vVertex.x - 0.25) * 4.0 - 0.5, outerZ);
			innerPos = vec3(0.5, (vVertex.x - 0.25) * 4.0 - 0.5, innerZ);
		}
		else if (vVertex.x > 0.5 && vVertex.x <= 0.75)		// bottom
		{
			outerPos = vec3((vVertex.x - 0.5) * 4.0 - 0.5, -0.5, outerZ);
			innerPos = vec3((vVertex.x - 0.5) * 4.0 - 0.5, -0.5, innerZ);
		}
		else if (vVertex.x > 0.75 && vVertex.x <= 1.0)		// left
		{
			outerPos = vec3(-0.5, (vVertex.x - 0.75) * 4.0 - 0.5, outerZ);
			innerPos = vec3(-0.5, (vVertex.x - 0.75) * 4.0 - 0.5, innerZ);
		}
	}
	else if (volumeType == 1) // circle
	{
		float radius = 1.0;
		float t = vVertex.x * 6.28318531;
		float x = radius*cos(t);
		float y = radius*sin(t);

		outerPos = vec3(x, y, outerZ);
		innerPos = vec3(x, y, innerZ);		
	}
	else if (volumeType == 2) // triangle
	{
		vec2 triPos;
		if (vVertex.x <= 0.3333)							// left
		{
			vec2 a = vec2(-0.5, -0.5);
			vec2 b = vec2(0.0, 0.5);
			 triPos = mix(a, b, vVertex.x / 0.3333);
		}
		else if (vVertex.x <= 0.6666)							// left
		{
			vec2 a = vec2(0.0, 0.5);
			vec2 b = vec2(0.5, -0.5);
			triPos = mix(a, b, (vVertex.x - 0.3333) / 0.3333);
		}
		else	// bottom
		{
			vec2 a = vec2(-0.5, -0.5);
			vec2 b = vec2(0.5, -0.5);
			triPos = mix(a, b, (vVertex.x - 0.6666) / 0.3333);
		}
		outerPos = vec3(triPos.x, triPos.y, outerZ);
		innerPos = vec3(triPos.x, triPos.y, innerZ);
	}

	vVertex.xyz = mix(outerPos, innerPos, vVertex.y);
	vVertex.y = vVertex.y - fbm( vVertex.z + vec2(time,time) * 1.0, oc, l, g) * volumeNoiseAmountScale;

    for (int i=0; i<ballCount; i++)
    {
        float limit = 0.25;
        vec3 ballVec = vVertex.xyz - ballPositions[i];
        float ballDist = length(ballVec);
        if (ballDist < limit)
            vVertex.xyz = vVertex.xyz + ballVec * (limit - ballDist) * 5.0;
    }

	depth = (outerZ - vVertex.z)/4.0;
	
	gl_Position			= gl_ModelViewProjectionMatrix * vVertex;
}