//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

uniform float time;
uniform float elapsed;
uniform vec2 attractorPos;
uniform vec3 roomDims;
uniform bool bounce;
uniform float noiseAmount;

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D colors;

varying vec4 texCoord;

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


void main(void)
{
	// position, lie and mass
    vec3 p0 = texture2D( positions, texCoord.st).rgb;
	float elap = texture2D( positions, texCoord.st ).a;
	
	// velocity
    vec3 v0 = texture2D( velocities, texCoord.st).rgb;
    float life = texture2D( velocities, texCoord.st ).a;
    vec3 v1 = v0;

    // color
    vec4 color = texture2D( colors, texCoord.st );

	// new noise
	float l = 2.3;
    float g = 0.4;
    int oc = 4;
	float scale = 0.005;
    float noise = fbm( p0.xy * scale + vec2(time,time) * 10.0, oc, l, g);
	float angle = noise * 15.0;
	vec3 noiseVector = vec3( cos( angle ), sin( angle ), cos( angle ) );
	v1 = v1 + noiseVector * 1.1 * noiseAmount * max(( 1.0 - life ), 0.0);

	v1 = v1 * 1.0;											//decay
    //v1.y = v1.y - 0.1;
    vec3 p1	= p0 + v1;										//(symplectic euler) position update

    // bounce
    if (bounce)
    {
        if (p1.y > roomDims.y)
        {
            p1.y = roomDims.y;
            v1.y = -abs(v1.y);
        }
        if (p1.y < -roomDims.y)
        {
            p1.y = -roomDims.y;
            v1.y = abs(v1.y);
        }
        if (p1.x > roomDims.x)
        {
            p1.x = roomDims.x;
            v1.x = -abs(v1.x);
        }
        if (p1.x < -roomDims.x)
        {
            p1.x = -roomDims.x;
            v1.x = abs(v1.x);
        }
    }

	// update life
	life = life + elapsed * 1.0;
    
    // Render to positions texture
    gl_FragData[0] = vec4(p1, elap);

    // Render to velocities texture
    gl_FragData[1] = vec4(v1, life);

    // Render to color texture
    gl_FragData[2] = color;
} 