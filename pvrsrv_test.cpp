
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <xf86drm.h>
//#define SUPPORT_LINUX_OSPAGE_MIGRATION 1
#include "pvr_drm.h"
#include "../img-pvrsrv-trace/pvr_srv_bridge.h"

// from mesa pvr_winsys.h
#define PVR_SRV_DRIVER_NAME "pvr"

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

#define PVR_SRV_VERSION_MAJ 24U
#define PVR_SRV_VERSION_MIN 2U
#define PVR_SRV_VERSION_BUILD 6603887

#define PVR_SRV_VERSION                                              \
	(((uint32_t)((uint32_t)(PVR_SRV_VERSION_MAJ) & 0xFFFFU) << 16U) |\
	(((PVR_SRV_VERSION_MIN) & 0xFFFFU) << 0U))

#define PVR_SRV_FLAGS_CLIENT_64BIT_COMPAT BITFIELD_BIT(5U)

// from mesa pvr_device_info.h
#define PVR_BVNC_PACK_SHIFT_B 48
#define PVR_BVNC_PACK_SHIFT_V 32
#define PVR_BVNC_PACK_SHIFT_N 16
#define PVR_BVNC_PACK_SHIFT_C 0

#define PVR_BVNC_PACK_MASK_B UINT64_C(0xFFFF000000000000)
#define PVR_BVNC_PACK_MASK_V UINT64_C(0x0000FFFF00000000)
#define PVR_BVNC_PACK_MASK_N UINT64_C(0x00000000FFFF0000)
#define PVR_BVNC_PACK_MASK_C UINT64_C(0x000000000000FFFF)

#define PVR_BVNC_UNPACK_B(bvnc) \
	((uint16_t)(((bvnc) & PVR_BVNC_PACK_MASK_B) >> PVR_BVNC_PACK_SHIFT_B))

#define PVR_BVNC_UNPACK_V(bvnc) \
	((uint16_t)(((bvnc) & PVR_BVNC_PACK_MASK_V) >> PVR_BVNC_PACK_SHIFT_V))

#define PVR_BVNC_UNPACK_N(bvnc) \
	((uint16_t)(((bvnc) & PVR_BVNC_PACK_MASK_N) >> PVR_BVNC_PACK_SHIFT_N))

#define PVR_BVNC_UNPACK_C(bvnc) \
	((uint16_t)(((bvnc) & PVR_BVNC_PACK_MASK_C) >> PVR_BVNC_PACK_SHIFT_C))

// from img-rogue device_connection.h
/* Flag to be passed over the bridge during connection stating whether CPU cache coherent is available*/
#define PVRSRV_CACHE_COHERENT_SHIFT (0)
#define	PVRSRV_CACHE_COHERENT_DEVICE_FLAG (1U << PVRSRV_CACHE_COHERENT_SHIFT)
#define	PVRSRV_CACHE_COHERENT_CPU_FLAG (2U << PVRSRV_CACHE_COHERENT_SHIFT)
#define	PVRSRV_CACHE_COHERENT_EMULATE_FLAG (4U << PVRSRV_CACHE_COHERENT_SHIFT)
#define PVRSRV_CACHE_COHERENT_MASK (7U << PVRSRV_CACHE_COHERENT_SHIFT)

/* Flag to be passed over the bridge during connection stating whether CPU non-mappable memory is present */
#define PVRSRV_NONMAPPABLE_MEMORY_PRESENT_SHIFT (7)
#define PVRSRV_NONMAPPABLE_MEMORY_PRESENT_FLAG (1U << PVRSRV_NONMAPPABLE_MEMORY_PRESENT_SHIFT)

/* Flag to be passed over the bridge to indicate PDump activity */
#define PVRSRV_PDUMP_IS_RECORDING_SHIFT (4)
#define PVRSRV_PDUMP_IS_RECORDING (1U << PVRSRV_PDUMP_IS_RECORDING_SHIFT)

/* Flag to be passed over the bridge during connection stating SVM allocation availability */
#define PVRSRV_DEVMEM_SVM_ALLOC_SHIFT (8)
#define PVRSRV_DEVMEM_SVM_ALLOC_UNSUPPORTED (1U << PVRSRV_DEVMEM_SVM_ALLOC_SHIFT)
#define PVRSRV_DEVMEM_SVM_ALLOC_SUPPORTED (2U << PVRSRV_DEVMEM_SVM_ALLOC_SHIFT)
#define PVRSRV_DEVMEM_SVM_ALLOC_CANFAIL (4U << PVRSRV_DEVMEM_SVM_ALLOC_SHIFT)

/* Flag to be passed over the bridge during connection stating whether GPU uses FBCDC v3.1 */
#define PVRSRV_FBCDC_V3_1_USED_SHIFT (11)
#define PVRSRV_FBCDC_V3_1_USED (1U << PVRSRV_FBCDC_V3_1_USED_SHIFT)

