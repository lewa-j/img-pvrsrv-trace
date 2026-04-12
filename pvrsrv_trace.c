//lewa_j 2025
//Attempt to debug vulkan driver 24.2@6603887 on OrangePi RV2 with PowerVR B-Series BXE-2-32 MC1
//Using bits and pieces from mesa imagination driver and https://github.com/orangepi-xunlong/linux-orangepi/tree/orange-pi-6.6-ky/drivers/gpu/drm/img-rogue

//Usage: ./pvrsrv_trace ./vk_test -s

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <linux/ptrace.h>
#include <syscall.h>
#include <string.h>
#include <xf86drm.h>

#define PACKED __attribute__((__packed__))

/* Ioctl to pass cmd and ret structures. */
struct drm_srvkm_cmd {
   uint32_t bridge_id;
   uint32_t bridge_func_id;
   uint64_t in_data_ptr;
   uint64_t out_data_ptr;
   uint32_t in_data_size;
   uint32_t out_data_size;
};

struct pvr_sync_rename_ioctl_data {
	char szName[32];
};

struct drm_srvkm_sw_sync_create_fence_data {
   char name[32];
   __s32 fence;
   __u32 pad;
   __u64 sync_pt_idx;
};

struct drm_srvkm_sw_timeline_advance_data {
   __u64 sync_pt_idx;
};

#define PVR_SRVKM_SERVICES_INIT 1U
#define PVR_SRVKM_SYNC_INIT 2U
#define PVR_SRVKM_SYNC_EXP_FENCE_INIT 3U
#define PVR_SRVKM_SERVICES_PAGE_MIGRATE_INIT 4U
/* Ioctl to initialize a module. */
struct drm_srvkm_init_data {
   uint32_t init_module;
};

#define DRM_SRVKM_CMD 0U /* PVR Services command. */
/* PVR Sync commands */
#define DRM_SRVKM_SYNC_RENAME_CMD 1U
#define DRM_SRVKM_SYNC_FORCE_SW_ONLY_CMD 2U
/* PVR Software Sync commands */
#define DRM_SRVKM_SW_SYNC_CREATE_FENCE_CMD 3U
#define DRM_SRVKM_SW_SYNC_INC_CMD 4U

#define DRM_SRVKM_INIT 5U /* PVR Services Render Device Init command. */

#define DRM_IOCTL_SRVKM_CMD \
   DRM_IOWR(DRM_COMMAND_BASE + DRM_SRVKM_CMD, struct drm_srvkm_cmd)

#define DRM_IOCTL_SRVKM_SYNC_RENAME_CMD \
	DRM_IOW(DRM_COMMAND_BASE + DRM_SRVKM_SYNC_RENAME_CMD, \
		struct pvr_sync_rename_ioctl_data)
#define DRM_IOCTL_SRVKM_SYNC_FORCE_SW_ONLY_CMD \
   DRM_IO(DRM_COMMAND_BASE + DRM_SRVKM_SYNC_FORCE_SW_ONLY_CMD)

#define DRM_IOCTL_SRVKM_SW_SYNC_CREATE_FENCE_CMD                   \
   DRM_IOWR(DRM_COMMAND_BASE + DRM_SRVKM_SW_SYNC_CREATE_FENCE_CMD, \
            struct drm_srvkm_sw_sync_create_fence_data)
#define DRM_IOCTL_SRVKM_SW_SYNC_INC_CMD                  \
   DRM_IOR(DRM_COMMAND_BASE + DRM_SRVKM_SW_SYNC_INC_CMD, \
           struct drm_srvkm_sw_timeline_advance_data)

#define DRM_IOCTL_SRVKM_INIT \
   DRM_IOWR(DRM_COMMAND_BASE + DRM_SRVKM_INIT, struct drm_srvkm_init_data)

//
//same as DRM_IOCTL_SRVKM_INIT but IOW not IOWR
#define DRM_IOCTL_PVR_SRVKM_INIT \
	DRM_IOW(DRM_COMMAND_BASE + DRM_SRVKM_INIT, \
		struct drm_srvkm_init_data)

//
#define PVR_SRV_BRIDGE_SRVCORE 1UL

#define PVR_SRV_BRIDGE_SRVCORE_CONNECT 0UL
#define PVR_SRV_BRIDGE_SRVCORE_DISCONNECT 1UL
#define PVR_SRV_BRIDGE_SRVCORE_ACQUIREGLOBALEVENTOBJECT 2UL
#define PVR_SRV_BRIDGE_SRVCORE_RELEASEGLOBALEVENTOBJECT 3UL
#define PVR_SRV_BRIDGE_SRVCORE_EVENTOBJECTOPEN 4UL
#define PVR_SRV_BRIDGE_SRVCORE_EVENTOBJECTCLOSE 6UL
#define PVR_SRV_BRIDGE_SRVCORE_ALIGNMENTCHECK 10UL
#define PVR_SRV_BRIDGE_SRVCORE_GETMULTICOREINFO 12UL
#define PVR_SRV_BRIDGE_SRVCORE_ACQUIREINFOPAGE 15UL
#define PVR_SRV_BRIDGE_SRVCORE_RELEASEINFOPAGE 16UL


#define PVR_SRV_BRIDGE_SYNC 2UL

#define PVR_SRV_BRIDGE_SYNC_ALLOCSYNCPRIMITIVEBLOCK 0UL
#define PVR_SRV_BRIDGE_SYNC_FREESYNCPRIMITIVEBLOCK 1UL
#define PVR_SRV_BRIDGE_SYNC_SYNCPRIMSET 2UL
#define PVR_SRV_BRIDGE_SYNC_SYNCALLOCEVENT 7UL
#define PVR_SRV_BRIDGE_SYNC_SYNCFREEEVENT 8UL

#define PVR_SRV_BRIDGE_MM 6UL

