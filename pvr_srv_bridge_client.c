
#include "pvr_drm.h"
#include "pvr_srv_bridge.h"
#include <xf86drm.h>

#include <stdio.h>

#define Log printf
#define LogError printf

// from mesa pvr_srv_bridge.h
// from img-rogue pvrversion.h rgx_options.h
#define SUPPORT_RGX_SET_OFFSET BITFIELD_BIT(4U)
#define DEBUG_SET_OFFSET BITFIELD_BIT(10U)
#define SUPPORT_BUFFER_SYNC_SET_OFFSET BITFIELD_BIT(11U)
#define NUM_DRIVERS_SUPPORTED_CHECK_SET_OFFSET BITFIELD_BIT(17U)
#define PERCONTEXT_FREELIST_SET_OFFSET BITFIELD_BIT(31U)

#define RGX_BUILD_OPTIONS                       \
	(SUPPORT_RGX_SET_OFFSET | DEBUG_SET_OFFSET |\
	SUPPORT_BUFFER_SYNC_SET_OFFSET |            \
	NUM_DRIVERS_SUPPORTED_CHECK_SET_OFFSET |    \
	PERCONTEXT_FREELIST_SET_OFFSET)

#define PVR_SRV_FLAGS_CLIENT_64BIT_COMPAT BITFIELD_BIT(5U)

int pvr_srv_bridge_call(int fd, uint8_t bridge_id, uint32_t function_id,
	void *input, uint32_t input_buffer_size,void *output, uint32_t output_buffer_size)
{
	struct drm_pvr_srvkm_cmd cmd = {
		.bridge_id = bridge_id,
		.bridge_func_id = function_id,
		.in_data_ptr = (uint64_t)(uintptr_t)input,
		.out_data_ptr = (uint64_t)(uintptr_t)output,
		.in_data_size = input_buffer_size,
		.out_data_size = output_buffer_size,
	};

	int ret = drmIoctl(fd, DRM_IOCTL_PVR_SRVKM_CMD, &cmd);
	return ret;
}


pvr_srv_error PVRSRVConnect(int fd, uint64_t *packedBvnc, uint32_t *capabilityFlags, uint8_t *kernelArch)
{
	struct pvr_srv_bridge_connect_cmd cmd = {
		.build_options = RGX_BUILD_OPTIONS,
		.DDK_build = PVR_SRV_VERSION_BUILD,
		.DDK_version = PVR_SRV_VERSION,
		.flags = PVR_SRV_FLAGS_CLIENT_64BIT_COMPAT,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_bridge_connect_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_SRVCORE, PVR_SRV_BRIDGE_SRVCORE_CONNECT,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_SRVCORE_CONNECT %d error %d", result, ret.error);
		return ret.error;
	}
	if (packedBvnc)
		*packedBvnc = ret.bvnc;
	if (capabilityFlags)
		*capabilityFlags = ret.capability_flags;
	if (kernelArch)
		*kernelArch = ret.kernel_arch;
	return ret.error;
}

pvr_srv_error PVRSRVDisconnect(int fd)
{
	/* Initialize ret.error to a default error */
	struct pvr_srv_bridge_disconnect_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_SRVCORE, PVR_SRV_BRIDGE_SRVCORE_DISCONNECT,
		nullptr, 0, &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_SRVCORE_DISCONNECT %d error %d", result, ret.error);
	}
	return ret.error;
}

pvr_srv_error PVRSRVGetDevClockSpeed(int fd, uint32_t *clock_speed)
{
	/* Initialize ret.error to a default error */
	struct pvr_srv_bridge_getdevclockspeed_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_SRVCORE, PVR_SRV_BRIDGE_SRVCORE_GETDEVCLOCKSPEED,
		nullptr, 0, &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_SRVCORE_GETDEVCLOCKSPEED %d error %d", result, ret.error);
		return ret.error;
	}

	if (clock_speed)
		*clock_speed = ret.clock_speed;
	return ret.error;
}

pvr_srv_error PVRSRVGetMultiCoreInfo(int fd, uint32_t caps_size, uint32_t *num_cores, uint64_t *caps)
{
	struct pvr_srv_bridge_getmulticoreinfo_cmd cmd = {
		.caps = caps,
		.caps_size = caps_size,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_bridge_getmulticoreinfo_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_SRVCORE, PVR_SRV_BRIDGE_SRVCORE_GETMULTICOREINFO,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_SRVCORE_GETMULTICOREINFO %d error %d", result, ret.error);
		return ret.error;
	}

	if (num_cores)
		*num_cores = ret.num_cores;

	return ret.error;
}

pvr_srv_error PVRSRVAcquireInfoPage(int fd, pvr_handle_t *out_pmr)
{
	/* Initialize ret.error to a default error */
	struct pvr_srv_bridge_acquireinfopage_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_SRVCORE, PVR_SRV_BRIDGE_SRVCORE_ACQUIREINFOPAGE,
		nullptr, 0, &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_SRVCORE_ACQUIREINFOPAGE %d error %d", result, ret.error);
		return ret.error;
	}

	if (out_pmr)
		*out_pmr = ret.pmr;

	return ret.error;
}

pvr_srv_error PVRSRVReleaseInfoPage(int fd, pvr_handle_t pmr)
{
	struct pvr_srv_bridge_releaseinfopage_cmd cmd = {
		.pmr = pmr,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_bridge_releaseinfopage_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_SRVCORE, PVR_SRV_BRIDGE_SRVCORE_RELEASEINFOPAGE,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_SRVCORE_RELEASEINFOPAGE %d error %d", result, ret.error);
	}
	return ret.error;
}


