#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string.h>

int test_phys_device(VkInstance inst, VkPhysicalDevice pd, bool silent)
{
	VkPhysicalDeviceProperties2 pdProps2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
	VkPhysicalDeviceProperties &pdProps = pdProps2.properties;
	VkPhysicalDeviceDriverProperties driverProps{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES};
	pdProps2.pNext = &driverProps;
	vkGetPhysicalDeviceProperties2(pd, &pdProps2);
	printf("  props: %s type %d %d.%d.%d. driver %d.%d.%d\n", pdProps.deviceName, pdProps.deviceType, VK_VERSION_MAJOR(pdProps.apiVersion), VK_VERSION_MINOR(pdProps.apiVersion), VK_VERSION_PATCH(pdProps.apiVersion), VK_VERSION_MAJOR(pdProps.driverVersion), VK_VERSION_MINOR(pdProps.driverVersion), VK_VERSION_PATCH(pdProps.driverVersion));
	printf("  driver id %d name \"%s\" info \"%s\" conf ver %d.%d.%d.%d\n",driverProps.driverID, driverProps.driverName, driverProps.driverInfo, driverProps.conformanceVersion.major, driverProps.conformanceVersion.minor, driverProps.conformanceVersion.subminor, driverProps.conformanceVersion.patch);
	printf("  limits: maxColorAttachments %d\n", pdProps.limits.maxColorAttachments);
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(pd,nullptr,&count,nullptr);
	printf("  extensions %d\n", count);
	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateDeviceExtensionProperties(pd,nullptr,&count,extensions.data());
	for (int ei = 0; ei < (int)extensions.size(); ei++)
	{
		if (silent) break;
		printf("   %3.1d: %s:%d\n", ei, extensions[ei].extensionName, extensions[ei].specVersion);
	}

	count = 0;
	vkEnumerateDeviceLayerProperties(pd, &count, nullptr);
	printf("  device layers %d\n", count);	
	std::vector<VkLayerProperties> layers(count);
	vkEnumerateDeviceLayerProperties(pd, &count, layers.data());
	for (int li = 0; li < (int)layers.size(); li++)
	{
		uint32_t leCount = 0;
		vkEnumerateDeviceExtensionProperties(pd, layers[li].layerName, &leCount, nullptr);
		printf("   layer %d: %s device extensions %d\n", li, layers[li].layerName, leCount); 
	}

	VkPhysicalDeviceFeatures pdFeats;
	vkGetPhysicalDeviceFeatures(pd, &pdFeats);
	printf("  features: geomShader %d tessShader %d\n", pdFeats.geometryShader, pdFeats.tessellationShader);
	
	VkPhysicalDeviceMemoryProperties pdMem;
	vkGetPhysicalDeviceMemoryProperties(pd, &pdMem);
	printf("  memory: types %d, heaps %d\n", pdMem.memoryTypeCount, pdMem.memoryHeapCount);
	for (uint32_t mti = 0; mti < pdMem.memoryTypeCount; mti++)
		printf("   type %u: flags %.4X heap %d\n", mti, pdMem.memoryTypes[mti].propertyFlags, pdMem.memoryTypes[mti].heapIndex);
	for (uint32_t mhi = 0; mhi < pdMem.memoryHeapCount; mhi++)
		printf("   heap %u: size %lu flags %.4X\n", mhi, pdMem.memoryHeaps[mhi].size, pdMem.memoryHeaps[mhi].flags);

	count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, nullptr);
	printf("  queue families %u\n", count);
	std::vector<VkQueueFamilyProperties> qfp(count);
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, qfp.data());
	for (int qfi = 0; qfi < (int)qfp.size(); qfi++)
	{
		auto &tg = qfp[qfi].minImageTransferGranularity;
		printf("   %d: flags %.4X count %d timestampValidBits %d minImageTransferGranularity(%d,%d,%d)\n", qfi, qfp[qfi].queueFlags, qfp[qfi].queueCount, qfp[qfi].timestampValidBits, tg.width, tg.height, tg.depth);
	}

	float queuePriority = 1;
	VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &queuePriority};
	VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,nullptr,0,1,&queueInfo,0,nullptr,0,nullptr,nullptr };
	VkDevice device = 0;
	VkResult r = vkCreateDevice(pd, &deviceInfo, nullptr, &device);
	printf("%d vkCreateDevice %p\n", r, device);
	VkQueue queue = 0;
	vkGetDeviceQueue(device, 0, 0, &queue);
	printf("vkGetDeviceQueue %p\n", queue);

	if (!device)
		return 1;
	
	VkCommandPool commandPool = 0;
	VkCommandPoolCreateInfo cpcInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 0};
	r = vkCreateCommandPool(device, &cpcInfo, nullptr, &commandPool);
	printf("%d vkCreateCommandPool %p\n", r, commandPool);

	VkCommandBuffer cb = 0;
	VkCommandBufferAllocateInfo cbaInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
	r = vkAllocateCommandBuffers(device, &cbaInfo, &cb);
	printf("%d vkAllocateCommandBuffers %p\n", r, cb);

	VkExtent2D extent{16, 16};
	VkFormat imgFmt = VK_FORMAT_R8G8B8A8_UNORM;

	VkRenderPass renderPass = 0;
	VkAttachmentDescription attachment{0, imgFmt, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL};
	VkAttachmentReference colAttRef{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
	VkSubpassDescription subpass{0,VK_PIPELINE_BIND_POINT_GRAPHICS,0,nullptr,1,&colAttRef,nullptr,nullptr,0,nullptr};
	VkRenderPassCreateInfo rpcInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr};
	r = vkCreateRenderPass(device, &rpcInfo, nullptr, &renderPass);
	printf("%d vkCreateRenderPass %p\n", r, renderPass);

	VkImage image = 0;
	VkImageCreateInfo icInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0,
		VK_IMAGE_TYPE_2D, imgFmt, {extent.width, extent.height, 1}, 1, 1,
		VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE,
		0, nullptr, VK_IMAGE_LAYOUT_PREINITIALIZED};
	r = vkCreateImage(device, &icInfo, nullptr, &image);
	printf("%d vkCreateImage %p\n", r, image);
	VkMemoryRequirements memReq{};
	vkGetImageMemoryRequirements(device, image, &memReq);
	printf("vkGetImageMemoryRequirements size %ld align %ld typeBits %X\n", memReq.size, memReq.alignment, memReq.memoryTypeBits);
	int memoryType = -1;
	for (uint32_t mti = 0; mti < pdMem.memoryTypeCount; mti++)
	{
		if (!(memReq.memoryTypeBits & (1 << mti)))
			continue;
		const uint32_t mask = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		if ((pdMem.memoryTypes[mti].propertyFlags & mask) == mask)
			 memoryType = mti;
	}
	printf("selected memory type %d\n", memoryType);
	if (memoryType == -1)
		return -1;
	VkMemoryAllocateInfo maInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, memReq.size, (uint32_t)memoryType};
	VkDeviceMemory imgMem = 0;
	r = vkAllocateMemory(device, &maInfo, nullptr, &imgMem);
	printf("%d vkAllocateMemory %p\n", r, imgMem);
	void *imgData = nullptr;
	r = vkMapMemory(device, imgMem, 0, memReq.size, 0, &imgData);
	printf("%d vkMapMemory %p\n", r, imgData);
	if (imgData)
	{
		memset(imgData, 0x01, memReq.size);
		vkUnmapMemory(device, imgMem);
		printf("vkUnmapMemory\n");
		imgData = nullptr;
	}
	r = vkBindImageMemory(device, image, imgMem, 0);
	printf("%d vkBindImageMemory\n", r);

	VkImageView imageView = 0;
	VkImageViewCreateInfo ivcInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, image, VK_IMAGE_VIEW_TYPE_2D, imgFmt, {},
		{VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}};
	r = vkCreateImageView(device, &ivcInfo, nullptr, &imageView);
	printf("%d vkCreateImageView %p\n", r, imageView);

	VkFramebuffer framebuffer = 0;
	VkFramebufferCreateInfo fbcInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, renderPass, 1, &imageView, extent.width, extent.height, 1};
	r = vkCreateFramebuffer(device, &fbcInfo, nullptr, &framebuffer);
	printf("%d vkCreateFrameBuffer %p\n", r, framebuffer);

	VkCommandBufferBeginInfo cbbInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, 0};//why flag?
	r = vkBeginCommandBuffer(cb, &cbbInfo);
	printf("%d vkBeginCommandBuffer\n", r);
	{
		VkClearValue clearValue{ {0.2f,0.3f,0.7f,1.0f} };
		VkRenderPassBeginInfo rpbInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, renderPass, framebuffer, {{0,0},extent}, 1, &clearValue};
		vkCmdBeginRenderPass(cb, &rpbInfo, VK_SUBPASS_CONTENTS_INLINE);
		printf("vkCmdBeginRenderPass\n");
		
		VkClearAttachment ca{VK_IMAGE_ASPECT_COLOR_BIT, 0,{{0.f,0.f,0.f,0.f}}};
		VkClearRect cr{{{4,4},{8,8}},0,1};
		vkCmdClearAttachments(cb, 1, &ca, 1, &cr);
		printf("vkCmdClearAttachments\n");
		
		vkCmdEndRenderPass(cb);
		printf("vkCmdEndRenderPass\n");
	}
	r = vkEndCommandBuffer(cb);
	printf("%d vkEndCommandBuffer\n", r);

	VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cb, 0, nullptr};
	r = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	printf("%d vkQueueSubmit\n", r);
	r = vkDeviceWaitIdle(device);
	printf("%d vkDeviceWaitIdle\n", r);

	r = vkMapMemory(device, imgMem, 0, memReq.size, 0, &imgData);
	printf("%d vkMapMemory %p\n", r, imgData);
	if (imgData)
	{
		uint8_t *p = ((uint8_t*)imgData);
		for (int j = 0; j < 16; j++)
		{
			for (int i = 0; i < 16 * 4; i++)
				printf("%.2X", *(p++));
			printf("\n");
		}
		vkUnmapMemory(device, imgMem);
		printf("vkUnmapMemory\n");
	}

	vkFreeCommandBuffers(device, commandPool, 1, &cb);
	printf("vkFreeCommandBuffers\n");

	vkDestroyFramebuffer(device, framebuffer, nullptr);
	printf("vkDestroyFramebuffer\n");
	vkDestroyRenderPass(device, renderPass, nullptr);
	printf("vkDestroyRenderPass\n");
	
	vkDestroyImage(device, image, nullptr);
	printf("vkDestroyImage\n");
	vkFreeMemory(device, imgMem, nullptr);
	printf("vkFreeMemory\n");

	vkDestroyDevice(device, nullptr);
	printf("vkDestroyDevice\n");

	return 0;
}