#define PVR_SRV_BRIDGE_MM_PMRMAKELOCALIMPORTHANDLE 3UL
#define PVR_SRV_BRIDGE_MM_PMRUNMAKELOCALIMPORTHANDLE 4UL
#define PVR_SRV_BRIDGE_MM_PMRLOCALIMPORTPMR	6UL
#define PVR_SRV_BRIDGE_MM_PMRUNREFPMR 7UL
#define PVR_SRV_BRIDGE_MM_PHYSMEMNEWRAMBACKEDPMR 8UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTCTXCREATE 9UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTCTXDESTROY 10UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTHEAPCREATE 11UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTHEAPDESTROY 12UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTMAPPMR 13UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTUNMAPPMR 14UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTRESERVERANGE 15UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTRESERVERANGEANDMAPPMR 16UL
#define PVR_SRV_BRIDGE_MM_DEVMEMINTUNRESERVERANGE 17UL
#define PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCOUNT 22UL
#define PVR_SRV_BRIDGE_MM_HEAPCFGHEAPDETAILS 24UL
#define PVR_SRV_BRIDGE_MM_PHYSHEAPGETMEMINFO 26UL
#define PVR_SRV_BRIDGE_MM_DEVMEMXINTRESERVERANGE 30UL
#define PVR_SRV_BRIDGE_MM_DEVMEMXINTUNRESERVERANGE 31UL
#define PVR_SRV_BRIDGE_MM_DEVMEMXINTMAPPAGES 32UL
#define PVR_SRV_BRIDGE_MM_DEVMEMXINTUNMAPPAGES 33UL
#define PVR_SRV_BRIDGE_MM_DEVMEMXINTMAPVRANGETOBACKINGPAGE 34UL

#define PVR_SRV_BRIDGE_RGXTQ 128UL

#define PVR_SRV_BRIDGE_RGXTQ_RGXCREATETRANSFERCONTEXT 0UL
#define PVR_SRV_BRIDGE_RGXTQ_RGXDESTROYTRANSFERCONTEXT 1UL
#define PVR_SRV_BRIDGE_RGXTQ_RGXTQGETSHAREDMEMORY 4UL
#define PVR_SRV_BRIDGE_RGXTQ_RGXTQRELEASESHAREDMEMORY 5UL

#define PVR_SRV_BRIDGE_RGXCMP 129UL

#define PVR_SRV_BRIDGE_RGXCMP_RGXCREATECOMPUTECONTEXT 0UL
#define PVR_SRV_BRIDGE_RGXCMP_RGXDESTROYCOMPUTECONTEXT 1UL

#define PVR_SRV_BRIDGE_RGXTA3D 130UL

#define PVR_SRV_BRIDGE_RGXTA3D_RGXDESTROYHWRTDATASET 0UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEZSBUFFER 1UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXDESTROYZSBUFFER 2UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXDESTROYFREELIST 5UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXDESTROYRENDERCONTEXT 6UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXKICKTA3D2 10UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEHWRTDATASET 12UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEFREELIST 13UL
#define PVR_SRV_BRIDGE_RGXTA3D_RGXCREATERENDERCONTEXT 14UL

const char *srv_bridge_id_to_str(int b)
{
	switch (b)
	{
#define X(name) case PVR_SRV_BRIDGE_##name: return #name;
		X(SRVCORE)
		X(SYNC)
		X(MM)
		X(RGXTQ)
		X(RGXCMP)
		X(RGXTA3D)
#undef X
	}
	return "unknown";
}

const char *srv_bridge_func_to_str(int b, int f)
{
	switch (b)
	{
		case PVR_SRV_BRIDGE_SRVCORE:
		{
			switch (f)
			{
#define X(name) case PVR_SRV_BRIDGE_SRVCORE_##name: return #name;
				X(CONNECT)
				X(DISCONNECT)
				X(ACQUIREGLOBALEVENTOBJECT)
				X(RELEASEGLOBALEVENTOBJECT)
				X(EVENTOBJECTOPEN)
				X(EVENTOBJECTCLOSE)
				X(ALIGNMENTCHECK)
				X(GETMULTICOREINFO)
				X(ACQUIREINFOPAGE)
				X(RELEASEINFOPAGE)
#undef X
			}
			break;
		}
		case PVR_SRV_BRIDGE_SYNC:
		{
			switch (f)
			{
#define X(name) case PVR_SRV_BRIDGE_SYNC_##name: return #name;
				X(ALLOCSYNCPRIMITIVEBLOCK)
				X(FREESYNCPRIMITIVEBLOCK)
				X(SYNCPRIMSET)
				X(SYNCALLOCEVENT)
				X(SYNCFREEEVENT)
#undef X
			}
			break;
		}
		case PVR_SRV_BRIDGE_MM:
		{
			switch (f)
			{
#define X(name) case PVR_SRV_BRIDGE_MM_##name: return #name;
				X(PMRMAKELOCALIMPORTHANDLE)
				X(PMRUNMAKELOCALIMPORTHANDLE)
				X(PMRLOCALIMPORTPMR)
				X(PMRUNREFPMR)
				X(PHYSMEMNEWRAMBACKEDPMR)
				X(DEVMEMINTCTXCREATE)
				X(DEVMEMINTCTXDESTROY)
				X(DEVMEMINTHEAPCREATE)
				X(DEVMEMINTHEAPDESTROY)
				X(DEVMEMINTMAPPMR)
				X(DEVMEMINTUNMAPPMR)
				X(DEVMEMINTRESERVERANGE)
				X(DEVMEMINTRESERVERANGEANDMAPPMR)
				X(DEVMEMINTUNRESERVERANGE)
				X(HEAPCFGHEAPCOUNT)
				X(HEAPCFGHEAPDETAILS)
				X(PHYSHEAPGETMEMINFO)
				X(DEVMEMXINTRESERVERANGE)
				X(DEVMEMXINTUNRESERVERANGE)
				X(DEVMEMXINTMAPPAGES)
				X(DEVMEMXINTUNMAPPAGES)
				X(DEVMEMXINTMAPVRANGETOBACKINGPAGE)
#undef X
			}
			break;
		}
		case PVR_SRV_BRIDGE_RGXTQ:
		{
			switch (f)
			{
#define X(name) case PVR_SRV_BRIDGE_RGXTQ_##name: return #name;
				X(RGXCREATETRANSFERCONTEXT)
				X(RGXDESTROYTRANSFERCONTEXT)
				X(RGXTQGETSHAREDMEMORY)
				X(RGXTQRELEASESHAREDMEMORY)
#undef X
			}
			break;
		}
		case PVR_SRV_BRIDGE_RGXCMP:
		{
			switch (f)
			{
#define X(name) case PVR_SRV_BRIDGE_RGXCMP_##name: return #name;
				X(RGXCREATECOMPUTECONTEXT)
				X(RGXDESTROYCOMPUTECONTEXT)
#undef X
			}
			break;
		}
		case PVR_SRV_BRIDGE_RGXTA3D:
		{
			switch (f)
			{
#define X(name) case PVR_SRV_BRIDGE_RGXTA3D_##name: return #name;
				X(RGXDESTROYHWRTDATASET)
				X(RGXCREATEZSBUFFER)
				X(RGXDESTROYZSBUFFER)
				X(RGXDESTROYFREELIST)
				X(RGXDESTROYRENDERCONTEXT)
				X(RGXKICKTA3D2)
				X(RGXCREATEHWRTDATASET)
				X(RGXCREATEFREELIST)
				X(RGXCREATERENDERCONTEXT)
#undef X
			}
			break;
		}	
	}
	return "unknown";
}

