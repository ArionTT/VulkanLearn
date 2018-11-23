#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (set = 3, binding = 2) uniform sampler2D CombineResult[3];
layout (set = 3, binding = 3) uniform sampler2D MotionNeighborMax[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outScreen;

int index = int(perFrameData.camDir.a);

const float MOTION_VEC_AMP = 10.0f;
const float MOTION_VEC_SAMPLE_COUNT = 16;

void main() 
{
	vec3 noneMotionColor = texture(CombineResult[index], inUv).rgb;

	vec3 fullMotionColor = vec3(0);
	vec2 motionNeighborMax = texture(MotionNeighborMax[index], inUv).rg;
	vec2 step = motionNeighborMax / MOTION_VEC_SAMPLE_COUNT * MOTION_VEC_AMP;	// either side samples a pre-defined amount of colors
	vec2 startPos = inUv + motionNeighborMax + step * 0.5f * PDsrand(inUv + vec2(perFrameData.time.x));	// Randomize starting position

	for (int i = int(-MOTION_VEC_SAMPLE_COUNT / 2.0f); i <= int(MOTION_VEC_SAMPLE_COUNT / 2.0f); i++)
	{
		fullMotionColor += texture(CombineResult[index], startPos + step * i).rgb;
	}

	fullMotionColor /= MOTION_VEC_SAMPLE_COUNT;

	const float noneMotion = 2.0f;
	const float fullMotion = 15.0f;
	const float span = fullMotion - noneMotion;
	
	float motionMag = length(motionNeighborMax * globalData.gameWindowSize.xy) * MOTION_VEC_AMP;
	float motionMix = clamp(motionMag - noneMotion, 0.0f, span) / span;
	vec3 final = mix(noneMotionColor, fullMotionColor, motionMix);

	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outScreen = vec4(final, 1.0);
}