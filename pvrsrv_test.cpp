
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
#include "pvr_srv_bridge.h"

// from mesa pvr_winsys.h
#define PVR_SRV_DRIVER_NAME "pvr"

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

// from img-rogue info_page_defs.h
#define INFO_PAGE_CHUNK_SIZE                        8
#define INFO_PAGE_BLOCK_END(start,size)             ((start) + (size) * INFO_PAGE_CHUNK_SIZE)
#define INFO_PAGE_ENTRY(start,index)                ((start) + (index))
#define INFO_PAGE_SIZE_IN_BYTES(end)                ((end) * sizeof(uint32_t))

#define INFO_PAGE_CACHEOP_BLOCK_START               0
#define INFO_PAGE_CACHEOP_BLOCK_END                 INFO_PAGE_BLOCK_END(INFO_PAGE_CACHEOP_BLOCK_START, 1)
#define INFO_PAGE_HWPERF_BLOCK_START                INFO_PAGE_CACHEOP_BLOCK_END
#define INFO_PAGE_HWPERF_BLOCK_END                  INFO_PAGE_BLOCK_END(INFO_PAGE_HWPERF_BLOCK_START, 1)
#define INFO_PAGE_TIMEOUT_BLOCK_START               INFO_PAGE_HWPERF_BLOCK_END
#define INFO_PAGE_TIMEOUT_BLOCK_END                 INFO_PAGE_BLOCK_END(INFO_PAGE_TIMEOUT_BLOCK_START, 2)
#define INFO_PAGE_BRIDGE_BLOCK_START                INFO_PAGE_TIMEOUT_BLOCK_END
#define INFO_PAGE_BRIDGE_BLOCK_END                  INFO_PAGE_BLOCK_END(INFO_PAGE_BRIDGE_BLOCK_START, 1)
#define INFO_PAGE_DEBUG_BLOCK_START                 INFO_PAGE_BRIDGE_BLOCK_END
#define INFO_PAGE_DEBUG_BLOCK_END                   INFO_PAGE_BLOCK_END(INFO_PAGE_DEBUG_BLOCK_START, 1)
#define INFO_PAGE_DEVMEM_BLOCK_START                INFO_PAGE_DEBUG_BLOCK_END
#define INFO_PAGE_DEVMEM_BLOCK_END                  INFO_PAGE_BLOCK_END(INFO_PAGE_DEVMEM_BLOCK_START, 1)

/* IMPORTANT: Make sure this always uses the last INFO_PAGE_[NAME]_BLOCK_END definition.*/
#define INFO_PAGE_TOTAL_SIZE                        INFO_PAGE_SIZE_IN_BYTES(INFO_PAGE_DEVMEM_BLOCK_END)

// pvrsrv_memallocflags.h
#define PVRSRV_MEMALLOCFLAG_GPU_READABLE		(uint64_t(1)<<0)
#define PVRSRV_MEMALLOCFLAG_GPU_WRITEABLE		(uint64_t(1)<<1)
#define PVRSRV_MEMALLOCFLAG_GPU_READ_PERMITTED	(uint64_t(1)<<2)
#define PVRSRV_MEMALLOCFLAG_GPU_WRITE_PERMITTED	(uint64_t(1)<<3)
#define PVRSRV_MEMALLOCFLAG_CPU_READABLE		(uint64_t(1)<<4)
#define PVRSRV_MEMALLOCFLAG_CPU_WRITEABLE		(uint64_t(1)<<5)
#define PVRSRV_MEMALLOCFLAG_CPU_READ_PERMITTED	(uint64_t(1)<<6)
#define PVRSRV_MEMALLOCFLAG_CPU_WRITE_PERMITTED	(uint64_t(1)<<7)

#define PVRSRV_MEMALLOCFLAG_GPU_UNCACHED_WC			(uint64_t(0)<<8)
#define PVRSRV_MEMALLOCFLAG_GPU_UNCACHED			(uint64_t(1)<<8)
#define PVRSRV_MEMALLOCFLAG_GPU_CACHE_COHERENT		(uint64_t(2)<<8)
#define PVRSRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT	(uint64_t(3)<<8)
#define PVRSRV_MEMALLOCFLAG_GPU_CACHED				(uint64_t(7)<<8)

#define PVRSRV_MEMALLOCFLAG_GPU_CACHE_MODE_MASK		(uint64_t(7)<<8)
#define PVRSRV_GPU_CACHE_MODE(uiFlags)				((uiFlags) & PVRSRV_MEMALLOCFLAG_GPU_CACHE_MODE_MASK)

#define PVRSRV_MEMALLOCFLAG_CPU_UNCACHED_WC			(uint64_t(0)<<11)
#define PVRSRV_MEMALLOCFLAG_CPU_CACHE_COHERENT		(uint64_t(2)<<11)
#define PVRSRV_MEMALLOCFLAG_CPU_CACHE_INCOHERENT	(uint64_t(3)<<11)
#define PVRSRV_MEMALLOCFLAG_CPU_CACHED				(uint64_t(7)<<11)

#define PVRSRV_MEMALLOCFLAG_CPU_CACHE_MODE_MASK		(uint64_t(7)<<11)
#define PVRSRV_CPU_CACHE_MODE(uiFlags)				((uiFlags) & PVRSRV_MEMALLOCFLAG_CPU_CACHE_MODE_MASK)

