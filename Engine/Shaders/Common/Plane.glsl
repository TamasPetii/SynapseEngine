#ifndef COMMON_PLANE_GLSL
#define COMMON_PLANE_GLSL

vec4 positions[4] = vec4[4](
	vec4(-1,-1, 0, 1),
	vec4( 1,-1, 0, 1),
	vec4(-1, 1, 0, 1),
	vec4( 1, 1, 0, 1)
);

vec2 texCoords[4] = vec2[4](
	vec2(0,0),
	vec2(1,0),
	vec2(0,1),
	vec2(1,1)
);

#endif