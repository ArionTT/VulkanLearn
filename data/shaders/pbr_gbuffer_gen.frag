#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inCSNormal;
layout (location = 2) in vec3 inCSTangent;
layout (location = 3) in vec3 inCSBitangent;
layout (location = 4) flat in int perMaterialIndex;
layout (location = 5) flat in int perObjectIndex;
layout (location = 6) in vec3 inCSPosition;
layout (location = 7) noperspective in vec2 inScreenPosition;
layout (location = 8) in vec3 inPrevCSPosition;

layout (location = 0) out vec4 outGBuffer0;
layout (location = 1) out vec4 outGBuffer1;
layout (location = 2) out vec4 outGBuffer2;
layout (location = 3) out vec4 outMotionVec;

#include "uniform_layout.sh"

struct PBRTextures
{
	vec4 albedoRougness;
	vec2 AOMetalic;

	float albedoRoughnessIndex;
	float normalAOIndex;
	float metallicIndex;
};

layout(set = 3, binding = 0) buffer MaterialUniforms
{
	PBRTextures textures[];
};

void main() 
{
	float metalic = textures[perMaterialIndex].AOMetalic.g;
	if (textures[perMaterialIndex].metallicIndex >= 0)
		metalic *= texture(R8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].metallicIndex), 0.0).r * textures[perMaterialIndex].AOMetalic.g;

	vec4 normal_ao = vec4(vec3(0), textures[perMaterialIndex].AOMetalic.x);
	if (textures[perMaterialIndex].normalAOIndex < 0)
	{
		normal_ao.xyz = normalize(inCSNormal);
	}
	else
	{
		normal_ao = texture(RGBA8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].normalAOIndex), 0.0);

		vec3 n = normalize(normal_ao.xyz * 2.0 - 1.0);
		mat3 TBN = mat3(normalize(inCSTangent), normalize(inCSBitangent), normalize(inCSNormal));
		n = TBN * n;

		normal_ao.xyz = n;
		normal_ao.w *= textures[perMaterialIndex].AOMetalic.x;
	}

	vec4 albedo_roughness = textures[perMaterialIndex].albedoRougness;
	if (textures[perMaterialIndex].albedoRoughnessIndex >= 0)
		albedo_roughness *= texture(RGBA8_1024_MIP_2DARRAY, vec3(inUv.st, textures[perMaterialIndex].albedoRoughnessIndex), 0.0);

	outGBuffer0.xyz = normal_ao.xyz * 0.5f + 0.5f;
	outGBuffer0.w = albedo_roughness.w;

	outGBuffer1 = vec4(albedo_roughness.rgb, 0);
	outGBuffer2 = vec4(vec3(albedo_roughness.w, metalic, 0), normal_ao.a);

	vec4 prevNDCPos = globalData.projection * vec4(inPrevCSPosition, 1.0f);
	vec2 prevTexCoord = (prevNDCPos.xy / prevNDCPos.w);

	outMotionVec.rg = prevTexCoord - inScreenPosition;
	outMotionVec.rg = outMotionVec.rg * 0.5f;

	// Calculate circle of confusion
	outGBuffer1.a = CalculateCoC(-inCSPosition.z);
}
