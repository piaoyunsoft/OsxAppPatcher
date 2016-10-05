#ifndef __PATCHER_H__
#define __PATCHER_H__

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "pthread.h"

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/sysctl.h>

#include <mach/mach.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>

#include "libkern/OSCacheControl.h"

uint64_t findBasicAddress(int pid);

vm_size_t readMemory(char *buf,vm_size_t len,int pid,vm_address_t address);

int writeMemory(void *dst_ptr, void *data_ptr, uint8_t data_len);

int writeOneByte(void *dst_ptr, uint8_t data);

void print_memory(void *ptr, unsigned long len);

int inline_hook(void *dst_fun, void *new_fun);

int dump_memory(void *ptr, size_t len, const char* filename);

#endif
