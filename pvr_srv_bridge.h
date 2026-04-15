//lewa_j 2026
#pragma once
// based on mesa pvr_srv_bridge.h
// and img-rogue common_*_bridge.h

#include <stdint.h>

// from mesa util/macros.h
#define PACKED __attribute__((__packed__))
#define BITFIELD_BIT(b) (1u << (b))


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


enum pvr_srv_error
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
};

typedef struct pvr_dev_addr
{
	uint64_t addr;
} pvr_dev_addr_t;

typedef void * pvr_handle_t;


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
	enum pvr_srv_error error;
	uint32_t capability_flags;
	uint8_t kernel_arch;
} PACKED;


struct pvr_srv_bridge_disconnect_ret
{
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_acquireglobaleventobject_ret
{
	pvr_handle_t global_event_object;
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_releaseglobaleventobject_cmd
{
	pvr_handle_t global_event_object;
} PACKED;

struct pvr_srv_bridge_releaseglobaleventobject_ret
{
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_getdevclockspeed_ret
{
	enum pvr_srv_error error;
	uint32_t clock_speed;
} PACKED;


struct pvr_srv_bridge_getmulticoreinfo_cmd
{
	uint64_t *caps;
	uint32_t caps_size;
} PACKED;

struct pvr_srv_bridge_getmulticoreinfo_ret
{
	uint64_t *caps;
	enum pvr_srv_error error;
	uint32_t num_cores;
} PACKED;


struct pvr_srv_bridge_acquireinfopage_ret
{
	pvr_handle_t pmr;
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_bridge_releaseinfopage_cmd
{
	pvr_handle_t pmr;
} PACKED;

struct pvr_srv_bridge_releaseinfopage_ret
{
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_pmr_unref_pmr_cmd
{
	void *pmr;
} PACKED;

struct pvr_srv_pmr_unref_pmr_ret
{
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_devmem_int_ctx_create_cmd
{
	bool kernel_memory_ctx;
} PACKED;

struct pvr_srv_devmem_int_ctx_create_ret
{
	pvr_handle_t server_memctx;
	pvr_handle_t server_memctx_data;
	enum pvr_srv_error error;
	uint32_t cpu_cache_line_size;
} PACKED;


struct pvr_srv_devmem_int_ctx_destroy_cmd
{
	void *server_memctx;
} PACKED;

struct pvr_srv_devmem_int_ctx_destroy_ret
{
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_heap_cfg_count_ret
{
	enum pvr_srv_error error;
	uint32_t heap_config_count;
} PACKED;


struct pvr_srv_heap_count_cmd
{
	uint32_t heap_config_index;
} PACKED;

struct pvr_srv_heap_count_ret
{
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
	uint32_t log2_page_size;
	uint32_t log2_alignment;
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
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
	uint64_t out_flags;
} PACKED;


struct pvr_srv_devmem_int_map_pmr_cmd
{
	pvr_handle_t pmr;
	pvr_handle_t reservation;
} PACKED;

struct pvr_srv_devmem_int_map_pmr_ret
{
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
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
	enum pvr_srv_error error;
	int32_t update_fence_2d;
	int32_t update_fence_3d;
} PACKED;

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
	enum pvr_srv_error error;
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
	pvr_handle_t*hwrt_dataset;
	/* ROGUE_FWIF_NUM_RTDATA_FREELISTS size array of handles. */
	pvr_handle_t*free_lists;
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
	pvr_handle_t*hwrt_dataset;
	enum pvr_srv_error error;
} PACKED;


struct pvr_srv_rgx_kick_ta3d2_cmd
{
	uint64_t deadline;
	pvr_handle_t hw_rt_dataset;
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

struct pvr_srv_rgx_kick_ta3d2_ret
{
	enum pvr_srv_error error;
	int32_t update_fence;
	int32_t update_fence_3d;
} PACKED;

