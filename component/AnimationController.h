#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../Maths/DualQuaternion.h"

class SkeletonAnimationInstance;
class MeshRenderer;

class AnimationController : public BaseComponent
{
	DECLARE_CLASS_RTTI(AnimationController);

public:
	static std::shared_ptr<AnimationController> Create(const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance = nullptr);
	~AnimationController() { delete[] m_pCurrentAnimationIndices; }

public:
	void UpdateBoneTransform(const std::shared_ptr<BaseObject>& pObject);
	void SyncBoneTransformToUniform(const std::shared_ptr<BaseObject>& pObject, uint32_t boneIndex, const DualQuaterniond& boneOffsetDQ);
	std::shared_ptr<SkeletonAnimationInstance> GetAnimationInstance() const { return m_pAnimationInstance; }
	double GetAnimationPlayedTime() const { return m_animationPlayedTime; }
	void SetMeshRenderer(const std::shared_ptr<MeshRenderer>& pMeshRenderer) { m_pMeshRenderer = pMeshRenderer; }

public:
	void Update() override;
	void OnPreRender() override;

protected:
	bool Init(const std::shared_ptr<AnimationController>& pAnimationController, const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance = nullptr);

protected:
	void OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject) override;
	void InitBoneObjects(std::weak_ptr<BaseObject> pRootObject);

protected:
	std::shared_ptr<SkeletonAnimationInstance>	m_pAnimationInstance;
	std::shared_ptr<MeshRenderer>				m_pMeshRenderer;

	double		m_animationPlayedTime;

	uint32_t*	m_pCurrentAnimationIndices;
};