const char *drm_ioctl_to_str(int c)
{
	switch (c)
	{
#define X(name) case DRM_IOCTL_##name: return #name;
		X(VERSION)
		X(SRVKM_CMD)
		X(SRVKM_SYNC_RENAME_CMD)
		X(SRVKM_SYNC_FORCE_SW_ONLY_CMD)
		X(SRVKM_SW_SYNC_CREATE_FENCE_CMD)
		X(SRVKM_SW_SYNC_INC_CMD)
		X(SRVKM_INIT)
		X(PVR_SRVKM_INIT)
#undef X
	}
	return nullptr;
}


#define ROGUE_FWIF_NUM_RTDATAS 4U
#define ROGUE_FWIF_NUM_GEOMDATAS 4U
#define ROGUE_FWIF_NUM_RTDATA_FREELISTS 12U

enum pvr_srv_error {
   PVR_SRV_OK,
   PVR_SRV_ERROR_RETRY = 25,
   PVR_SRV_ERROR_DDK_VERSION_MISMATCH = 26,
   PVR_SRV_ERROR_DDK_BUILD_MISMATCH = 27,
   PVR_SRV_ERROR_BUILD_OPTIONS_MISMATCH = 28,
   PVR_SRV_ERROR_BRIDGE_CALL_FAILED = 37,
   PVR_SRV_ERROR_HANDLE_INDEX_OUT_OF_RANGE = 203,
   PVR_SRV_ERROR_BRIDGE_ARRAY_SIZE_TOO_BIG = 350,
   PVR_SRV_ERROR_FORCE_I32 = 0x7fffffff
};

typedef struct pvr_dev_addr {
   uint64_t addr;
} pvr_dev_addr_t;


struct pvr_srv_bridge_connect_cmd {
   uint32_t build_options;
   uint32_t DDK_build;
   uint32_t DDK_version;
   uint32_t flags;
} PACKED;

struct pvr_srv_bridge_connect_ret {
   uint64_t bvnc;
   enum pvr_srv_error error;
   uint32_t capability_flags;
   uint8_t kernel_arch;
} PACKED;


struct pvr_srv_physmem_new_ram_backed_pmr_cmd {
   uint64_t size;
   uint32_t *mapping_table;
   const char *annotation;
   uint32_t annotation_size;
   uint32_t log2_page_size;
   uint32_t phy_blocks;
   uint32_t virt_blocks;
   uint32_t pdump_flags;
   uint32_t pid;
   uint64_t flags;
} PACKED;

struct pvr_srv_physmem_new_ram_backed_pmr_ret {
   void *pmr;
   enum pvr_srv_error error;
   uint64_t out_flags;
} PACKED;


struct pvr_srv_devmem_int_map_pmr_cmd {
   void *pmr;
   void *reservation;
} PACKED;

