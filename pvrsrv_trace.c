//lewa_j 2025-2026
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
#include <inttypes.h>
#include <xf86drm.h>

#define SUPPORT_LINUX_OSPAGE_MIGRATION
#include "pvr_drm.h"
#include "pvr_srv_bridge.h"

// mesa version is IOWR, but DRM_IOCTL_PVR_SRVKM_INIT is IOW. recognize both
#define DRM_IOCTL_SRVKM_INIT \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_PVR_SRVKM_INIT, struct drm_pvr_srvkm_init_data)

const char *syscall_to_str(int n)
{
	switch (n)
	{
		case SYS_openat: return "openat";
		case SYS_close: return "close";
		case SYS_mmap: return "mmap";
		case SYS_munmap: return "munmap";
		case SYS_ioctl: return "ioctl";
		case SYS_read: return "read";
		case SYS_write: return "write";
		case SYS_fstat: return "fstat";
		case SYS_getpid: return "getpid";
		case SYS_gettid: return "gettid";
		case SYS_socket: return "socket";
		case SYS_bind: return "bind";
		case SYS_listen: return "listen";
		case SYS_setsockopt: return "setsockopt";
		case SYS_mprotect: return "mprotect";
		case SYS_prctl: return "prctl";
		case SYS_brk: return "brk";
		case SYS_rt_sigaction: return "rt_sigaction";
		case SYS_rt_sigprocmask: return "rt_sigprocmask";
		case SYS_clone3: return "clone3";
		case SYS_ppoll: return "ppoll";
		case SYS_futex: return "futex";
	}
	return nullptr;
}

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
				X(GETDEVCLOCKSPEED)
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
				X(RGXSUBMITTRANSFER2)
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
		X(PVR_SRVKM_CMD)
		X(PVR_SYNC_RENAME_CMD)
		X(PVR_SYNC_FORCE_SW_ONLY_CMD)
		X(PVR_SW_SYNC_CREATE_FENCE_CMD)
		X(PVR_SW_SYNC_INC_CMD)
		X(PVR_SRVKM_INIT)
		X(SRVKM_INIT)
#undef X
	}
	return nullptr;
}


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

void print_pvr_sync_rename(int pid, __u64 src)
{
	struct pvr_sync_rename_ioctl_data data = {0};
	memcpy_from_trace(pid, src, &data, sizeof(data));
	printf("pvr_sync_rename: name \"%s\"\n", data.szName);
}

void print_pvr_sw_sync_create_fence(int pid, __u64 src)
{
	struct pvr_sw_sync_create_fence_data data = {0};
	memcpy_from_trace(pid, src, &data, sizeof(data));
	printf("pvr_sw_sync_create_fence: name \"%s\" fence %d pad 0x%X sync_pt_idx 0x%llX\n",
		data.name, data.fence, data.pad, data.sync_pt_idx);
}

void print_pvr_sw_timeline_advance(int pid, __u64 src)
{
	struct pvr_sw_timeline_advance_data data = {0};
	memcpy_from_trace(pid, src, &data, sizeof(data));
	printf("pvr_sw_timeline_advance(PVR_SW_SYNC_INC_CMD): sync_pt_idx 0x%llX\n", data.sync_pt_idx);
}