/* Flag to be passed over the bridge during connection stating whether System has
   DMA transfer capability to and from device memory */
#define PVRSRV_SYSTEM_DMA_SHIFT (12)
#define PVRSRV_SYSTEM_DMA_USED (1U << PVRSRV_SYSTEM_DMA_SHIFT)

/* Flag to be passed over the bridge during connection stating whether GPU supports TFBC and is
   configured to use lossy compression control group 1 (25% / 37.5% / 50%) */
#define PVRSRV_TFBC_LOSSY_GROUP_SHIFT (13)
#define PVRSRV_TFBC_LOSSY_GROUP_1 (1U << PVRSRV_TFBC_LOSSY_GROUP_SHIFT)

// from img-rogue multicore_defs.h
/* Capability bits returned to client in RGXGetMultiCoreInfo */
#define RGX_MULTICORE_CAPABILITY_FRAGMENT_EN    (0x00000040U)
#define RGX_MULTICORE_CAPABILITY_GEOMETRY_EN    (0x00000020U)
#define RGX_MULTICORE_CAPABILITY_COMPUTE_EN     (0x00000010U)
#define RGX_MULTICORE_CAPABILITY_PRIMARY_EN     (0x00000008U)
#define RGX_MULTICORE_ID_CLRMSK                 (0xFFFFFFF8U)

// from img-rogue devicemem_utils.h
#define PVR_DEVMEM_HEAPNAME_MAXLENGTH 160

#define Log printf
#define LogError printf