struct pvr_srv_devmem_int_map_pmr_ret {
   enum pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_reserve_range_cmd {
   pvr_dev_addr_t addr;
   uint64_t size;
   void *server_heap;
   uint64_t flags;
} PACKED;

struct pvr_srv_devmem_int_reserve_range_ret {
   void *reservation;
   enum pvr_srv_error error;
} PACKED;


struct PVRSRV_BRIDGE_IN_DEVMEMINTRESERVERANGEANDMAPPMR
{
	pvr_dev_addr_t sAddress;
	size_t uiLength;
	void *hDevmemServerHeap;
	void *hPMR;
	uint64_t uiFlags;
} PACKED;

struct PVRSRV_BRIDGE_OUT_DEVMEMINTRESERVERANGEANDMAPPMR
{
	void *hReservation;
	enum pvr_srv_error eError;
} PACKED;


struct pvr_srv_rgx_create_free_list_cmd {
   void *free_list_reservation;
   void *mem_ctx_priv_data;
   void *global_free_list;
   uint32_t grow_free_list_pages;
   uint32_t grow_param_threshold;
   uint32_t init_free_list_pages;
   uint32_t max_free_list_pages;
   bool free_list_check;
} PACKED;

struct pvr_srv_rgx_create_free_list_ret {
   void *cleanup_cookie;
   enum pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_create_hwrt_dataset_cmd {
   uint64_t flipped_multi_sample_ctl;
   uint64_t multi_sample_ctl;
   /* ROGUE_FWIF_NUM_RTDATAS sized array. */
   const pvr_dev_addr_t *macrotile_array_dev_addrs;
   /* ROGUE_FWIF_NUM_RTDATAS sized array. */
   const pvr_dev_addr_t *pm_mlist_dev_addrs;
   /* ROGUE_FWIF_NUM_GEOMDATAS sized array. */
   const pvr_dev_addr_t *rtc_dev_addrs;
   /* ROGUE_FWIF_NUM_RTDATAS sized array. */
   const pvr_dev_addr_t *rgn_header_dev_addrs;
   /* ROGUE_FWIF_NUM_GEOMDATAS sized array. */
   const pvr_dev_addr_t *tail_ptrs_dev_addrs;
   /* ROGUE_FWIF_NUM_GEOMDATAS sized array. */
   const pvr_dev_addr_t *vheap_table_dev_adds;
   /* ROGUE_FWIF_NUM_RTDATAS sized array of handles. */
   void **hwrt_dataset;
   /* ROGUE_FWIF_NUM_RTDATA_FREELISTS size array of handles. */
   void **free_lists;
   uint32_t isp_merge_lower_x;
   uint32_t isp_merge_lower_y;
   uint32_t isp_merge_scale_x;
   uint32_t isp_merge_scale_y;
   uint32_t isp_merge_upper_x;
   uint32_t isp_merge_upper_y;
   uint32_t isp_mtile_size;
   uint32_t mtile_stride;
   uint32_t ppp_screen;
   uint32_t rgn_header_size;
   uint32_t te_aa;
   uint32_t te_mtile1;
   uint32_t te_mtile2;
   uint32_t te_screen;
   uint32_t tpc_size;
   uint32_t tpc_stride;
   uint16_t max_rts;
}PACKED;

struct pvr_srv_rgx_create_hwrt_dataset_ret {
   /* ROGUE_FWIF_NUM_RTDATAS sized array of handles. */
   void **hwrt_dataset;
   enum pvr_srv_error error;
}PACKED;


struct pvr_srv_rgx_kick_ta3d2_cmd {
   uint64_t deadline;
   void *hw_rt_dataset;
   void *msaa_scratch_buffer;
   void *pr_fence_ufo_sync_prim_block;
   void *render_ctx;
   void *zs_buffer;
   uint32_t *client_3d_update_sync_offset;
   uint32_t *client_3d_update_value;
   uint32_t *client_ta_fence_sync_offset;
   uint32_t *client_ta_fence_value;
   uint32_t *client_ta_update_sync_offset;
   uint32_t *client_ta_update_value;
   uint32_t *sync_pmr_flags;
   uint8_t *cmd_3d;
   uint8_t *cmd_3d_pr;
   uint8_t *cmd_ta;
   char *update_fence_name;
   char *update_fence_name_3d;
   void **client_3d_update_sync_prim_block;
   void **client_ta_fence_sync_prim_block;
   void **client_ta_update_sync_prim_block;
   void **sync_pmrs;
   int32_t check_fence;
   int32_t check_fence_3d;
   int32_t update_timeline;
   int32_t update_timeline_3d;
   uint32_t cmd_3d_size;
   uint32_t cmd_3d_pr_size;
   uint32_t client_3d_update_count;
   uint32_t client_ta_fence_count;
   uint32_t client_ta_update_count;
   uint32_t ext_job_ref;
   uint32_t num_draw_calls;
   uint32_t num_indices;
   uint32_t num_mrts;
   uint32_t pdump_flags;
   uint32_t client_pr_fence_ufo_sync_offset;
   uint32_t client_pr_fence_value;
   uint32_t render_target_size;
   uint32_t sync_pmr_count;
   uint32_t cmd_ta_size;
   bool abort;
   bool kick_3d;
   bool kick_pr;
   bool kick_ta;
} PACKED;

struct pvr_srv_rgx_kick_ta3d2_ret {
   enum pvr_srv_error error;
   int32_t update_fence;
   int32_t update_fence_3d;
} PACKED;


int strcpy_from_trace(int pid, __u64 src, char *dst, size_t dst_len)
{
	dst[0] = 0;
	if (!src)
		return 0;
	int offs = 0;
	while (offs < dst_len)
	{
		long data = ptrace(PTRACE_PEEKDATA, pid, src + offs, NULL);
		int l = strnlen((const char*)&data, sizeof(data));
		if (offs + l >= dst_len)
			l = dst_len - offs - 1;
		memcpy(dst + offs, &data, l);
		offs += l;
		if (l != sizeof(data))
			break;
	}
	dst[offs] = 0;
	return offs;
}

int memcpy_from_trace(int pid, __u64 src, void *dst, size_t len)
{
	if (!src)
		return 0;
	int offs = 0;
	while (offs < len)
	{
		long data = ptrace(PTRACE_PEEKDATA, pid, src + offs, NULL);
		int l = sizeof(data);
		if (offs + l > len)
			l = len - offs;
		memcpy(dst + offs, &data, l);
		offs += l;
	}
	return offs;
}

int insert_new_fd(int *array, int size, int value)
{
	for (int i = 0; i < size; i++)
	{
		if (array[i])
			continue;
		array[i] = value;
		return i;
	}
	return -1;
}

int remove_fd(int *array, int size, int value)
{
	for (int i = 0; i < size; i++)
	{
		if (array[i] != value)
			continue;
		array[i] = 0;
		return i;
	}
	return -1;
}

int find_fd(int *array, int size, int value)
{
	for (int i = 0; i < size; i++)
	{
		if (array[i] == value)
			return i;
	}
	return -1;
}

static int i_min(int a, int b)
{
	return (a < b) ? a : b;
}

void print_drm_version(int pid, __u64 src)
{
	drm_version_t data = {0};
	memcpy_from_trace(pid, src, &data, sizeof(data));
	
	printf("drm_version: %d.%d.%d name_len %zu date_len %zu desc_len %zu\n", data.version_major, data.version_minor, data.version_patchlevel,
		data.name_len, data.date_len, data.desc_len);
	char name[2048]={0};
	memcpy_from_trace(pid, (__u64)data.name, name, i_min(data.name_len, sizeof(name) - 1));
	char date[2048]={0};
	memcpy_from_trace(pid, (__u64)data.date, date, i_min(data.date_len, sizeof(date) - 1));
	char desc[2048]={0};
	memcpy_from_trace(pid, (__u64)data.desc, desc, i_min(data.desc_len, sizeof(desc) - 1));
	printf(" name \"%s\" date \"%s\" desc \"%s\"\n", name, date, desc);
}

void print_pvrsrv_init(int pid, __u64 src)
{
	struct drm_srvkm_init_data data = {0};
	memcpy_from_trace(pid, src, &data, sizeof(data));
	
	printf("drm_srvkm_init: %u (", data.init_module);

	switch (data.init_module)
	{
#define X(name) case PVR_SRVKM_##name: printf( #name ")\n"); break;
		X(SERVICES_INIT)
		X(SYNC_INIT)
		X(SYNC_EXP_FENCE_INIT)
		X(SERVICES_PAGE_MIGRATE_INIT)
#undef X
		default:
		printf("unknown)\n");
	}
}

static void dump_hex(char *data, int size)
{
	int di = 0;
	while (di < size)
	{
		for (int j = 0; j < 16; j++)
		{
			printf(" %.2X", data[di++]);
			if (di >= size)
			break;
		}
		printf("\n");
	}
}

bool print_pvr_srv_cmd_data(int pid, struct drm_srvkm_cmd *cmd)
{
#define VALIDATE_SIZES(name) \
	if (cmd->in_data_size != sizeof(din)) { \
		fprintf(stderr, #name "_cmd size missmatch %u %zu\n", cmd->in_data_size, sizeof(din)); \
		return false; \
	} \
	if (cmd->out_data_size != sizeof(dout)) { \
		fprintf(stderr, #name "_ret size missmatch %u %zu\n", cmd->out_data_size, sizeof(dout)); \
		return false; \
	}
	
	if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_CONNECT)
	{
		struct pvr_srv_bridge_connect_cmd din = {0};
		struct pvr_srv_bridge_connect_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_bridge_connect_pmr);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));
		
