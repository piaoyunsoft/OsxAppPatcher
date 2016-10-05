#include "patcher.h"
#include "config.h"

#if DEBUG_LOG
	#define dprintf printf
#else
	#define dprintf(...)
#endif

#if defined(_MAC64) || defined(__LP64__)
	#define __address_t mach_vm_address_t
	#define __size_t mach_vm_size_t
#else
	#define __address_t vm_address_t
	#define __size_t vm_size_t
#endif

uint64_t findBasicAddress(int pid){
	__size_t region_size = 0;
	__address_t region = 0;
	mach_port_t task = 0;
	int ret = 0;

	ret = task_for_pid(mach_task_self(),pid,&task);
	if (ret != 0)
	{	
		printf("task_for_pid() message %s!\n",mach_error_string(ret));
		return 0;
	}

	  /* Get region boundaries */
#if defined(_MAC64) || defined(__LP64__)
	vm_region_basic_info_data_64_t info;
	mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
	vm_region_flavor_t flavor = VM_REGION_BASIC_INFO_64;
	if ((ret = mach_vm_region(mach_task_self(), &region, &region_size, flavor, (vm_region_info_t)&info, 
		(mach_msg_type_number_t*)&info_count, (mach_port_t*)&task)) != KERN_SUCCESS)
	{
		printf("mach_vm_region() message %s!\n",mach_error_string(ret));
		return 0;
	}
#else
	vm_region_basic_info_data_t info;
	mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT;
	vm_region_flavor_t flavor = VM_REGION_BASIC_INFO;
	if ((ret = vm_region(mach_task_self(), &region, &region_size, flavor, (vm_region_info_t)&info, 
		(mach_msg_type_number_t*)&info_count, (mach_port_t*)&task)) != KERN_SUCCESS)
	{
		printf("vm_region() message %s!\n",mach_error_string(ret));
		return 0;
	}
#endif
	return region;
}

vm_size_t readMemory(char *buf,vm_size_t len,int pid,vm_address_t address)
{
	vm_size_t outSize = 0;
	mach_port_t task = 0;

	int ret = task_for_pid(mach_task_self(),pid,&task);
	if (ret != 0)
	{	
		printf("task_for_pid() message %s!\n",mach_error_string(ret));
		return 0;
	}

	ret = vm_read_overwrite(task,address,len,(vm_address_t)buf,&outSize);
	if (ret != 0)
	{
		printf("vm_read_overwrite() message %s!\n",mach_error_string(ret));
		return 0;
	}
	return outSize;
}

 int writeMemory(void *dst_ptr, void *data_ptr, uint8_t data_len)
 {
 	mach_port_t task;
 	__size_t region_size = 0;
 	__address_t region = (vm_address_t)dst_ptr;

  /* Get region boundaries */
 #if defined(_MAC64) || defined(__LP64__)
 	vm_region_basic_info_data_64_t info;
 	mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
 	vm_region_flavor_t flavor = VM_REGION_BASIC_INFO_64;
 	if (mach_vm_region(mach_task_self(), &region, &region_size, flavor, (vm_region_info_t)&info, (mach_msg_type_number_t*)&info_count, (mach_port_t*)&task) != 0)
 	{
 		return 0;
 	}
 #else
 	vm_region_basic_info_data_t info;
 	mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT;
 	vm_region_flavor_t flavor = VM_REGION_BASIC_INFO;
 	if (vm_region(mach_task_self(), &region, &region_size, flavor, (vm_region_info_t)&info, (mach_msg_type_number_t*)&info_count, (mach_port_t*)&task) != 0)
 	{
 		return 0;
 	}
 #endif

 	if ((uint64_t)(dst_ptr + data_len) > region + region_size)
 	{
 		return 0;
 	}

  /* Change memory protections to rw- */
 	if (vm_protect(mach_task_self(), region, region_size, 0, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY) != KERN_SUCCESS)
 	{
 		//_LineLog();
 		return 0;
 	}

  /* Actually perform the write */
 	memcpy(dst_ptr, data_ptr, data_len);

  /* Flush CPU data cache to save write to RAM */
 	sys_dcache_flush(dst_ptr, sizeof(data_len));

  /* Invalidate instruction cache to make the CPU read patched instructions from RAM */
 	sys_icache_invalidate(dst_ptr, sizeof(data_len));

  /* Change memory protections back to r-x */
 	vm_protect(mach_task_self(), region, region_size, 0, VM_PROT_EXECUTE | VM_PROT_READ);
 	return 1;
 }

 int writeOneByte(void *dst_ptr, uint8_t data) {
 	return writeMemory(dst_ptr, &data, 1);
 }

void print_memory(void *ptr, unsigned long len) {
	unsigned char *mem_ptr = (unsigned char*)ptr;

	for (int i = 0; i < len; ++i)
	{
		printf("%.2hhx ", mem_ptr[i]);
	}
	putchar('\n');

	fflush(stdout);
}

int dump_memory(void *ptr, size_t len, const char* filename) {
	FILE *fp;

	fp = fopen(filename, "wb");
	if (fp != NULL) {
		fwrite(ptr, len, 1, fp);
		return 1;
	}
	return 0;
}

unsigned char inline_hook_code[] = {
	0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //movabsq 0x1122334455667788, %rax
	0xff, 0xe0, 	//jmp *%rax
	0xc3 // ret
};

int inline_hook(void *dst_fun, void *new_fun) {
	*((uint64_t*)(inline_hook_code+2)) = (uint64_t)new_fun;
	return writeMemory(dst_fun, inline_hook_code, 13);
}

extern void yourcode(int pid, uint64_t basicaddress);

static void startup() {
	int pid = getpid();

	mach_vm_address_t address = 0;

	address = findBasicAddress(pid);

	dprintf("Target pid     : %d\n",pid);
	dprintf("Base address   : %llx\n", address);

	if (address == 0)
	{
		printf("findBasicAddress() faild!\n");
	}

	yourcode(pid, (uint64_t)address);
}

static void* patcher_thread(void *p)
{

	startup();

	return NULL;
}

void __attribute__((constructor)) patcher_enter()
{
	int err;
	pthread_t ntid;

#if STARTUP_MODE

	dprintf("Thread         : main thread\n");

	startup();

#else
	dprintf("Thread         : standalone thread\n");

    err = pthread_create(&ntid, NULL, patcher_thread, NULL);
    if (err != 0)
    {
    	printf("Can't create patcher thread: %s\n", strerror(err));
    	return ;
    }

#endif
    
#if PROCESS_DELAY
    sleep(PROCESS_DELAY);
#endif
}

