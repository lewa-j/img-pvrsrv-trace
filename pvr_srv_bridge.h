//lewa_j 2026
#pragma once
// based on mesa pvr_srv_bridge.h
// and img-rogue common_*_bridge.h

#include <stdint.h>
#include <stddef.h>

// from mesa util/macros.h
#define PACKED __attribute__((__packed__))
#define BITFIELD_BIT(b) (1u << (b))

#define PVR_SRV_VERSION_MAJ 24U
#define PVR_SRV_VERSION_MIN 2U
#define PVR_SRV_VERSION_BUILD 6603887

#define PVR_SRV_VERSION                                              \
	(((uint32_t)((uint32_t)(PVR_SRV_VERSION_MAJ) & 0xFFFFU) << 16U) |\
	(((PVR_SRV_VERSION_MIN) & 0xFFFFU) << 0U))


#define PVR_SRV_BRIDGE_SRVCORE 1UL

#define PVR_SRV_BRIDGE_SRVCORE_CONNECT 0UL
#define PVR_SRV_BRIDGE_SRVCORE_DISCONNECT 1UL
#define PVR_SRV_BRIDGE_SRVCORE_ACQUIREGLOBALEVENTOBJECT 2UL
#define PVR_SRV_BRIDGE_SRVCORE_RELEASEGLOBALEVENTOBJECT 3UL
#define PVR_SRV_BRIDGE_SRVCORE_EVENTOBJECTOPEN 4UL
#define PVR_SRV_BRIDGE_SRVCORE_EVENTOBJECTCLOSE 6UL
#define PVR_SRV_BRIDGE_SRVCORE_GETDEVCLOCKSPEED 8UL
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
#define PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGCOUNT 21UL
#define PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCOUNT 22UL
#define PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGNAME 23UL
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
#define PVR_SRV_BRIDGE_RGXTQ_RGXSUBMITTRANSFER2 3UL
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



#define ROGUE_FWIF_NUM_RTDATAS 4U
#define ROGUE_FWIF_NUM_GEOMDATAS 4U
#define ROGUE_FWIF_NUM_RTDATA_FREELISTS 12U


typedef enum pvr_srv_error_e
{
	PVR_SRV_OK,
	PVR_SRV_ERROR_RETRY = 25,
	PVR_SRV_ERROR_DDK_VERSION_MISMATCH = 26,
	PVR_SRV_ERROR_DDK_BUILD_MISMATCH = 27,
	PVR_SRV_ERROR_BUILD_OPTIONS_MISMATCH = 28,
	PVR_SRV_ERROR_BRIDGE_CALL_FAILED = 37,
	PVR_SRV_ERROR_HANDLE_INDEX_OUT_OF_RANGE = 203,
	PVR_SRV_ERROR_BRIDGE_ARRAY_SIZE_TOO_BIG = 350,
	PVR_SRV_ERROR_FORCE_I32 = 0x7fffffff
} pvr_srv_error;

typedef struct pvr_dev_addr
{
	uint64_t addr;
} pvr_dev_addr_t;

typedef void * pvr_handle_t;

// SRVCORE

struct pvr_srv_bridge_connect_cmd
{
	uint32_t build_options;
	uint32_t DDK_build;
	uint32_t DDK_version;
	uint32_t flags;
} PACKED;

struct pvr_srv_bridge_connect_ret
{
	uint64_t bvnc;
	pvr_srv_error error;
	uint32_t capability_flags;
	uint8_t kernel_arch;
} PACKED;


