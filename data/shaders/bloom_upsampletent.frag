#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"
#include "global_parameters.sh"
#include "utilities.sh"

layout (set = 3, binding = 3) uniform sampler2D PrevBloomUpsample[3];

layout(push_constant) uniform PushConsts {
	layout (offset = 0) vec2 texelSize;
} pushConsts;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outBloomFrag;

void main() 
{
	outBloomFrag = vec4(UpsampleTent(PrevBloomUpsample[frameIndex], inUv, pushConsts.texelSize, vec4(globalData.BloomSettings0.z)).rgb, 1.0f);
}