pvr_srv_error PVRSRVPMRLocalImportPMR(int fd, pvr_handle_t ext_handle, pvr_handle_t *pmr, uint64_t *size, uint64_t *align)
{
	struct pvr_srv_pmr_localimportpmr_cmd cmd = {
		.ext_handle = ext_handle,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_pmr_localimportpmr_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_PMRLOCALIMPORTPMR,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_PMRLOCALIMPORTPMR %d error %d", result, ret.error);
		return ret.error;
	}

	if (pmr)
		*pmr = ret.pmr;
	if (size)
		*size = ret.size;
	if (align)
		*align = ret.align;

	return ret.error;
}

pvr_srv_error PVRSRVPMRUnrefPMR(int fd, pvr_handle_t pmr)
{
	struct pvr_srv_pmr_unref_pmr_cmd cmd = {
		.pmr = pmr,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_pmr_unref_pmr_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_PMRUNREFPMR,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_PMRUNREFPMR %d error %d", result, ret.error);
	}
	return ret.error;
}

pvr_srv_error PVRSRVDevmemIntCtxCreate(int fd, bool kernelMemoryCtx,
	pvr_handle_t *devMemServerContext, pvr_handle_t *privData, uint32_t *CPUCacheLineSize)
{
	struct pvr_srv_devmem_int_ctx_create_cmd cmd = {
		.kernel_memory_ctx = kernelMemoryCtx,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_devmem_int_ctx_create_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_DEVMEMINTCTXCREATE,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_DEVMEMINTCTXCREATE %d error %d", result, ret.error);
		return ret.error;
	}

	if (devMemServerContext)
		*devMemServerContext = ret.server_memctx;
	if (privData)
		*privData = ret.server_memctx_data;
	if (CPUCacheLineSize)
		*CPUCacheLineSize = ret.cpu_cache_line_size;

	return ret.error;
}

pvr_srv_error PVRSRVDevmemIntCtxDestroy(int fd, pvr_handle_t devMemServerContext)
{
	struct pvr_srv_devmem_int_ctx_destroy_cmd cmd = {
		.server_memctx = devMemServerContext,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_devmem_int_ctx_destroy_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_DEVMEMINTCTXDESTROY,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_DEVMEMINTCTXDESTROY %d error %d", result, ret.error);
	}
	return ret.error;
}

pvr_srv_error PVRSRVHeapCfgHeapConfigCount(int fd, uint32_t *heap_config_count)
{
	/* Initialize ret.error to a default error */
	struct pvr_srv_heap_cfg_count_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGCOUNT,
		nullptr, 0, &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGCOUNT %d error %d", result, ret.error);
		return ret.error;
	}

	if (heap_config_count)
		*heap_config_count = ret.heap_config_count;

	return ret.error;
}

pvr_srv_error PVRSRVHeapCfgHeapCount(int fd, uint32_t heap_config_index, uint32_t *heap_count)
{
	struct pvr_srv_heap_count_cmd cmd = {
		.heap_config_index = heap_config_index,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_heap_count_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCOUNT,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCOUNT %d error %d", result, ret.error);
		return ret.error;
	}

	if (heap_count)
		*heap_count = ret.heap_count;
	return ret.error;
}

pvr_srv_error PVRSRVHeapCfgHeapConfigName(int fd, uint32_t heap_config_index, uint32_t config_name_size, char *config_name_buffer)
{
	struct pvr_srv_heap_cfg_name_cmd cmd = {
		.config_name_buffer = config_name_buffer,
		.heap_config_index = heap_config_index,
		.config_name_bufer_size = config_name_size,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_heap_cfg_name_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGNAME,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGNAME %d error %d", result, ret.error);
	}
	return ret.error;
}

pvr_srv_error PVRSRVHeapCfgHeapDetails(int fd, uint32_t heap_config_index, uint32_t heap_index, uint32_t name_size, char *name_buffer,
pvr_dev_addr_t *base_addr, uint64_t *heap_size, uint64_t *reserved_size, uint32_t *log2_data_page_size, uint32_t *log2_import_alignment)
{
	struct pvr_srv_heap_cfg_details_cmd cmd = {
		.buffer = name_buffer,
		.heap_config_index = heap_config_index,
		.heap_index = heap_index,
		.buffer_size = name_size,
	};

	/* Initialize ret.error to a default error */
	struct pvr_srv_heap_cfg_details_ret ret = {
		.error = PVR_SRV_ERROR_BRIDGE_CALL_FAILED,
	};

	int result = pvr_srv_bridge_call(fd, PVR_SRV_BRIDGE_MM, PVR_SRV_BRIDGE_MM_HEAPCFGHEAPDETAILS,
		&cmd, sizeof(cmd), &ret, sizeof(ret));
	if (result || ret.error != PVR_SRV_OK)
	{
		LogError("PVR_SRV_BRIDGE_MM_HEAPCFGHEAPDETAILS %d error %d", result, ret.error);
		return ret.error;
	}

	if (base_addr)
		*base_addr = ret.base_addr;
	if (heap_size)
		*heap_size = ret.size;
	if (reserved_size)
		*reserved_size = ret.reserved_size;
	if (log2_data_page_size)
		*log2_data_page_size = ret.log2_page_size;
	if (log2_import_alignment)
		*log2_import_alignment = ret.log2_alignment;
	return ret.error;
}

