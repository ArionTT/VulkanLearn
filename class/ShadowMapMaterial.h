#pragma once
#include "Material.h"

class ShadowMapMaterial : public Material
{
public:
	static std::shared_ptr<ShadowMapMaterial> CreateDefaultMaterial(bool skinned = false);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override
	{
		DrawIndirect(pCmdBuf, pFrameBuffer, pingpong, overrideVP);
	}
};