		printf("pvr_srv_bridge_connect: build_options %X DDK_build %d DDK_version %X flags %X\n",
			din.build_options, din.DDK_build, din.DDK_version, din.flags);
		printf(" out: bvnc %lX error %d capability_flags %X kernel_arch %X\n",
			dout.bvnc, dout.error, dout.capability_flags, dout.kernel_arch);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PHYSMEMNEWRAMBACKEDPMR)
	{
		struct pvr_srv_physmem_new_ram_backed_pmr_cmd din = {0};
		struct pvr_srv_physmem_new_ram_backed_pmr_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_physmem_new_ram_backed_pmr);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		char annotation[2048] = {0};
		memcpy_from_trace(pid, (__u64)din.annotation, annotation, i_min(din.annotation_size, sizeof(annotation) - 1));

		printf("pvr_srv_physmem_new_ram_backed_pmr:\n size %lu mapping_table %p\n annotation %p \"%s\" size %u\n"
			" log2_page_size %u phy_blocks %u virt_blocks %u pdump_flags %X pid %u flags %lX\n",
			din.size,din.mapping_table,din.annotation, annotation,din.annotation_size,din.log2_page_size,
			din.phy_blocks,din.virt_blocks,din.pdump_flags,din.pid,din.flags);
		printf(" out: pmr %p error %d out_flags %lX\n",
			dout.pmr, dout.error, dout.out_flags);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTMAPPMR)
	{
		struct pvr_srv_devmem_int_map_pmr_cmd din = {0};
		struct pvr_srv_devmem_int_map_pmr_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_devmem_int_map_pmr);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		printf("pvr_srv_devmem_int_map_pmr:\n pmr %p reservation %p\n", din.pmr, din.reservation);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTRESERVERANGE)
	{
		struct pvr_srv_devmem_int_reserve_range_cmd din = {0};
		struct pvr_srv_devmem_int_reserve_range_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_devmem_int_reserve_range);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		printf("pvr_srv_devmem_int_reserve_range:\n address %p size %zu serverHeap %p flags %lX\n",
			(void*)din.addr.addr,din.size,din.server_heap,din.flags);
		printf(" out: reservation %p rrror %d\n",
			dout.reservation, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTRESERVERANGEANDMAPPMR)
	{
		struct PVRSRV_BRIDGE_IN_DEVMEMINTRESERVERANGEANDMAPPMR din = {0};
		struct PVRSRV_BRIDGE_OUT_DEVMEMINTRESERVERANGEANDMAPPMR dout = {0};
		VALIDATE_SIZES(DevmemIntReserveRangeAndMapPMR);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		printf("DevmemIntReserveRangeAndMapPMR:\n address %p size %zu serverHeap %p pmr %p flags %lX\n",
			(void*)din.sAddress.addr,din.uiLength,din.hDevmemServerHeap,din.hPMR,din.uiFlags);
		printf(" out: reservation %p error %d\n",
			dout.hReservation, dout.eError);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTA3D && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEHWRTDATASET)
	{
		printf("\n!!! PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEHWRTDATASET\n\n");
		struct pvr_srv_rgx_create_hwrt_dataset_cmd din = {0};
		struct pvr_srv_rgx_create_hwrt_dataset_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_rgx_create_hwrt_dataset);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		printf("pvr_srv_rgx_create_hwrt_dataset_cmd: flipped_multi_sample_ctl %lX multi_sample_ctl %lX hwrt_dataset %p\n"
			" isp_merge: lower_x %u lower_y %u scale_x %u scale_y %u upper_x %u upper_y %u\n"
			" isp_mtile_size %u mtile_stride %u ppp_screen %u rgn_header_size %u te_aa %u te_mtile1 %u te_mtile2 %u te_screen %u\n"
			" tpc_size %u tpc_stride %u max_rts %u\n",
			din.flipped_multi_sample_ctl, din.multi_sample_ctl, din.hwrt_dataset,
			din.isp_merge_lower_x, din.isp_merge_lower_y, din.isp_merge_scale_x, din.isp_merge_scale_y, din.isp_merge_upper_x, din.isp_merge_upper_y,
			din.isp_mtile_size, din.mtile_stride, din.ppp_screen, din.rgn_header_size, din.te_aa, din.te_mtile1, din.te_mtile2, din.te_screen,
			din.tpc_size, din.tpc_stride, din.max_rts);

		printf(" macrotile_array_dev_addrs %p:\n", din.macrotile_array_dev_addrs);
		pvr_dev_addr_t macrotile[ROGUE_FWIF_NUM_RTDATAS];
		memcpy_from_trace(pid, (__u64)din.macrotile_array_dev_addrs, &macrotile, sizeof(macrotile));
		for (int i = 0; i < ROGUE_FWIF_NUM_RTDATAS; i++)
			printf("  [%d]: %p\n", i, (void*)macrotile[i].addr);
		
		printf(" pm_mlist_dev_addrs %p:\n", din.pm_mlist_dev_addrs);
		pvr_dev_addr_t pm_mlist[ROGUE_FWIF_NUM_RTDATAS];
		memcpy_from_trace(pid, (__u64)din.pm_mlist_dev_addrs, &pm_mlist, sizeof(pm_mlist));
		for (int i = 0; i < ROGUE_FWIF_NUM_RTDATAS; i++)
			printf("  [%d]: %p\n", i, (void*)pm_mlist[i].addr);
		
		printf(" rtc_dev_addrs %p:\n", din.rtc_dev_addrs);
		pvr_dev_addr_t rtc[ROGUE_FWIF_NUM_GEOMDATAS];
		memcpy_from_trace(pid, (__u64)din.rtc_dev_addrs, &rtc, sizeof(rtc));
		for (int i = 0; i < ROGUE_FWIF_NUM_GEOMDATAS; i++)
			printf("  [%d]: %p\n", i, (void*)rtc[i].addr);

		printf(" rgn_header_dev_addrs %p:\n", din.rgn_header_dev_addrs);
		pvr_dev_addr_t rgn_header[ROGUE_FWIF_NUM_RTDATAS];
		memcpy_from_trace(pid, (__u64)din.rgn_header_dev_addrs, &rgn_header, sizeof(rgn_header));
		for (int i = 0; i < ROGUE_FWIF_NUM_RTDATAS; i++)
			printf("  [%d]: %p\n", i, (void*)rgn_header[i].addr);
		
		printf(" tail_ptrs_dev_addrs %p:\n", din.tail_ptrs_dev_addrs);
		pvr_dev_addr_t tail_ptrs[ROGUE_FWIF_NUM_GEOMDATAS];
		memcpy_from_trace(pid, (__u64)din.tail_ptrs_dev_addrs, &tail_ptrs, sizeof(tail_ptrs));
		for (int i = 0; i < ROGUE_FWIF_NUM_GEOMDATAS; i++)
			printf("  [%d]: %p\n", i, (void*)tail_ptrs[i].addr);
			
		printf(" vheap_table_dev_adds %p:\n", din.vheap_table_dev_adds);
		pvr_dev_addr_t vheap_table[ROGUE_FWIF_NUM_GEOMDATAS];
		memcpy_from_trace(pid, (__u64)din.vheap_table_dev_adds, &vheap_table, sizeof(vheap_table));
		for (int i = 0; i < ROGUE_FWIF_NUM_GEOMDATAS; i++)
			printf("  [%d]: %p\n", i, (void*)vheap_table[i].addr);

		printf(" free_lists %p:\n", din.free_lists);
		void *freeLists[ROGUE_FWIF_NUM_RTDATA_FREELISTS];
		memcpy_from_trace(pid, (__u64)din.free_lists, &freeLists, sizeof(freeLists));
		for (int i = 0; i < ROGUE_FWIF_NUM_RTDATA_FREELISTS; i++)
			printf("  [%d]: %p\n", i, freeLists[i]);
		
		printf(" out: error %d\n", dout.error);

		printf(" hwrt_dataset %p:\n", dout.hwrt_dataset);
		void *hwrt_dataset[ROGUE_FWIF_NUM_RTDATAS];
		memcpy_from_trace(pid, (__u64)dout.hwrt_dataset, &hwrt_dataset, sizeof(hwrt_dataset));
		for (int i = 0; i < ROGUE_FWIF_NUM_RTDATAS; i++)
			printf("  [%d]: %p\n", i, hwrt_dataset[i]);
	}
	else if(cmd->bridge_id == PVR_SRV_BRIDGE_RGXTA3D && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTA3D_RGXKICKTA3D2)
	{
		struct pvr_srv_rgx_kick_ta3d2_cmd din = {0};
		struct pvr_srv_rgx_kick_ta3d2_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_rgx_kick_ta3d2);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		printf("pvr_srv_rgx_kick_ta3d2: deadline %ld hw_rt_dataset %p msaa_scratch_buffer %p pr_fence_ufo_sync_prim_block %p render_ctx %p zs_buffer %p\n"
		" client_3d_update_sync_offset %p client_3d_update_value %p client_ta_fence_sync_offset %p client_ta_fence_value %p\n"
		" client_ta_update_sync_offset %p client_ta_update_value %p sync_pmr_flags %p cmd_3d %p cmd_3d_pr %p cmd_ta %p\n"
		" update_fence_name %p update_fence_name_3d %p client_3d_update_sync_prim_block %p client_ta_fence_sync_prim_block %p\n"
		" client_ta_update_sync_prim_block %p sync_pmrs %p\n"
		" check_fence %d check_fence_3d %d update_timeline %d update_timeline_3d %d\n"
		" cmd_3d_size %u cmd_3d_pr_size %u client_3d_update_count %u client_ta_fence_count %u client_ta_update_count %u\n"
		" ext_job_ref %u num_draw_calls %u num_indices %u num_mrts %u pdump_flags %u client_pr_fence_ufo_sync_offset %u\n"
		" client_pr_fence_value %u render_target_size %u sync_pmr_count %u cmd_ta_size %u\n"
		" abort %d kick_3d %d kick_pr %d kick_ta %d\n",
			din.deadline, din.hw_rt_dataset, din.msaa_scratch_buffer, din.pr_fence_ufo_sync_prim_block, din.render_ctx, din.zs_buffer,
			din.client_3d_update_sync_offset, din.client_3d_update_value, din.client_ta_fence_sync_offset, din.client_ta_fence_value,
			din.client_ta_update_sync_offset, din.client_ta_update_value, din.sync_pmr_flags, din.cmd_3d, din.cmd_3d_pr, din.cmd_ta,
			din.update_fence_name, din.update_fence_name_3d, din.client_3d_update_sync_prim_block, din.client_ta_fence_sync_prim_block,
			din.client_ta_update_sync_prim_block, din.sync_pmrs,
			din.check_fence, din.check_fence_3d, din.update_timeline, din.update_timeline_3d,
			din.cmd_3d_size, din.cmd_3d_pr_size, din.client_3d_update_count, din.client_ta_fence_count, din.client_ta_update_count,
			din.ext_job_ref, din.num_draw_calls, din.num_indices, din.num_mrts, din.pdump_flags, din.client_pr_fence_ufo_sync_offset,
			din.client_pr_fence_value, din.render_target_size, din.sync_pmr_count, din.cmd_ta_size,
			din.abort, din.kick_3d, din.kick_pr, din.kick_ta);

		char buff[512];
		if (din.cmd_3d_size && din.cmd_3d)
		{
			int sz = i_min(din.cmd_3d_size, sizeof(buff));
			memcpy_from_trace(pid, (__u64)din.cmd_3d, buff, sz);
			printf(" cmd_3d:\n");
			dump_hex(buff, sz);
		}
		if (din.cmd_ta_size && din.cmd_ta)
		{
			int sz = i_min(din.cmd_ta_size, sizeof(buff));
			memcpy_from_trace(pid, (__u64)din.cmd_ta, buff, sz);
			printf(" cmd_ta:\n");
			dump_hex(buff, sz);
		}

		printf(" out: error %d update_fence %d update_fence_3d %d\n", dout.error, dout.update_fence, dout.update_fence_3d);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTA3D && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEFREELIST)
	{
		struct pvr_srv_rgx_create_free_list_cmd din = {0};
		struct pvr_srv_rgx_create_free_list_ret dout = {0};
		VALIDATE_SIZES(pvr_srv_rgx_create_free_list);
		memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din));
		memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

		printf("rgx_create_free_list:\n free_list_reservation %p\n mem_ctx_priv_data %p\n global_free_list %p\n"
			" grow_free_list_pages %u\n grow_param_threshold %u\n init_free_list_pages %u\n max_free_list_pages %u\n free_list_check %d\n",
			din.free_list_reservation, din.mem_ctx_priv_data, din.global_free_list, din.grow_free_list_pages,
			din.grow_param_threshold, din.init_free_list_pages, din.max_free_list_pages, din.free_list_check);
		printf(" out: cleanup_cookie %p error %d\n",
			dout.cleanup_cookie, dout.error);
	}
	else
	{
		return false;
	}