int main(int argc, const char **argv)
{
	bool silent = false;

	if (argc > 1 && !strcmp(argv[1], "-s"))
		silent = true;

	uint32_t apiVer = 0;
	vkEnumerateInstanceVersion(&apiVer);
	printf("vulkan instance version %d.%d.%d\n", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_PATCH(apiVer));

	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	printf("layers %d\n", count);	
	std::vector<VkLayerProperties> layers(count);
	vkEnumerateInstanceLayerProperties(&count, layers.data());
	for (int i = 0; i < (int)layers.size(); i++)
	{
		uint32_t leCount = 0;
		vkEnumerateInstanceExtensionProperties(layers[i].layerName, &leCount, nullptr);
		printf(" %d: %s \"%s\" extensions %d\n", i, layers[i].layerName, layers[i].description, leCount); 
	}
	count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensions(count);
	printf("instacne extensions %d\n", count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
	for (int i = 0; i < (int)extensions.size(); i++)
	{
		if (silent) break;
		printf(" %3.1d: %s:%d\n", i, extensions[i].extensionName, extensions[i].specVersion);
	}

	std::vector<const char*> enabledInstExtensions{};
	VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO,nullptr,0,0,0,0,VK_API_VERSION_1_4};
	VkInstanceCreateInfo instInfo {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr,0,&appInfo,0,nullptr,(uint32_t)enabledInstExtensions.size(),enabledInstExtensions.data()};
	VkInstance inst = 0;
	VkResult r = vkCreateInstance(&instInfo, nullptr, &inst);
	printf("%d instance %p\n", r, inst);

	count = 0;
	vkEnumeratePhysicalDevices(inst, &count, nullptr);
	printf("physical devices %d\n", count);
	std::vector<VkPhysicalDevice> physDevices(count);
	vkEnumeratePhysicalDevices(inst, &count, physDevices.data());
	for (int i = 0; i < (int)physDevices.size(); i++)
	{
		printf(" %d: %p\n", i, physDevices[i]);
		test_phys_device(inst, physDevices[i], silent);
	}

	vkDestroyInstance(inst, nullptr);
	printf("vkDestroyInstance\n");
	return 0;
}