void print_pvrsrv_init(int pid, __u64 src)
{
	struct drm_pvr_srvkm_init_data data = {0};
	memcpy_from_trace(pid, src, &data, sizeof(data));

	printf("drm_pvr_srvkm_init: %u (", data.init_module);

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

bool print_pvr_srv_cmd_data(int pid, struct drm_pvr_srvkm_cmd *cmd)
{
#define VALIDATE_IN_SIZE(name) \
	if (cmd->in_data_size != sizeof(din)) { \
		fprintf(stderr, #name "_cmd size missmatch %u %zu\n", cmd->in_data_size, sizeof(din)); \
		return false; \
	}

#define VALIDATE_OUT_SIZE(name) \
	if (cmd->out_data_size != sizeof(dout)) { \
		fprintf(stderr, #name "_ret size missmatch %u %zu\n", cmd->out_data_size, sizeof(dout)); \
		return false; \
	}

#define LOAD_CMD_DATA(name) \
	struct name##_cmd din = {0}; \
	struct name##_ret dout = {0}; \
	VALIDATE_IN_SIZE(name) \
	VALIDATE_OUT_SIZE(name) \
	memcpy_from_trace(pid, cmd->in_data_ptr, &din, sizeof(din)); \
	memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

#define LOAD_CMD_OUT_DATA(name) \
	struct name##_ret dout = {0}; \
	VALIDATE_OUT_SIZE(name); \
	memcpy_from_trace(pid, cmd->out_data_ptr, &dout, sizeof(dout));

	if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_CONNECT)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_connect);
		printf("pvr_srv_bridge_connect: build_options 0x%X DDK_build %d DDK_version 0x%X flags 0x%X\n",
			din.build_options, din.DDK_build, din.DDK_version, din.flags);
		printf(" out: bvnc %lX error %d capability_flags 0x%X kernel_arch %dbit\n",
			dout.bvnc, dout.error, dout.capability_flags, dout.kernel_arch);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_DISCONNECT)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_bridge_disconnect);
		printf("pvr_srv_bridge_disconnect: out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_ACQUIREGLOBALEVENTOBJECT)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_bridge_acquireglobaleventobject);
		printf("pvr_srv_bridge_acquireglobaleventobject: out: global_event_object %p error %d\n",
			dout.global_event_object, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_RELEASEGLOBALEVENTOBJECT)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_releaseglobaleventobject);
		printf("pvr_srv_bridge_releaseglobaleventobject: global_event_object %p\n", din.global_event_object);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_EVENTOBJECTOPEN)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_eventobjectopen);
		printf("pvr_srv_bridge_eventobjectopen: event_object %p\n", din.event_object);
		printf(" out: os_event %p error %d\n", dout.os_event, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_EVENTOBJECTCLOSE)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_eventobjectclose);
		printf("pvr_srv_bridge_eventobjectclose: os_event_km %p\n", din.os_event_km);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_GETDEVCLOCKSPEED)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_bridge_getdevclockspeed);
		printf("pvr_srv_bridge_getdevclockspeed: out: error %d clock_speed %d\n", dout.error, dout.clock_speed);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_ALIGNMENTCHECK)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_alignmentcheck);
		printf("pvr_srv_bridge_alignmentcheck: align_checks %p align_checks_size %d\n",
			din.align_checks, din.align_checks_size);
		printf(" out: error %d\n", dout.error);

		uint32_t checks[64] = {0};
		if (din.align_checks_size)
		{
			int sz = i_min(din.align_checks_size * sizeof(uint32_t), sizeof(checks));
			memcpy_from_trace(pid, (__u64)din.align_checks, checks, sz);
		}
		for (uint32_t i = 0; i < i_min(64, din.align_checks_size); i++)
		{
			printf("  %u: 0x%X \t", i, checks[i]);
			if ((i & 0x7) == 0x7)
				printf("\n");
		}
		printf("\n");
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_GETMULTICOREINFO)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_getmulticoreinfo);
		printf("pvr_srv_bridge_getmulticoreinfo: caps %p caps_size %d\n",
			din.caps, din.caps_size);
		printf(" out: caps %p error %d num_cores %d\n", dout.caps, dout.error, dout.num_cores);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_ACQUIREINFOPAGE)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_bridge_acquireinfopage);
		printf("pvr_srv_bridge_acquireinfopage: out: pmr %p error %d\n", dout.pmr, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SRVCORE && cmd->bridge_func_id == PVR_SRV_BRIDGE_SRVCORE_RELEASEINFOPAGE)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_releaseinfopage);
		printf("pvr_srv_bridge_releaseinfopage: pmr %p\n", din.pmr);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SYNC && cmd->bridge_func_id == PVR_SRV_BRIDGE_SYNC_ALLOCSYNCPRIMITIVEBLOCK)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_bridge_alloc_sync_primitive_block);
		printf("pvr_alloc_sync_primitive_block: out: handle %p pmr %p error %d size 0x%X addr 0x%X\n",
			dout.handle, dout.pmr, dout.error, dout.size, dout.addr);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SYNC && cmd->bridge_func_id == PVR_SRV_BRIDGE_SYNC_FREESYNCPRIMITIVEBLOCK)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_free_sync_primitive_block);
		printf("pvr_free_sync_primitive_block: handle %p out: error %d\n", din.handle, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SYNC && cmd->bridge_func_id == PVR_SRV_BRIDGE_SYNC_SYNCPRIMSET)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_sync_prim_set);
		printf("pvr_sync_prim_set: handle %p index 0x%X value 0x%X\n",
			din.handle, din.index, din.value);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SYNC && cmd->bridge_func_id == PVR_SRV_BRIDGE_SYNC_SYNCALLOCEVENT)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_sync_allocevent);
		char class_name[2048] = {0};
		memcpy_from_trace(pid, (__u64)din.class_name, class_name, i_min(din.class_name_size, sizeof(class_name) - 1));
		printf("pvr_sync_allocevent: class_name[%d] %p \"%s\" fw_addr 0x%X server_sync %d\n",
			din.class_name_size, din.class_name, class_name, din.fw_addr, din.server_sync);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_SYNC && cmd->bridge_func_id == PVR_SRV_BRIDGE_SYNC_SYNCFREEEVENT)
	{
		LOAD_CMD_DATA(pvr_srv_bridge_sync_freeevent);
		printf("pvr_sync_freeevent: fw_addr 0x%X out: error %d\n", din.fw_addr, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PMRMAKELOCALIMPORTHANDLE)
	{
		LOAD_CMD_DATA(pvr_srv_pmr_makelocalimporthandle);
		printf("pvr_srv_pmr_makelocalimporthandle: buffer %p\n", din.buffer);
		printf(" out: ext_mem %p error %d\n", dout.ext_mem, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PMRUNMAKELOCALIMPORTHANDLE)
	{
		LOAD_CMD_DATA(pvr_srv_pmr_unmakelocalimporthandle);
		printf("pvr_srv_pmr_unmakelocalimporthandle: ext_mem %p\n", din.ext_mem);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PMRLOCALIMPORTPMR)
	{
		LOAD_CMD_DATA(pvr_srv_pmr_localimportpmr);
		printf("pvr_srv_pmr_localimportpmr: ext_handle %p\n", din.ext_handle);
		printf(" out: align 0x%lX size 0x%lX pmr %p error %d\n",
			dout.align, dout.size, dout.pmr, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PMRUNREFPMR)
	{
		LOAD_CMD_DATA(pvr_srv_pmr_unref_pmr);
		printf("pvr_srv_pmr_unref_pmr: pmr %p\n", din.pmr);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTCTXCREATE)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_ctx_create);
		printf("pvr_srv_devmem_int_ctx_create: kernel_memory_ctx %d\n", din.kernel_memory_ctx);
		printf(" out: server_memctx %p data %p error %d cpu_cache_line_size %d\n",
			dout.server_memctx, dout.server_memctx_data, dout.error, dout.cpu_cache_line_size);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTCTXDESTROY)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_ctx_destroy);
		printf("pvr_srv_devmem_int_ctx_destroy: server_memctx %p\n", din.server_memctx);
		printf(" out: error %d \n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGCOUNT)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_heap_cfg_count);
		printf("pvr_srv_heap_cfg_count: out: error %d heap_config_count %d\n", dout.error, dout.heap_config_count);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCOUNT)
	{
		LOAD_CMD_DATA(pvr_srv_heap_count);
		printf("pvr_srv_heap_count: heap_config_index %d\n", din.heap_config_index);
		printf(" out: error %d heap_count %d\n", dout.error, dout.heap_count);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_HEAPCFGHEAPCONFIGNAME)
	{
		LOAD_CMD_DATA(pvr_srv_heap_cfg_name);

		char name[256] = {0};
		memcpy_from_trace(pid, (__u64)din.config_name_buffer, name, i_min(din.config_name_bufer_size, sizeof(name) - 1));

		printf("pvr_srv_heap_cfg_name: buffer %p \"%s\" heap_config_index %d buffer_size %d\n",
			din.config_name_buffer, name, din.heap_config_index, din.config_name_bufer_size);
		printf(" out: buffer %p error %d\n", dout.heap_config_name, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_HEAPCFGHEAPDETAILS)
	{
		LOAD_CMD_DATA(pvr_srv_heap_cfg_details);

		char name[256] = {0};
		memcpy_from_trace(pid, (__u64)din.buffer, name, i_min(din.buffer_size, sizeof(name) - 1));

		printf("pvr_srv_heap_cfg_details: buffer %p \"%s\" heap_config_index %d heap_index %d buffer_size %d\n",
			din.buffer, name, din.heap_config_index, din.heap_index, din.buffer_size);
		printf(" out: base_addr %p size 0x%" PRIX64 " reserved_size 0x%" PRIX64 " buffer %p error %d log2_page_size %d log2_alignment %d\n",
			(void*)dout.base_addr.addr, dout.size, dout.reserved_size, dout.buffer, dout.error, dout.log2_page_size, dout.log2_alignment);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTHEAPCREATE)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_heap_create);
		printf("pvr_srv_devmem_int_heap_create: server_memctx %p heap_config_index %d heap_index %d\n",
			din.server_memctx, din.heap_config_index, din.heap_index);
		printf(" out: server_heap %p error %d\n", dout.server_heap, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PHYSMEMNEWRAMBACKEDPMR)
	{
		LOAD_CMD_DATA(pvr_srv_physmem_new_ram_backed_pmr);

		uint32_t mapping_table = -1;
		memcpy_from_trace(pid, (__u64)din.mapping_table, &mapping_table, sizeof(mapping_table));

		char annotation[2048] = {0};
		memcpy_from_trace(pid, (__u64)din.annotation, annotation, i_min(din.annotation_size, sizeof(annotation) - 1));

		printf("pvr_srv_physmem_new_ram_backed_pmr: size 0x%" PRIX64 " mapping_table %p (%d)\n"
			" annotation[%u] %p \"%s\"\n"
			" log2_page_size %u phy_blocks %u virt_blocks %u pdump_flags 0x%X pid 0x%X flags 0x%lX\n",
			din.size, din.mapping_table, mapping_table,
			din.annotation_size, din.annotation, annotation,
			din.log2_page_size, din.phy_blocks, din.virt_blocks, din.pdump_flags, din.pid, din.flags);
		printf(" out: pmr %p error %d out_flags 0x%lX\n", dout.pmr, dout.error, dout.out_flags);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTMAPPMR)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_map_pmr);
		printf("pvr_srv_devmem_int_map_pmr: pmr %p reservation %p\n", din.pmr, din.reservation);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTRESERVERANGE)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_reserve_range);
		printf("pvr_srv_devmem_int_reserve_range: address %p size 0x%zX serverHeap %p flags 0x%lX\n",
			(void*)din.addr.addr, din.size, din.server_heap, din.flags);
		printf(" out: reservation %p error %d\n", dout.reservation, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTRESERVERANGEANDMAPPMR)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_reserve_range_and_map_pmr);
		printf("devmem_int_reserve_range_and_map_pmr: address %p size 0x%zX serverHeap %p pmr %p flags 0x%lX\n",
			(void*)din.address.addr, din.length, din.server_heap, din.pmr, din.flags);
		printf(" out: reservation %p error %d\n",
			dout.reservation, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMINTUNRESERVERANGE)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_int_unreserve_range);
		printf("devmem_int_unreserve_range: reservation %p", din.reservation);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_PHYSHEAPGETMEMINFO)
	{
		LOAD_CMD_DATA(pvr_srv_physheap_getmeminfo);
		printf("pvr_srv_physheap_getmeminfo: phys_heap_mem_stats %p phys_heap_id %p phys_heap_count %d\n",
			din.phys_heap_mem_stats, din.phys_heap_id, din.phys_heap_count);
		printf(" out: phys_heap_mem_stats %p error %d\n", dout.phys_heap_mem_stats, dout.error);

		enum pvr_phys_heap heap_ids[16] = {0};
		struct pvr_phys_heap_mem_stats mem_stats[16] = {0};
		if (din.phys_heap_count)
		{
			memcpy_from_trace(pid, (__u64)din.phys_heap_id, &heap_ids, i_min(din.phys_heap_count * sizeof(enum pvr_phys_heap), sizeof(heap_ids)));
			memcpy_from_trace(pid, (__u64)din.phys_heap_mem_stats, &mem_stats,
				i_min(din.phys_heap_count * sizeof(struct pvr_phys_heap_mem_stats), sizeof(mem_stats)));
		}
		for (uint32_t i = 0; i < i_min(16, din.phys_heap_count); i++)
		{
			printf("  %u: id 0x%X: size 0x%lX free_size 0x%lX flags 0x%X type 0x%X\n", i, heap_ids[i],
				mem_stats[i].total_size, mem_stats[i].free_size, mem_stats[i].phys_heap_flags, mem_stats[i].phys_heap_type);
		}
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMXINTRESERVERANGE)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_x_int_reserve_range);
		printf("devmem_x_int_reserve_range: address %p length 0x%zX serverHeap %p\n",
			(void*)din.address.addr, din.length, din.server_heap);
		printf(" out: reservation %p error %d\n", dout.reservation, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMXINTUNRESERVERANGE)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_x_int_unreserve_range);
		printf("devmem_x_int_unreserve_range: reservation %p", din.reservation);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMXINTMAPPAGES)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_x_int_map_pages);
		printf("devmem_x_int_map_pages: pmr %p reservation %p page_count %d phys_page_offset %d virt_page_offset %d flags 0x%lX\n",
			din.pmr, din.reservation, din.page_count, din.phys_page_offset, din.virt_page_offset, din.flags);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_MM && cmd->bridge_func_id == PVR_SRV_BRIDGE_MM_DEVMEMXINTUNMAPPAGES)
	{
		LOAD_CMD_DATA(pvr_srv_devmem_x_int_unmap_pages);
		printf("devmem_x_int_unmap_pages: reservation %p page_count %d virt_page_offset %d",
			din.reservation, din.page_count, din.virt_page_offset);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTQ && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTQ_RGXCREATETRANSFERCONTEXT)
	{
		LOAD_CMD_DATA(pvr_srv_rgx_create_transfer_context);
		printf("rgx_create_transfer_context:\n robustness_address 0x%lX priv_data %p reset_framework_cmd %p priority %d\n"
			" context_flags 0x%X reset_framework_cmd_size 0x%X packed_ccb_size_u8888 0x%X\n",
			din.robustness_address, din.priv_data, din.reset_framework_cmd, din.priority,
			din.context_flags, din.reset_framework_cmd_size, din.packed_ccb_size_u8888);
		printf(" out: transfer_context %p error %d\n", dout.transfer_context, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTQ && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTQ_RGXSUBMITTRANSFER2)
	{
		LOAD_CMD_DATA(pvr_srv_rgx_submit_transfer2);
		printf("rgx_submit_transfer2:\n transfer_context %p client_update_count %p cmd_size %p sync_pmr_flags %p\n"
			" tq_prepare_flags %p update_sync_offset %p update_value %p fw_command %p\n"
			" update_fence_name %p sync_pmrs %p update_ufo_sync_prim_block %p\n"
			" update_timeline_2d %d update_timeline_3d %d check_fence %d\n"
			" ext_job_ref %u prepare_count %u sync_pmr_count %u\n",
			din.transfer_context, din.client_update_count, din.cmd_size, din.sync_pmr_flags,
			din.tq_prepare_flags, din.update_sync_offset, din.update_value, din.fw_command,
			din.update_fence_name, din.sync_pmrs, din.update_ufo_sync_prim_block,
			din.update_timeline_2d, din.update_timeline_3d, din.check_fence,
			din.ext_job_ref, din.prepare_count, din.sync_pmr_count);
		printf(" out: error %d update_fence_2d %d update_fence_3d %d\n",
			dout.error, dout.update_fence_2d, dout.update_fence_3d);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTQ && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTQ_RGXTQGETSHAREDMEMORY)
	{
		LOAD_CMD_OUT_DATA(pvr_srv_rgxtq_getsharedmemory);
		printf("rgxtq_getsharedmemory: out: cli_pmr_mem %p error %d\n",
			dout.cli_pmr_mem, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTQ && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTQ_RGXTQRELEASESHAREDMEMORY)
	{
		LOAD_CMD_DATA(pvr_srv_rgxtq_releasesharedmemory);
		printf("rgxtq_releasesharedmemory: pmr_mem %p\n",din.pmr_mem);
		printf(" out: error %d\n", dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXCMP && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXCMP_RGXCREATECOMPUTECONTEXT)
	{
		LOAD_CMD_DATA(pvr_srv_rgx_create_compute_context);
		printf("rgx_create_compute_context: robustness_address 0x%lX priv_data %p reset_framework_cmd %p static_compute_context_state %p\n"
			" priority %d context_flags 0x%X reset_framework_cmd_size 0x%X max_deadline_ms %u packed_ccb_size 0x%X static_compute_context_state_size 0x%X\n",
			din.robustness_address, din.priv_data, din.reset_framework_cmd, din.static_compute_context_state,
			din.priority, din.context_flags, din.reset_framework_cmd_size, din.max_deadline_ms, din.packed_ccb_size, din.static_compute_context_state_size);
		printf(" out: compute_context %p error %d\n", dout.compute_context, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTA3D && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTA3D_RGXCREATEHWRTDATASET)
	{
		LOAD_CMD_DATA(pvr_srv_rgx_create_hwrt_dataset);
		printf("rgx_create_hwrt_dataset_cmd: flipped_multi_sample_ctl 0x%lX multi_sample_ctl 0x%lX hwrt_dataset %p\n"
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
		LOAD_CMD_DATA(pvr_srv_rgx_kick_ta3d2);
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
		LOAD_CMD_DATA(pvr_srv_rgx_create_free_list);
		printf("rgx_create_free_list: free_list_reservation %p mem_ctx_priv_data %p global_free_list %p\n"
			" grow_free_list_pages %u grow_param_threshold %u init_free_list_pages %u max_free_list_pages %u free_list_check %d\n",
			din.free_list_reservation, din.mem_ctx_priv_data, din.global_free_list, din.grow_free_list_pages,
			din.grow_param_threshold, din.init_free_list_pages, din.max_free_list_pages, din.free_list_check);
		printf(" out: cleanup_cookie %p error %d\n",
			dout.cleanup_cookie, dout.error);
	}
	else if (cmd->bridge_id == PVR_SRV_BRIDGE_RGXTA3D && cmd->bridge_func_id == PVR_SRV_BRIDGE_RGXTA3D_RGXCREATERENDERCONTEXT)
	{
		LOAD_CMD_DATA(pvr_srv_rgx_create_render_context);
		printf("rgx_create_render_context: vdm_callstack_addr %p robustness_address 0x%lX priv_data %p reset_framework_cmd %p static_render_context_state %p\n"
			" priority %d context_flags 0x%X reset_framework_cmd_size 0x%X max_3d_deadline_ms %u max_ta_deadline_ms %u\n"
			" packed_ccb_size 0x%X static_render_context_state_size 0x%X call_stack_depth %d\n",
			(void*)din.vdm_callstack_addr.addr, din.robustness_address, din.priv_data, din.reset_framework_cmd, din.static_render_context_state,
			din.priority, din.context_flags, din.reset_framework_cmd_size, din.max_3d_deadline_ms, din.max_ta_deadline_ms,
			din.packed_ccb_size, din.static_render_context_state_size, din.call_stack_depth);
		printf(" out: render_context %p error %d\n", dout.render_context, dout.error);
	}
	else
	{
		return false;
	}
#undef VALIDATE_IN_SIZE
#undef VALIDATE_OUT_SIZE
#undef LOAD_CMD_DATA
#undef LOAD_CMD_OUT_DATA
	return true;
}

void print_pvrsrv_cmd(int pid, __u64 src)
{
	struct drm_pvr_srvkm_cmd cmd = {0};
	memcpy_from_trace(pid, src, &cmd, sizeof(cmd));
	
	if (!print_pvr_srv_cmd_data(pid, &cmd))
	{
		printf("drm_pvr_srvkm_cmd: bridge_id %u(%s) func_id %u(%s) in data %p out data %p in size %u out size %u\n",
			cmd.bridge_id, srv_bridge_id_to_str(cmd.bridge_id), cmd.bridge_func_id, srv_bridge_func_to_str(cmd.bridge_id,cmd.bridge_func_id),
			(void*)cmd.in_data_ptr, (void*)cmd.out_data_ptr, cmd.in_data_size, cmd.out_data_size);
	}
}

bool print_drm_ioctl(int pid, __u64 c, __u64 src)
{
	switch (c)
	{
	case DRM_IOCTL_VERSION:
		print_drm_version(pid, src);
		break;
	case DRM_IOCTL_PVR_SRVKM_CMD:
		print_pvrsrv_cmd(pid, src);
		break;
	case DRM_IOCTL_PVR_SYNC_RENAME_CMD:
		print_pvr_sync_rename(pid, src);
		break;
	case DRM_IOCTL_PVR_SW_SYNC_CREATE_FENCE_CMD:
		print_pvr_sw_sync_create_fence(pid, src);
		break;
	case DRM_IOCTL_PVR_SW_SYNC_INC_CMD:
		print_pvr_sw_timeline_advance(pid, src);
		break;
	case DRM_IOCTL_PVR_SRVKM_INIT:
	case DRM_IOCTL_SRVKM_INIT:
		print_pvrsrv_init(pid, src);
		break;
	default:
		return false;
	}
	return true;
}

void print_syscall(struct ptrace_syscall_info *sci, struct ptrace_syscall_info *sci_exit, int pid, const char *open_path)
{
#if 1
	if (!(sci->entry.nr == SYS_openat || sci->entry.nr == SYS_close || sci->entry.nr == SYS_read || sci->entry.nr == SYS_write
		 || sci->entry.nr == SYS_mmap || sci->entry.nr == SYS_munmap || sci->entry.nr == SYS_fstat || sci->entry.nr == SYS_ioctl
		 || sci->entry.nr == SYS_socket || sci->entry.nr == SYS_setsockopt || sci->entry.nr == SYS_bind || sci->entry.nr == SYS_listen))
		return;
#endif
	if (sci->entry.nr == SYS_openat && sci_exit->exit.is_error)
		return;
	if (sci->entry.nr == SYS_write && sci->entry.args[0] == 1)//stdout
		return;

	if (sci->entry.nr == SYS_ioctl && !sci_exit->exit.is_error
		&& (sci->entry.args[1] == DRM_IOCTL_VERSION
		|| sci->entry.args[1] == DRM_IOCTL_PVR_SRVKM_CMD
		|| sci->entry.args[1] == DRM_IOCTL_PVR_SYNC_RENAME_CMD
		|| sci->entry.args[1] == DRM_IOCTL_PVR_SW_SYNC_CREATE_FENCE_CMD
		|| sci->entry.args[1] == DRM_IOCTL_PVR_SW_SYNC_INC_CMD
		|| sci->entry.args[1] == DRM_IOCTL_PVR_SRVKM_INIT
		|| sci->entry.args[1] == DRM_IOCTL_SRVKM_INIT))
	{
		printf("ioctl(0x%llX) ", sci->entry.args[0]);
		print_drm_ioctl(pid, sci->entry.args[1], sci->entry.args[2]);
		return;
	}

	if (sci->entry.nr == SYS_openat)
	{
		printf("openat(%lld, \"%s\", 0x%llX)", sci->entry.args[0], open_path, sci->entry.args[2]);
	}
	else if (sci->entry.nr == SYS_close)
	{
		printf("close(0x%llX)", sci->entry.args[0]);
	}
	else if (sci->entry.nr == SYS_read)
	{
		printf("read(0x%llX, buf 0x%llX, count 0x%llX)", sci->entry.args[0], sci->entry.args[1], sci->entry.args[2]);
	}
	else if (sci->entry.nr == SYS_write)
	{
		printf("write(0x%llX, buf 0x%llX, count 0x%llX)", sci->entry.args[0], sci->entry.args[1], sci->entry.args[2]);
	}
	else if (sci->entry.nr == SYS_fstat)
	{
		printf("fstat(0x%llX, buf 0x%llX)", sci->entry.args[0], sci->entry.args[1]);
	}
	else if (sci->entry.nr == SYS_mmap)
	{
		//TODO: PROT_READ|PROT_WRITE, MAP_SHARED
		printf("mmap(addr 0x%llX, length 0x%llX, prot 0x%llX, flags 0x%llX, fd 0x%llX, offset 0x%llX)",
			sci->entry.args[0], sci->entry.args[1], sci->entry.args[2],
			sci->entry.args[3], sci->entry.args[4], sci->entry.args[5]);
	}
	else if (sci->entry.nr == SYS_munmap)
	{
		printf("munmap(addr 0x%llX, length 0x%llX)", sci->entry.args[0], sci->entry.args[1]);
	}
	else if (sci->entry.nr == SYS_socket)
	{
		printf("socket(domain 0x%llX, type 0x%llX, protocol 0x%llX)",
			sci->entry.args[0], sci->entry.args[1], sci->entry.args[2]);
	}
	else if (sci->entry.nr == SYS_setsockopt)
	{
		printf("setsockopt(socket 0x%llX, level 0x%llX, option_name 0x%llX, option_value 0x%llX, option_len 0x%llX)",
			sci->entry.args[0], sci->entry.args[1], sci->entry.args[2], sci->entry.args[3], sci->entry.args[4]);
	}
	else if (sci->entry.nr == SYS_bind)
	{
		printf("bind(socket 0x%llX, addr 0x%llX, addr_len 0x%llX)",
			sci->entry.args[0], sci->entry.args[1], sci->entry.args[2]);
	}
	else if (sci->entry.nr == SYS_listen)
	{
		printf("listen(socket 0x%llX, backlog 0x%llX)", sci->entry.args[0], sci->entry.args[1]);
	}
	else if (sci->entry.nr == SYS_ioctl)
	{
		__u64 c = sci->entry.args[1];
		if (_IOC_TYPE(c) != DRM_IOCTL_BASE)
		{
			printf("== ioctl(0x%llX, 0x%llX(not drm), 0x%llX)",sci->entry.args[0], c, sci->entry.args[2]);
		}
		else
		{
			const char *name = drm_ioctl_to_str(c);
			if (name)
				printf("== ioctl(0x%llX, DRM %s, 0x%llX)", sci->entry.args[0], name, sci->entry.args[2]);
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
				printf("== ioctl(0x%llX, 0x%llX(%s 0x%X %d), 0x%llX)",sci->entry.args[0], c, dir_str, (int)_IOC_NR(c), (int)_IOC_SIZE(c), sci->entry.args[2]);
			}
		}
	}
	else
	{
		const char *scname = syscall_to_str(sci->entry.nr);
		if (scname)
			printf("%s", scname);
		else
			printf("== sycall %lld", sci->entry.nr);
		printf("(");
		int used_args = 6;
		if (sci->entry.nr == SYS_getpid)
			used_args = 0;
		for (int i = 0; i < used_args; i++)
			printf("%lld%s", sci->entry.args[i], i == used_args - 1 ? "" : ", ");
		printf(")");
	}

	printf(" = 0x%llX%s\n", sci_exit->exit.rval, sci_exit->exit.is_error ? " ERROR" : "");
	
	if (sci->entry.nr == SYS_ioctl)
	{
		print_drm_ioctl(pid, sci->entry.args[1], sci->entry.args[2]);
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
				|| !strncmp(open_path, "/dev/dri/card0", sizeof(open_path))
				|| !strncmp(open_path, "/dev/dri/card1", sizeof(open_path)))
			{
				printf("!!! open dri device = 0x%llX\n", sci_exit.exit.rval);
				insert_new_fd(dri_fds, dri_fds_count, sci_exit.exit.rval);
			}
		}
		else if (sci.entry.nr == SYS_close)
		{
			if (remove_fd(dri_fds, dri_fds_count, sci.entry.args[0]) != -1)
				printf("!! closed dri device 0x%llX\n", sci.entry.args[0]);
		}

		ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	}

	exit(0);
}

