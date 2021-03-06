#include "AnimationController.h"
#include "BoneObject.h"
#include "../class/SkeletonAnimation.h"
#include "../class/SkeletonAnimationInstance.h"
#include "../Base/BaseObject.h"
#include "../Maths/DualQuaternion.h"
#include "../class/UniformData.h"
#include "../class/Mesh.h"
#include "../class/Timer.h"
#include "MeshRenderer.h"

DEFINITE_CLASS_RTTI(AnimationController, BaseComponent);

std::shared_ptr<AnimationController> AnimationController::Create(const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance)
{
	std::shared_ptr<AnimationController> pAnimationController = std::make_shared<AnimationController>();
	if (pAnimationController.get() && pAnimationController->Init(pAnimationController, pAnimationInstance))
		return pAnimationController;

	return nullptr;
}

bool AnimationController::Init(const std::shared_ptr<AnimationController>& pAnimationController, const std::shared_ptr<SkeletonAnimationInstance>& pAnimationInstance)
{
	if (!BaseComponent::Init(pAnimationController))
		return false;

	m_pAnimationInstance = pAnimationInstance;

	m_pCurrentAnimationIndices = new uint32_t[m_pAnimationInstance->GetAnimation()->m_animationDataDiction[0].objectAnimationDiction.size()];
	memset(m_pCurrentAnimationIndices, 0, sizeof(uint32_t) * m_pAnimationInstance->GetAnimation()->m_animationDataDiction[0].objectAnimationDiction.size());

	return true;
}

void AnimationController::Update()
{
	double elapsed = Timer::GetElapsedTime();
	m_animationPlayedTime += elapsed / 1000.0;
	m_animationPlayedTime = fmod(m_animationPlayedTime, m_pAnimationInstance->GetAnimation()->m_animationDataDiction[0].duration);
}

void AnimationController::UpdateBoneTransform(const std::shared_ptr<BaseObject>& pObject)
{
	std::shared_ptr<SkeletonAnimation> pAnimation = m_pAnimationInstance->GetAnimation();
	auto iter = pAnimation->m_animationDataDiction[0].objectAnimationLookupTable.find(pObject->GetNameHashCode());

	// If current object contains animation information, it's local transform will be changed accordingly
	if (iter != pAnimation->m_animationDataDiction[0].objectAnimationLookupTable.end())
	{
		uint32_t keyFrameCount = (uint32_t)pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames.size();
		uint32_t currentKeyFrameIndex = m_pCurrentAnimationIndices[iter->second];

		// Check if there is actually a next key frame
		if (m_pCurrentAnimationIndices[iter->second] + 1 < keyFrameCount
			// And if current keyframe is done, move to next possible animation
			&& m_animationPlayedTime > pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[m_pCurrentAnimationIndices[iter->second] + 1].time)
		{
			do
			{
				currentKeyFrameIndex += 1;
			} while (currentKeyFrameIndex < keyFrameCount
				&& m_animationPlayedTime > pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].time);

			currentKeyFrameIndex--;
			m_pCurrentAnimationIndices[iter->second] = currentKeyFrameIndex;
		}
		// If the whole animation is done, then start over again(FIXME: should add animation state and configuration later to configure this)
		else if (m_animationPlayedTime < pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[m_pCurrentAnimationIndices[iter->second]].time)
		{
			currentKeyFrameIndex = 0;

			do
			{
				currentKeyFrameIndex += 1;
			} while (m_animationPlayedTime > pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].time);

			currentKeyFrameIndex--;
			m_pCurrentAnimationIndices[iter->second] = currentKeyFrameIndex;
		}

		uint32_t nextKeyFrameIndex = currentKeyFrameIndex + 1;

		// To deal with the situation that some object's animation is shorter than the others, therefore it'll keep last key frame until current animation is done
		if (nextKeyFrameIndex >= keyFrameCount)
			nextKeyFrameIndex = currentKeyFrameIndex;

		double currentAnimationTime = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].time;
		double nextAnimationTime = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[nextKeyFrameIndex].time;
		double factor = (m_animationPlayedTime - currentAnimationTime) / (nextAnimationTime - currentAnimationTime);

		// To deal with the situation that some object's animation is shorter than the others, therefore it'll keep last key frame until current animation is done
		factor = factor > 1.0f ? 1.0f : factor;

		Quaterniond& currentRotation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[currentKeyFrameIndex].transform;
		Vector3d& currentTranslation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].translationKeyFrames[currentKeyFrameIndex].transform;

		Quaterniond& nextRotation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].rotationKeyFrames[nextKeyFrameIndex].transform;
		Vector3d& nextTranslation = pAnimation->m_animationDataDiction[0].objectAnimationDiction[iter->second].translationKeyFrames[nextKeyFrameIndex].transform;

		Quaterniond blendRotation = Quaterniond::SLerp(currentRotation, nextRotation, factor);
		Vector3d blendTranslation = currentTranslation * (1.0f - factor) + nextTranslation * factor;

		pObject->SetRotation(blendRotation);
		pObject->SetPos(blendTranslation);
	}
}

void AnimationController::OnPreRender()
{
	if (m_pMeshRenderer == nullptr)
		return;

	m_pMeshRenderer->SetUtilityIndex(m_pAnimationInstance->GetAnimationChunkIndex());
	m_pMeshRenderer->OverrideModelMatrix(GetBaseObject()->GetCachedWorldTransform());
}

void AnimationController::SyncBoneTransformToUniform(const std::shared_ptr<BaseObject>& pObject, uint32_t boneIndex, const DualQuaterniond& boneOffsetDQ)
{
	Matrix4d transform = GetBaseObject()->GetCachedWorldTransform();
	transform.Inverse();
	transform *= pObject->GetCachedWorldTransform();
	transform *= Matrix4d(boneOffsetDQ.AcquireRotation().Matrix(), boneOffsetDQ.AcquireTranslation());

	m_pAnimationInstance->SetBoneTransform(pObject->GetNameHashCode(), boneIndex, DualQuaterniond(transform.RotationMatrix(), transform.TranslationVector()));
}

void AnimationController::OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject)
{
	InitBoneObjects(GetBaseObject());
}

void AnimationController::InitBoneObjects(std::weak_ptr<BaseObject> pRootObject)
{
	if (pRootObject.expired())
		return;

	std::shared_ptr<BaseObject> pSharedRootObject = pRootObject.lock();
	DualQuaterniond boneOffsetDQ;
	uint32_t boneIndex;
	// Check if current object is a bone
	if (UniformData::GetInstance()->GetPerBoneIndirectUniforms()->GetBoneInfo(m_pAnimationInstance->GetMesh()->GetMeshBoneChunkIndexOffset(), pSharedRootObject->GetNameHashCode(), boneIndex, boneOffsetDQ))
	{
		std::shared_ptr<BoneObject> pBoneObject = BoneObject::Create(std::dynamic_pointer_cast<AnimationController>(GetSelfSharedPtr()), boneIndex, boneOffsetDQ);
		pSharedRootObject->AddComponent(pBoneObject);
	}

	for (uint32_t i = 0; i < pSharedRootObject->GetChildrenCount(); i++)
		InitBoneObjects(pSharedRootObject->GetChild(i));
}