#define PVRSRV_MEMALLOCFLAG_KERNEL_CPU_MAPPABLE		(uint64_t(1)<<14)

#define PVRSRV_PHYS_HEAP_HINT_SHIFT			(59)
#define PVRSRV_PHYS_HEAP_HINT_MASK			(uint64_t(0x1F) << PVRSRV_PHYS_HEAP_HINT_SHIFT)
#define PVRSRV_GET_PHYS_HEAP_HINT(uiFlags)	((pvr_phys_heap)(((uiFlags) & PVRSRV_PHYS_HEAP_HINT_MASK) >> PVRSRV_PHYS_HEAP_HINT_SHIFT))


#define Log printf
#define LogError printf

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

	Log("INFO_PAGE_TOTAL_SIZE %zu\n", INFO_PAGE_TOTAL_SIZE);

	pvr_handle_t info_page = 0;
	ret = PVRSRVAcquireInfoPage(fd, &info_page);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("AcquireInfoPage %p\n", info_page);

	const int log2_page_size = 12;//TODO get from os

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
			(off_t)info_page_imported << log2_page_size
			);
		Log("mmap(info_page) %p\n", info_page_ptr);

		if (info_page_ptr)
		{
			dump_hex((char*)info_page_ptr, INFO_PAGE_TOTAL_SIZE, 16);
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

	struct heap_details_t
	{
		pvr_dev_addr_t base_addr;
		uint64_t heap_size = 0;
		uint64_t reserved_size = 0;
		uint32_t log2_data_page_size = 0;
		uint32_t log2_import_alignment = 0;
	};
	heap_details_t heap_infos[10];

	uint32_t general_heap_index = 0;
	for (uint32_t hci = 0; hci < heap_config_count; hci++)
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
			heap_details_t hd;
			ret = PVRSRVHeapCfgHeapDetails(fd, hci, hi, sizeof(buffer), buffer,
				&hd.base_addr, &hd.heap_size, &hd.reserved_size, &hd.log2_data_page_size, &hd.log2_import_alignment);
			if (ret)
			{
				close(fd);
				return -1;
			}
			Log("    %d: name \"%-22s\" size %10" PRIX64 " reserved %5" PRIX64 " base_addr %10" PRIX64 " log2_data_page_size %d log2_import_alignment %d\n",
				hi, buffer, hd.heap_size, hd.reserved_size, hd.base_addr.addr, hd.log2_data_page_size, hd.log2_import_alignment);
			if (hci == 0 && hi < 10)
			{
				heap_infos[hi] = hd;
				if (strcmp(buffer, "General") == 0)
					general_heap_index = hi;
			}
		}
	}

	pvr_handle_t general_heap = 0;
	pvr_handle_t general_heap_pmr = 0;
	ret = PVRSRVDevmemIntHeapCreate(fd, server_memctx, 0, general_heap_index, &general_heap);
	if (ret)
	{
		close(fd);
		return -1;
	}
	Log("DevmemIntHeapCreate(general %d) %p\n", general_heap_index, general_heap);

	{
		heap_details_t &gh = heap_infos[general_heap_index];
		uint32_t size = 1 << gh.log2_data_page_size;
		uint32_t mapping_table = 0;
		uint64_t flags =
			PVRSRV_MEMALLOCFLAG_GPU_READABLE |
			PVRSRV_MEMALLOCFLAG_GPU_WRITEABLE |
			PVRSRV_MEMALLOCFLAG_CPU_READABLE |
			PVRSRV_MEMALLOCFLAG_CPU_WRITEABLE |
			PVRSRV_MEMALLOCFLAG_GPU_CACHE_INCOHERENT;
		uint64_t out_flags = 0;
		const char *annotation = "General Static Memory";
		ret = PVRSRVPhysmemNewRamBackedPMR(fd, size, 1, 1, &mapping_table, gh.log2_data_page_size, flags,
			strlen(annotation) + 1, annotation, getpid(), &general_heap_pmr, 0, &out_flags);
		if (ret)
		{
			close(fd);
			return -1;
		}
		Log("PhysmemNewRamBackedPMR(general) %p flags 0x%" PRIX64 " out_flags 0x%" PRIX64 , general_heap_pmr, flags, out_flags);
		Log(" PHYS_HEAP_HINT %d\n", PVRSRV_GET_PHYS_HEAP_HINT(out_flags));
	}

	//cleanup
	ret = PVRSRVPMRUnrefPMR(fd, general_heap_pmr);
	Log("PMRUnrefPMR(general_heap_pmr) %d\n", ret);

	ret = PVRSRVDevmemIntHeapDestroy(fd, general_heap);
	Log("DevmemIntHeapDestroy %d\n", ret);

	ret = PVRSRVDevmemIntCtxDestroy(fd, server_memctx);
	Log("MM_DEVMEMINTCTXDESTROY %d\n", ret);

	ret = PVRSRVDisconnect(fd);
	Log("SRVCORE_DISCONNECT %d\n", ret);

	close(fd);
	Log("closed fd %d\n", fd);

	return 0;
}