#undef VALIDATE_SIZES
	return true;
}

void print_pvrsrv_cmd(int pid, __u64 src)
{
	struct drm_srvkm_cmd cmd = {0};
	memcpy_from_trace(pid, src, &cmd, sizeof(cmd));
	
	if (!print_pvr_srv_cmd_data(pid, &cmd))
	{
		printf("drm_srvkm_cmd: bridge_id %u(%s) func_id %u(%s) in data %p out data %p in size %u out size %u\n",
			cmd.bridge_id, srv_bridge_id_to_str(cmd.bridge_id), cmd.bridge_func_id, srv_bridge_func_to_str(cmd.bridge_id,cmd.bridge_func_id),
			(void*)cmd.in_data_ptr, (void*)cmd.out_data_ptr, cmd.in_data_size, cmd.out_data_size);
	}
}

void print_syscall(struct ptrace_syscall_info *sci, struct ptrace_syscall_info *sci_exit, int pid, const char *open_path)
{
	if (!(sci->entry.nr == SYS_openat || sci->entry.nr == SYS_close || sci->entry.nr == SYS_ioctl))
		return;

	if (sci->entry.nr == SYS_openat && sci_exit->exit.is_error)
		return;

	if (sci->entry.nr == SYS_ioctl && sci->entry.args[1] == DRM_IOCTL_SRVKM_CMD && !sci_exit->exit.is_error)
	{
		printf("ioctl(%lld) ", sci->entry.args[0]);
		print_pvrsrv_cmd(pid, sci->entry.args[2]);
		return;
	}

	if (sci->entry.nr == SYS_openat)
	{
		printf(" openat(%lld, \"%s\", %llX)", sci->entry.args[0], open_path, sci->entry.args[2]);
	}
	else if (sci->entry.nr == SYS_close)
	{
		printf(" close(%lld)", sci->entry.args[0]);
	}
	else if (sci->entry.nr == SYS_ioctl)
	{
		__u64 c = sci->entry.args[1];
		if (_IOC_TYPE(c) != DRM_IOCTL_BASE)
		{
			printf(" ioctl(%lld, %llX(not drm), 0x%llX)",sci->entry.args[0], c, sci->entry.args[2]);
		}
		else
		{
			const char *name = drm_ioctl_to_str(c);
			if (name)
				printf(" ioctl(%lld, DRM %s, 0x%llX)", sci->entry.args[0], name, sci->entry.args[2]);
			else
			{
				int dir = _IOC_DIR(c);
				const char *dir_str = "?";
				if (dir == _IOC_NONE)
					dir_str = "-";
				else if (dir == _IOC_WRITE)
					dir_str = "W";
				else if (dir == _IOC_READ)
					dir_str = "R";
				else if (dir == (_IOC_READ|_IOC_WRITE))
					dir_str = "RW";
				printf(" ioctl(%lld, %llX(%s %X %d), 0x%llX)",sci->entry.args[0], c, dir_str, (int)_IOC_NR(c), (int)_IOC_SIZE(c), sci->entry.args[2]);
			}
		}
	}
	else
	{
		printf("sycall %lld", sci->entry.nr);
		printf("(");
		constexpr int used_args = 6;
		for (int i = 0; i < used_args; i++)
			printf("%lld%s", sci->entry.args[i], i == used_args - 1 ? "" : ", ");
		printf(")");
	}

	printf(" = %llX%s\n", sci_exit->exit.rval, sci_exit->exit.is_error ? " ERROR" : "");
	
	if (sci->entry.nr == SYS_ioctl)
	{
		switch (sci->entry.args[1])
		{
		case DRM_IOCTL_VERSION:
			print_drm_version(pid, sci->entry.args[2]);
		break;
		case DRM_IOCTL_PVR_SRVKM_INIT:
		case DRM_IOCTL_SRVKM_INIT:
			print_pvrsrv_init(pid, sci->entry.args[2]);
			break;
		case DRM_IOCTL_SRVKM_CMD:
			print_pvrsrv_cmd(pid, sci->entry.args[2]);
		break;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: trace_test <file> [args]\n");
		exit(0);
	}

	//vla
	char *chargs[argc];
	for (int i = 0; i < argc - 1; i++)
	{
		chargs[i] = argv[i + 1];
	}
	chargs[argc - 1] = NULL;
	
	pid_t child = fork();
	if (child == 0)
	{
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execvp(chargs[0], chargs);
		exit(0);
	}

	waitpid(child, NULL, 0);
	ptrace(PTRACE_SETOPTIONS, child, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
	ptrace(PTRACE_SYSCALL, child, NULL, NULL);

	constexpr int dri_fds_count = 256;
	int dri_fds[dri_fds_count] = {0};

	int status = 0;
	while (waitpid(child, &status, 0) && ! WIFEXITED(status))
	{
	#if 0
		struct user_regs_struct regs;
		ptrace(PTRACE_GETREGSET, child, NULL, &regs);
		printf("system call %d from %d\n", regs.orig_rax, child);
	#endif
	#if 0
		struct iovec regs;
		ptrace(PTRACE_GETREGSET, child, NT_PRSTATUS, &regs);
		printf("ptrace(PTRACE_GETREGSET) len %zu\n", regs.iov_len);
	#endif
		struct ptrace_syscall_info sci;
		long r = ptrace(PTRACE_GET_SYSCALL_INFO, child, sizeof(sci), &sci);
		if (sci.op != PTRACE_SYSCALL_INFO_ENTRY)
		{
			fprintf(stderr, "%ld ptrace(PTRACE_GET_SYSCALL_INFO, %d, %zu) op %d, expected op %d\n", r, child, sizeof(sci), sci.op, PTRACE_SYSCALL_INFO_ENTRY);
			break;
		}

		char open_path[2048] = {0};
		if (sci.entry.nr == SYS_openat)
			strcpy_from_trace(child, sci.entry.args[1], open_path, sizeof(open_path));

		ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		
		if (!waitpid(child, &status, 0))
			break;
		if (WIFEXITED(status))
			break;
		
		struct ptrace_syscall_info sci_exit;
		r = ptrace(PTRACE_GET_SYSCALL_INFO, child, sizeof(sci_exit), &sci_exit);
		if (sci_exit.op != PTRACE_SYSCALL_INFO_EXIT)
		{
			fprintf(stderr, "%ld ptrace(PTRACE_GET_SYSCALL_INFO, %d, %zu) op %d, expected op %d\n", r, child, sizeof(sci_exit), sci_exit.op, PTRACE_SYSCALL_INFO_EXIT);
			break;
		}

		if (sci.entry.nr != SYS_ioctl || find_fd(dri_fds, dri_fds_count, sci.entry.args[0]) != -1)
			print_syscall(&sci, &sci_exit, child, open_path);
		else if(sci.entry.nr == SYS_ioctl)
			printf("skipped non dri ioctl\n");

		if (sci.entry.nr == SYS_openat && !sci_exit.exit.is_error)
		{
			if (!strncmp(open_path, "/dev/dri/renderD128", sizeof(open_path))
				|| !strncmp(open_path, "/dev/dri/card1", sizeof(open_path)))
			{
				printf("!!! open dri device = %lld\n", sci_exit.exit.rval);
				insert_new_fd(dri_fds, dri_fds_count, sci_exit.exit.rval);
			}
		}
		else if (sci.entry.nr == SYS_close)
		{
			if (remove_fd(dri_fds, dri_fds_count, sci.entry.args[0]) != -1)
				printf("!! closed dri device %lld\n", sci.entry.args[0]);
		}

		ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	}

	exit(0);
}