static int pvr_srv_bridge_call(int fd, uint8_t bridge_id, uint32_t function_id,
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

void print_drm_device(drmDevicePtr dev)
{
	if (!dev)
		return;
	Log("drm device %p available_nodes mask 0x%X bustype %d\n", dev, dev->available_nodes, dev->bustype);
	if (dev->available_nodes & (1<<DRM_NODE_PRIMARY))
		Log(" DRM_NODE_PRIMARY %s\n", dev->nodes[DRM_NODE_PRIMARY]);
	if (dev->available_nodes & (1<<DRM_NODE_RENDER))
		Log(" DRM_NODE_RENDER %s\n", dev->nodes[DRM_NODE_RENDER]);

	if (dev->bustype == DRM_BUS_PLATFORM)
	{
		Log("businfo platform name \"%s\"\n", dev->businfo.platform->fullname);
		Log("deviceinfo platform compatible:\n");
		for (int i = 0; dev->deviceinfo.platform->compatible[i]; i++)
		{
			Log(" %d: \"%s\"\n", i, dev->deviceinfo.platform->compatible[i]);
		}
	}
}

static pvr_srv_error PVRSRVConnect(int fd, uint64_t *packedBvnc, uint32_t *capabilityFlags, uint8_t *kernelArch)
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

static pvr_srv_error PVRSRVDisconnect(int fd)
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

static pvr_srv_error PVRSRVGetDevClockSpeed(int fd, uint32_t *clock_speed)
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

static pvr_srv_error PVRSRVGetMultiCoreInfo(int fd, uint32_t caps_size, uint32_t *num_cores, uint64_t *caps)
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

static pvr_srv_error PVRSRVAcquireInfoPage(int fd, pvr_handle_t *out_pmr)
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

static pvr_srv_error PVRSRVReleaseInfoPage(int fd, pvr_handle_t pmr)
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


static pvr_srv_error PVRSRVPMRLocalImportPMR(int fd, pvr_handle_t ext_handle, pvr_handle_t *pmr, uint64_t *size, uint64_t *align)
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

static pvr_srv_error PVRSRVPMRUnrefPMR(int fd, pvr_handle_t pmr)
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

static pvr_srv_error PVRSRVDevmemIntCtxCreate(int fd, bool kernelMemoryCtx,
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

static pvr_srv_error PVRSRVDevmemIntCtxDestroy(int fd, pvr_handle_t devMemServerContext)
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

static pvr_srv_error PVRSRVHeapCfgHeapConfigCount(int fd, uint32_t *heap_config_count)
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

static pvr_srv_error PVRSRVHeapCfgHeapCount(int fd, uint32_t heap_config_index, uint32_t *heap_count)
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

static pvr_srv_error PVRSRVHeapCfgHeapConfigName(int fd, uint32_t heap_config_index, uint32_t config_name_size, char *config_name_buffer)
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
static pvr_srv_error PVRSRVHeapCfgHeapDetails(int fd, uint32_t heap_config_index, uint32_t heap_index, uint32_t name_size, char *name_buffer,
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

static void dump_hex(char *data, int size, int row = 16)
{
	int di = 0;
	while (di < size)
	{
		for (int j = 0; j < row; j++)
		{
			printf(" %.2X", data[di++]);
			if (di >= size)
			break;
		}
		printf("\n");
	}
}

int main(int argc, const char **argv)
{
	const char *path = "/dev/dri/card0";
	if (argc >= 2)
		path = argv[1];

	//int fd = drmOpen("pvr", 0);//new mainlined is "powervr"
	Log("opening %s\n", path);
	int fd = open(path, O_RDWR | O_CLOEXEC);
	Log("fd %d\n", fd);
	if (fd < 0)
	{
		return -EINVAL;
	}

	{
		drmVersionPtr ver = drmGetLibVersion(fd);
		if (!ver)
			LogError("drmGetLibVersion %p\n", ver);
		if (ver)
		{
			Log("drm lib version %d.%d.%d\n", ver->version_major, ver->version_minor,ver->version_patchlevel);
			drmFreeVersion(ver);
		}
	}

	drmVersionPtr ver = drmGetVersion(fd);
	if (!ver)
	{
		LogError("drmGetVersion %p\n", ver);
		return -1;
	}

	if (ver)
	{
		Log("drm driver version %d.%d.%d\n", ver->version_major, ver->version_minor,ver->version_patchlevel);
		Log("len name %d date %d desc %d\n", ver->name_len, ver->date_len, ver->desc_len);
		if (ver->name_len)
			Log(" name \"%s\"", ver->name);
		if (ver->date_len)
		Log(" date \"%s\"", ver->date);
		if (ver->desc_len)
			Log(" desc \"%s\"", ver->desc);
		Log("\n");

		if (strcmp(ver->name, PVR_SRV_DRIVER_NAME))
		{
			LogError("Unsupported driver \"%s\"\n", ver->name);
			drmFreeVersion(ver);
			return -1;
		}

		if (ver->version_major != PVR_SRV_VERSION_MAJ || ver->version_minor != PVR_SRV_VERSION_MIN)
		{
			LogError("Unsupported driver version (%u.%u)", ver->version_major, ver->version_minor);
			drmFreeVersion(ver);
			return -1;
		}
		drmFreeVersion(ver);
	}

	drmDevicePtr dev = {};
	if (!drmGetDevice(fd, &dev))
	{
		print_drm_device(dev);
		drmFreeDevice(&dev);
	}

	struct drm_pvr_srvkm_init_data init_data = { .init_module = PVR_SRVKM_SERVICES_INIT };
	int result = drmIoctl(fd, DRM_IOCTL_PVR_SRVKM_INIT, &init_data);
	if (result)
	{
		LogError("DRM_IOCTL_PVR_SRVKM_INIT failed %d, Errno: %s", result, strerror(errno));
		close(fd);
		return -1;
	}
	Log("DRM_IOCTL_PVR_SRVKM_INIT PVR_SRVKM_SERVICES_INIT\n");

	uint64_t bvnc = 0;
	uint32_t capability_flags = 0;
	uint8_t kernel_arch = 0;
	pvr_srv_error ret = PVRSRVConnect(fd, &bvnc, &capability_flags, &kernel_arch);
	if (ret)
	{
		close(fd);
		return -1;
	}

	Log("SRVCORE_CONNECT: kernel_arch %dbit", kernel_arch);
	Log(" BVNC %" PRIX64 " (%u.%u.%u.%u)\n", bvnc,
		PVR_BVNC_UNPACK_B(bvnc),
		PVR_BVNC_UNPACK_V(bvnc),
		PVR_BVNC_UNPACK_N(bvnc),
		PVR_BVNC_UNPACK_C(bvnc));

	Log(" capability_flags %X:\n", capability_flags);
#define X(name) if (capability_flags & PVRSRV_##name ) Log("\t" #name "\n");
X(CACHE_COHERENT_DEVICE_FLAG)
X(CACHE_COHERENT_CPU_FLAG)
X(CACHE_COHERENT_EMULATE_FLAG)
X(NONMAPPABLE_MEMORY_PRESENT_FLAG)
X(PDUMP_IS_RECORDING)
X(DEVMEM_SVM_ALLOC_UNSUPPORTED)
X(DEVMEM_SVM_ALLOC_SUPPORTED)
X(DEVMEM_SVM_ALLOC_CANFAIL)
X(FBCDC_V3_1_USED)
X(SYSTEM_DMA_USED)
X(TFBC_LOSSY_GROUP_1)
#undef X

	if (bvnc != 0x24001D003400B6) //36.29.52.182 PowerVR B-Series BXE-2-32 MC1
	{
		LogError("Unsupported device BVNC\n");
		close(fd);
		return -1;
	}

	pvr_handle_t info_page = 0;
	ret = PVRSRVAcquireInfoPage(fd, &info_page);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("AcquireInfoPage %p\n", info_page);

	if (info_page)
	{
		pvr_handle_t info_page_imported = 0;
		uint64_t info_page_size = 0;
		uint64_t info_page_align = 0;
		ret = PVRSRVPMRLocalImportPMR(fd, info_page, &info_page_imported, &info_page_size, &info_page_align);
		if (ret)
		{
			close(fd);
			return -1;
		}
		Log("PMRLocalImportPMR(info_page) pmr %p size %lX align %lX\n", info_page_imported, info_page_size, info_page_align);

		void *info_page_ptr = mmap(nullptr,
			info_page_size,
			PROT_READ,
			MAP_SHARED,
			fd,
			(off_t)info_page_imported * info_page_align//<< log2_page_size
			);
		Log("mmap(info_page) %p\n", info_page_ptr);

		if (info_page_ptr)
		{
			dump_hex((char*)info_page_ptr, info_page_size, 16);
			int um_ret = munmap(info_page_ptr, info_page_size);
			Log("munmap(info_page) %d\n", um_ret);
		}

		ret = PVRSRVPMRUnrefPMR(fd, info_page_imported);
		if (ret)
		{
			close(fd);
			return -1;
		}
		Log("PMRUnrefPMR(info_page)\n");
	}

	ret = PVRSRVReleaseInfoPage(fd, info_page);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("ReleaseInfoPage\n");

	uint32_t clock_speed = 0;
	ret = PVRSRVGetDevClockSpeed(fd, &clock_speed);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("SRVCORE_GETDEVCLOCKSPEED clock speed %d\n", clock_speed);

	uint32_t num_cores = 0;
	uint64_t caps[8] = {0};
	ret = PVRSRVGetMultiCoreInfo(fd, 8, &num_cores, caps);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("SRVCORE_GETMULTICOREINFO num cores %d\n", num_cores);
	for (uint32_t ci = 0; ci < num_cores; ci++)
	{
		Log("  core %d: caps 0x%" PRIX64 " ( ", ci, caps[ci]);
#define X(name) if (caps[ci] & RGX_MULTICORE_CAPABILITY_##name##_EN ) Log( #name " ");
X(FRAGMENT)
X(GEOMETRY)
X(COMPUTE)
X(PRIMARY)
#undef X
		printf(")\n");
	}

	pvr_handle_t server_memctx = 0;
	pvr_handle_t server_memctx_data = 0;
	uint32_t cpu_cache_line_size = 0;
	ret = PVRSRVDevmemIntCtxCreate(fd, false, &server_memctx, &server_memctx_data, &cpu_cache_line_size);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("MM_DEVMEMINTCTXCREATE %d ctx %p priv_data %p CPU cache line %d\n",
		ret, server_memctx, server_memctx_data, cpu_cache_line_size);

	uint32_t heap_config_count = 0;
	ret = PVRSRVHeapCfgHeapConfigCount(fd, &heap_config_count);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("MM_HEAPCFGHEAPCONFIGCOUNT %d\n", heap_config_count);
	for(uint32_t hci = 0; hci < heap_config_count; hci++)
	{
		char buffer[PVR_DEVMEM_HEAPNAME_MAXLENGTH] ={0};
		ret = PVRSRVHeapCfgHeapConfigName(fd, hci, sizeof(buffer), buffer);
		if (ret)
		{
			close(fd);
			return -1;
		}
		uint32_t heap_count = 0;
		ret = PVRSRVHeapCfgHeapCount(fd, hci, &heap_count);
		if (ret)
		{
			close(fd);
			return -1;
		}
		Log("  %d: name \"%s\" heap_count %d\n", hci, buffer, heap_count);
		for (uint32_t hi = 0; hi < heap_count; hi++)
		{
			pvr_dev_addr_t base_addr;
			uint64_t heap_size = 0;
			uint64_t reserved_size = 0;
			uint32_t log2_data_page_size = 0;
			uint32_t log2_import_alignment = 0;
			ret = PVRSRVHeapCfgHeapDetails(fd, hci, hi, sizeof(buffer), buffer,
				&base_addr, &heap_size, &reserved_size, &log2_data_page_size, &log2_import_alignment);
			if (ret)
			{
				close(fd);
				return -1;
			}
			Log("    %d: name \"%-22s\" size %10" PRIX64 " reserved %5" PRIX64 " base_addr %10" PRIX64 " log2_data_page_size %d log2_import_alignment %d\n",
				hi, buffer, heap_size, reserved_size, base_addr.addr, log2_data_page_size, log2_import_alignment);
		}
	}

	//cleanup

	ret = PVRSRVDevmemIntCtxDestroy(fd, server_memctx);
	Log("MM_DEVMEMINTCTXDESTROY %d\n", ret);

	ret = PVRSRVDisconnect(fd);
	Log("SRVCORE_DISCONNECT %d\n", ret);

	close(fd);
	Log("closed fd %d\n", fd);

	return 0;
}

