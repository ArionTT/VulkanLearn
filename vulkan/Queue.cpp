#include "Queue.h"
#include "CommandBuffer.h"
#include "Semaphore.h"
#include "Fence.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"
#include "FrameManager.h"

Queue::~Queue()
{
}

bool Queue::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Queue>& pSelf, uint32_t queueFamilyIndex)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	// We only acquire 1st queue in a queue family, for now
	vkGetDeviceQueue(pDevice->GetDeviceHandle(), queueFamilyIndex, 0, &m_queue);
	return true;
}

std::shared_ptr<Queue> Queue::Create(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex)
{
	std::shared_ptr<Queue> pQueue = std::make_shared<Queue>();
	if (pQueue.get() && pQueue->Init(pDevice, pQueue, queueFamilyIndex))
		return pQueue;
	return nullptr;
}

void Queue::SubmitPerFrameCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuffer, bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, FrameMgr()->GetCurrentFrameFence(), waitUtilQueueIdle);
}

void Queue::SubmitPerFrameCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers, bool waitUtilQueueIdle)
{
	SubmitCommandBuffers(cmdBuffers, std::vector<std::shared_ptr<Semaphore>>(), std::vector<VkPipelineStageFlags>(), FrameMgr()->GetCurrentFrameFence(), waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuffer, const std::shared_ptr<Fence>& pFence, bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, pFence, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers, const std::shared_ptr<Fence>& pFence, bool waitUtilQueueIdle)
{
	SubmitCommandBuffers(cmdBuffers, std::vector<std::shared_ptr<Semaphore>>(), std::vector<VkPipelineStageFlags>(), pFence, waitUtilQueueIdle);
}

void Queue::SubmitPerFrameCommandBuffer(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitPerFrameCommandBuffers(v, waitSemaphores, waitStages, waitUtilQueueIdle);
}

void Queue::SubmitPerFrameCommandBuffers(
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle)
{
	SubmitPerFrameCommandBuffers(cmdBuffers, std::vector<std::shared_ptr<Semaphore>>(), std::vector<VkPipelineStageFlags>(), std::vector<std::shared_ptr<Semaphore>>(), waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffer(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::shared_ptr<Fence>& pFence,
	bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, waitSemaphores, waitStages, pFence, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffers(
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::shared_ptr<Fence>& pFence,
	bool waitUtilQueueIdle)
{
	SubmitCommandBuffers(cmdBuffers, std::vector<std::shared_ptr<Semaphore>>(), std::vector<VkPipelineStageFlags>(), std::vector<std::shared_ptr<Semaphore>>(), pFence, waitUtilQueueIdle);
}

void Queue::SubmitPerFrameCommandBuffer(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitPerFrameCommandBuffers(v, waitSemaphores, waitStages, signalSemaphores, waitUtilQueueIdle);
}

void Queue::SubmitPerFrameCommandBuffers(
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
	FrameMgr()->CacheSubmissioninfo(GetSelfSharedPtr(), cmdBuffers, waitSemaphores, waitStages, signalSemaphores, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffer(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	const std::shared_ptr<Fence>& pFence,
	bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, waitSemaphores, waitStages, signalSemaphores, pFence, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffers(
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	const std::shared_ptr<Fence>& pFence,
	bool waitUtilQueueIdle)
{
	std::vector<VkCommandBuffer> deviceCmdBuffers;
	for (uint32_t i = 0; i < cmdBuffers.size(); i++)
		deviceCmdBuffers.push_back(cmdBuffers[i]->GetDeviceHandle());

	std::vector<VkSemaphore> deviceWaitSemaphores;
	for (uint32_t i = 0; i < waitSemaphores.size(); i++)
		deviceWaitSemaphores.push_back(waitSemaphores[i]->GetDeviceHandle());

	std::vector<VkSemaphore> deviceSignalSemaphores;
	for (uint32_t i = 0; i < signalSemaphores.size(); i++)
		deviceSignalSemaphores.push_back(signalSemaphores[i]->GetDeviceHandle());

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount	= (uint32_t)deviceCmdBuffers.size();
	submitInfo.pCommandBuffers		= deviceCmdBuffers.data();
	submitInfo.waitSemaphoreCount	= (uint32_t)deviceWaitSemaphores.size();
	submitInfo.pWaitSemaphores		= deviceWaitSemaphores.data();
	submitInfo.pWaitDstStageMask	= waitStages.data();
	submitInfo.signalSemaphoreCount = (uint32_t)deviceSignalSemaphores.size();
	submitInfo.pSignalSemaphores	= deviceSignalSemaphores.data();

	VkFence fence = 0;
	if (pFence.get())
		fence = pFence->GetDeviceHandle();

	vkQueueSubmit(GetDeviceHandle(), 1, &submitInfo, fence);

	if (waitUtilQueueIdle)
	{
		CHECK_VK_ERROR(vkQueueWaitIdle(GetDeviceHandle()));
	}
}