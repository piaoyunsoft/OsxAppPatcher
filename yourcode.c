#include "patcher.h"

/*

Usage:
	export DYLD_FORCE_FLAT_NAMESPACE=1
	export DYLD_INSERT_LIBRARIES=~/libpatcher.dylib

uint64_t findBasicAddress(int pid);
vm_size_t readMemory(char *buf,vm_size_t len,int pid,vm_address_t address);
int writeMemory(void *dst_ptr, void *data_ptr, uint8_t data_len);
int writeOneByte(void *dst_ptr, uint8_t data);
void print_memory(void *ptr, unsigned long len);
int inline_hook(void *dst_fun, void *new_fun);
int dump_memory(void *ptr, size_t len, const char* filename);

*/

void mymain(){
	printf("Oh!!!\n");
}

void yourcode(int pid, uint64_t basicaddress) {
	printf("Hello world!\n");
	void *entrypoint = (void*)(basicaddress+0x4ca04);
	inline_hook(entrypoint, mymain);
}