struct pvr_srv_bridge_disconnect_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_acquireglobaleventobject_ret
{
	pvr_handle_t global_event_object;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_releaseglobaleventobject_cmd
{
	pvr_handle_t global_event_object;
} PACKED;

struct pvr_srv_bridge_releaseglobaleventobject_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_eventobjectopen_cmd
{
	pvr_handle_t event_object;
} PACKED;

struct pvr_srv_bridge_eventobjectopen_ret
{
	pvr_handle_t os_event;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_eventobjectclose_cmd
{
	pvr_handle_t os_event_km;
} PACKED;

struct pvr_srv_bridge_eventobjectclose_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_getdevclockspeed_ret
{
	pvr_srv_error error;
	uint32_t clock_speed;
} PACKED;


struct pvr_srv_bridge_alignmentcheck_cmd
{
	uint32_t *align_checks;
	uint32_t align_checks_size;
} PACKED;

struct pvr_srv_bridge_alignmentcheck_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_getmulticoreinfo_cmd
{
	uint64_t *caps;
	uint32_t caps_size;
} PACKED;

struct pvr_srv_bridge_getmulticoreinfo_ret
{
	uint64_t *caps;
	pvr_srv_error error;
	uint32_t num_cores;
} PACKED;


struct pvr_srv_bridge_acquireinfopage_ret
{
	pvr_handle_t pmr;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_releaseinfopage_cmd
{
	pvr_handle_t pmr;
} PACKED;

struct pvr_srv_bridge_releaseinfopage_ret
{
	pvr_srv_error error;
} PACKED;

// SYNC

struct pvr_srv_bridge_alloc_sync_primitive_block_ret
{
	pvr_handle_t handle;
	pvr_handle_t pmr;
	pvr_srv_error error;
	uint32_t size;
	uint32_t addr;
} PACKED;


struct pvr_srv_bridge_free_sync_primitive_block_cmd
{
	pvr_handle_t handle;
} PACKED;

struct pvr_srv_bridge_free_sync_primitive_block_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_sync_prim_set_cmd
{
	pvr_handle_t handle;
	uint32_t index;
	uint32_t value;
} PACKED;

struct pvr_srv_bridge_sync_prim_set_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_sync_allocevent_cmd
{
	const char *class_name;
	uint32_t class_name_size;
	uint32_t fw_addr;
	bool server_sync;
} PACKED;

struct pvr_srv_bridge_sync_allocevent_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_sync_freeevent_cmd
{
	uint32_t fw_addr;
} PACKED;

struct pvr_srv_bridge_sync_freeevent_ret
{
	pvr_srv_error error;
} PACKED;

// MM

struct pvr_srv_pmr_makelocalimporthandle_cmd
{
	pvr_handle_t buffer;
} PACKED;

struct pvr_srv_pmr_makelocalimporthandle_ret
{
	pvr_handle_t ext_mem;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_pmr_unmakelocalimporthandle_cmd
{
	pvr_handle_t ext_mem;
} PACKED;

struct pvr_srv_pmr_unmakelocalimporthandle_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_pmr_localimportpmr_cmd
{
	pvr_handle_t ext_handle;
} PACKED;

struct pvr_srv_pmr_localimportpmr_ret
{
	uint64_t align;
	uint64_t size;
	pvr_handle_t pmr;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_pmr_unref_pmr_cmd
{
	pvr_handle_t pmr;
} PACKED;

struct pvr_srv_pmr_unref_pmr_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_ctx_create_cmd
{
	bool kernel_memory_ctx;
} PACKED;

struct pvr_srv_devmem_int_ctx_create_ret
{
	pvr_handle_t server_memctx;
	pvr_handle_t server_memctx_data;
	pvr_srv_error error;
	uint32_t cpu_cache_line_size;
} PACKED;


struct pvr_srv_devmem_int_ctx_destroy_cmd
{
	void *server_memctx;
} PACKED;

struct pvr_srv_devmem_int_ctx_destroy_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_heap_cfg_count_ret
{
	pvr_srv_error error;
	uint32_t heap_config_count;
} PACKED;


struct pvr_srv_heap_count_cmd
{
	uint32_t heap_config_index;
} PACKED;

struct pvr_srv_heap_count_ret
{
	pvr_srv_error error;
	uint32_t heap_count;
} PACKED;


struct pvr_srv_heap_cfg_name_cmd
{
	char *config_name_buffer;
	uint32_t heap_config_index;
	uint32_t config_name_bufer_size;
} PACKED;

struct pvr_srv_heap_cfg_name_ret
{
	char *heap_config_name;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_heap_cfg_details_cmd
{
	char *buffer;
	uint32_t heap_config_index;
	uint32_t heap_index;
	uint32_t buffer_size;
} PACKED;

struct pvr_srv_heap_cfg_details_ret
{
	pvr_dev_addr_t base_addr;
	uint64_t size;
	uint64_t reserved_size;
	char *buffer;
	pvr_srv_error error;
	uint32_t log2_page_size;
	uint32_t log2_alignment;
} PACKED;


enum pvr_phys_heap
{
	PVR_SRV_PHYS_HEAP_FORCE_I32 = 0x7fffffff
};

enum pvr_phys_heap_type
{
	PVR_SRV_PHYS_HEAP_TYPE_UNKNOWN
};

struct pvr_phys_heap_mem_stats
{
	uint64_t total_size;
	uint64_t free_size;
	uint32_t phys_heap_flags;
	enum pvr_phys_heap_type phys_heap_type;
};

struct pvr_srv_physheap_getmeminfo_cmd
{
	struct pvr_phys_heap_mem_stats *phys_heap_mem_stats;
	enum pvr_phys_heap *phys_heap_id;
	uint32_t phys_heap_count;
} PACKED;

struct pvr_srv_physheap_getmeminfo_ret
{
	struct pvr_phys_heap_mem_stats *phys_heap_mem_stats;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_heap_create_cmd
{
	pvr_handle_t server_memctx;
	uint32_t heap_config_index;
	uint32_t heap_index;
} PACKED;

struct pvr_srv_devmem_int_heap_create_ret
{
	pvr_handle_t server_heap;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_heap_destroy_cmd
{
	pvr_handle_t server_heap;
} PACKED;

struct pvr_srv_devmem_int_heap_destroy_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_physmem_new_ram_backed_pmr_cmd
{
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

struct pvr_srv_physmem_new_ram_backed_pmr_ret
{
	pvr_handle_t pmr;
	pvr_srv_error error;
	uint64_t out_flags;
} PACKED;


struct pvr_srv_devmem_int_map_pmr_cmd
{
	pvr_handle_t pmr;
	pvr_handle_t reservation;
} PACKED;

struct pvr_srv_devmem_int_map_pmr_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_reserve_range_cmd
{
	pvr_dev_addr_t addr;
	uint64_t size;
	pvr_handle_t server_heap;
	uint64_t flags;
} PACKED;

struct pvr_srv_devmem_int_reserve_range_ret
{
	pvr_handle_t reservation;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_reserve_range_and_map_pmr_cmd
{
	pvr_dev_addr_t address;
	size_t length;
	pvr_handle_t server_heap;
	pvr_handle_t pmr;
	uint64_t flags;
} PACKED;

struct pvr_srv_devmem_int_reserve_range_and_map_pmr_ret
{
	pvr_handle_t reservation;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_unreserve_range_cmd
{
	pvr_handle_t reservation;
} PACKED;

struct pvr_srv_devmem_int_unreserve_range_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_x_int_reserve_range_cmd
{
	pvr_dev_addr_t address;
	size_t length;
	pvr_handle_t server_heap;
} PACKED;

struct pvr_srv_devmem_x_int_reserve_range_ret
{
	pvr_handle_t reservation;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_x_int_unreserve_range_cmd
{
	pvr_handle_t reservation;
} PACKED;

struct pvr_srv_devmem_x_int_unreserve_range_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_x_int_map_pages_cmd
{
	pvr_handle_t pmr;
	pvr_handle_t reservation;
	uint32_t page_count;
	uint32_t phys_page_offset;
	uint32_t virt_page_offset;
	uint64_t flags;
} PACKED;

struct pvr_srv_devmem_x_int_map_pages_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_x_int_unmap_pages_cmd
{
	pvr_handle_t reservation;
	uint32_t page_count;
	uint32_t virt_page_offset;
} PACKED;

struct pvr_srv_devmem_x_int_unmap_pages_ret
{
	pvr_srv_error error;
} PACKED;

// RGXTQ

struct pvr_srv_rgx_create_transfer_context_cmd
{
	uint64_t robustness_address;
	pvr_handle_t priv_data;
	uint8_t *reset_framework_cmd;
	int32_t priority;
	uint32_t context_flags;
	uint32_t reset_framework_cmd_size;
	uint32_t packed_ccb_size_u8888;
} PACKED;

struct pvr_srv_rgx_create_transfer_context_ret
{
	pvr_handle_t transfer_context;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_destroy_transfer_context_cmd
{
	pvr_handle_t transfer_context;
} PACKED;

struct pvr_srv_rgx_destroy_transfer_context_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_submit_transfer2_cmd
{
	pvr_handle_t transfer_context;
	uint32_t *client_update_count;
	uint32_t *cmd_size;
	uint32_t *sync_pmr_flags;
	uint32_t *tq_prepare_flags;
	uint32_t **update_sync_offset;
	uint32_t **update_value;
	uint8_t **fw_command;
	char *update_fence_name;
	void **sync_pmrs;
	void ***update_ufo_sync_prim_block;
	int32_t update_timeline_2d;
	int32_t update_timeline_3d;
	int32_t check_fence;
	uint32_t ext_job_ref;
	uint32_t prepare_count;
	uint32_t sync_pmr_count;
} PACKED;

struct pvr_srv_rgx_submit_transfer2_ret
{
	pvr_srv_error error;
	int32_t update_fence_2d;
	int32_t update_fence_3d;
} PACKED;


struct pvr_srv_rgxtq_getsharedmemory_ret
{
	pvr_handle_t cli_pmr_mem;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgxtq_releasesharedmemory_cmd
{
	pvr_handle_t pmr_mem;
} PACKED;

struct pvr_srv_rgxtq_releasesharedmemory_ret
{
	pvr_srv_error error;
} PACKED;

// RGXCMP

struct pvr_srv_rgx_create_compute_context_cmd
{
	uint64_t robustness_address;
	pvr_handle_t priv_data;
	uint8_t *reset_framework_cmd;
	uint8_t *static_compute_context_state;
	int32_t priority;
	uint32_t context_flags;
	uint32_t reset_framework_cmd_size;
	uint32_t max_deadline_ms;
	uint32_t packed_ccb_size;
	uint32_t static_compute_context_state_size;
} PACKED;

struct pvr_srv_rgx_create_compute_context_ret
{
	pvr_handle_t compute_context;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_destroy_compute_context_cmd
{
	pvr_handle_t compute_context;
} PACKED;

struct pvr_srv_rgx_destroy_compute_context_ret
{
	pvr_srv_error error;
} PACKED;

// RGXTA3D

struct pvr_srv_rgx_create_free_list_cmd
{
	pvr_handle_t free_list_reservation;
	pvr_handle_t mem_ctx_priv_data;
	pvr_handle_t global_free_list;
	uint32_t grow_free_list_pages;
	uint32_t grow_param_threshold;
	uint32_t init_free_list_pages;
	uint32_t max_free_list_pages;
	bool free_list_check;
} PACKED;

struct pvr_srv_rgx_create_free_list_ret
{
	pvr_handle_t cleanup_cookie;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_destroy_free_list_cmd
{
	pvr_handle_t cleanup_cookie;
} PACKED;

struct pvr_srv_rgx_destroy_free_list_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_create_hwrt_dataset_cmd
{
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
	pvr_handle_t *hwrt_dataset;
	/* ROGUE_FWIF_NUM_RTDATA_FREELISTS size array of handles. */
	pvr_handle_t *free_lists;
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
} PACKED;

struct pvr_srv_rgx_create_hwrt_dataset_ret
{
	/* ROGUE_FWIF_NUM_RTDATAS sized array of handles. */
	pvr_handle_t *hwrt_dataset;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_destroy_hwrt_dataset_cmd
{
	pvr_handle_t hwrt_dataset;
} PACKED;

struct pvr_srv_rgx_destroy_hwrt_dataset_ret
{
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_kick_ta3d2_cmd
{
	uint64_t deadline;
	pvr_handle_t hw_rt_dataset;
	void *msaa_scratch_buffer;
	void *pr_fence_ufo_sync_prim_block;
	pvr_handle_t render_ctx;
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

struct pvr_srv_rgx_kick_ta3d2_ret
{
	pvr_srv_error error;
	int32_t update_fence;
	int32_t update_fence_3d;
} PACKED;


struct pvr_srv_rgx_create_render_context_cmd
{
	pvr_dev_addr_t vdm_callstack_addr;
	uint64_t robustness_address;
	pvr_handle_t priv_data;
	uint8_t *reset_framework_cmd;
	uint8_t *static_render_context_state;
	int32_t priority;
	uint32_t context_flags;
	uint32_t reset_framework_cmd_size;
	uint32_t max_3d_deadline_ms;
	uint32_t max_ta_deadline_ms;
	uint32_t packed_ccb_size;
	uint32_t static_render_context_state_size;
	uint32_t call_stack_depth;
} PACKED;

struct pvr_srv_rgx_create_render_context_ret
{
	pvr_handle_t render_context;
	pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_destroy_render_context_cmd
{
	pvr_handle_t render_context;
} PACKED;

struct pvr_srv_rgx_destroy_render_context_ret
{
	pvr_srv_error error;
} PACKED;


// client
pvr_srv_error PVRSRVConnect(int fd, uint64_t *packedBvnc, uint32_t *capabilityFlags, uint8_t *kernelArch);
pvr_srv_error PVRSRVDisconnect(int fd);
// EventObjectOpen
// EventObjectClose
pvr_srv_error PVRSRVGetDevClockSpeed(int fd, uint32_t *clock_speed);
pvr_srv_error PVRSRVGetMultiCoreInfo(int fd, uint32_t caps_size, uint32_t *num_cores, uint64_t *caps);
pvr_srv_error PVRSRVAcquireInfoPage(int fd, pvr_handle_t *out_pmr);
pvr_srv_error PVRSRVReleaseInfoPage(int fd, pvr_handle_t pmr);

// PMRMakeLocalImportHandle
// PMRUnmakeLocalImportHandle
pvr_srv_error PVRSRVPMRLocalImportPMR(int fd, pvr_handle_t ext_handle, pvr_handle_t *pmr, uint64_t *size, uint64_t *align);
pvr_srv_error PVRSRVPMRUnrefPMR(int fd, pvr_handle_t pmr);
pvr_srv_error PVRSRVPhysmemNewRamBackedPMR(int fd, uint64_t size, uint32_t num_phys_chunks, uint32_t num_virt_chunks,
	uint32_t *mapping_table, uint32_t log2_page_size, uint64_t flags, uint32_t annotation_length, const char *annotation,
	uint32_t pid, pvr_handle_t *pmr, uint32_t pdump_flags, uint64_t *out_flags);

pvr_srv_error PVRSRVDevmemIntCtxCreate(int fd, bool kernelMemoryCtx,
	pvr_handle_t *devMemServerContext, pvr_handle_t *privData, uint32_t *CPUCacheLineSize);

pvr_srv_error PVRSRVDevmemIntCtxDestroy(int fd, pvr_handle_t devMemServerContext);
pvr_srv_error PVRSRVDevmemIntHeapCreate(int fd, pvr_handle_t devmem_ctx, uint32_t heap_config_index, uint32_t heap_index, pvr_handle_t *devmem_heap);
pvr_srv_error PVRSRVDevmemIntHeapDestroy(int fd, pvr_handle_t devmem_heap);

pvr_srv_error PVRSRVHeapCfgHeapConfigCount(int fd, uint32_t *heap_config_count);
pvr_srv_error PVRSRVHeapCfgHeapCount(int fd, uint32_t heap_config_index, uint32_t *heap_count);
pvr_srv_error PVRSRVHeapCfgHeapConfigName(int fd, uint32_t heap_config_index, uint32_t config_name_size, char *config_name_buffer);
pvr_srv_error PVRSRVHeapCfgHeapDetails(int fd, uint32_t heap_config_index, uint32_t heap_index, uint32_t name_size, char *name_buffer,
	pvr_dev_addr_t *base_addr, uint64_t *heap_size, uint64_t *reserved_size, uint32_t *log2_data_page_size, uint32_t *log2_import_alignment);

//RGXTQGetSharedMemory
//RGXTQReleaseSharedMemory
