/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * KVM host library for setting up and running virtual machine tests. Tests
 * can either use the default setup/run/host functions or use the advanced
 * API to create customized VMs.
 */

/*
 * Most basic usage:
 *
 * #include "kvm_test.h"
 *
 * #ifdef COMPILE_PAYLOAD
 *
 * void main(void)
 * {
 *	[VM guest code goes here]
 * }
 *
 * #else
 *
 * [optional VM host setup/run/cleanup code goes here]
 *
 * static struct tst_test test = {
 *	.test_all = tst_kvm_run,
 *	.setup = tst_kvm_setup,
 *	.cleanup = tst_kvm_cleanup,
 * };
 *
 * #endif
 */

#ifndef KVM_HOST_H_
#define KVM_HOST_H_

#include <inttypes.h>
#include <linux/kvm.h>
#include "kvm_common.h"

#define VM_KERNEL_BASEADDR 0x1000
#define VM_RESET_BASEADDR 0xfffffff0
#define VM_RESET_CODE_SIZE 8

#define MIN_FREE_RAM (10 * 1024 * 1024)
#define DEFAULT_RAM_SIZE (16 * 1024 * 1024)

struct tst_kvm_instance {
	int vm_fd, vcpu_fd;
	struct kvm_run *vcpu_info;
	size_t vcpu_info_size;
	void *ram;
	struct tst_kvm_result *result;
};

/* Test binary to be installed into the VM at VM_KERNEL_BASEADDR */
extern const char kvm_payload_start[], kvm_payload_end[];

/* CPU reset code to be installed into the VM at VM_RESET_BASEADDR */
extern const unsigned char tst_kvm_reset_code[VM_RESET_CODE_SIZE];

/* Default KVM test functions. */
void tst_kvm_setup(void);
void tst_kvm_run(void);
void tst_kvm_cleanup(void);

/*
 * Validate KVM guest test result (usually passed via result->result) and
 * fail with TBROK if the value cannot be safely passed to tst_res() or
 * tst_brk().
 */
void tst_kvm_validate_result(int value);

/*
 * Allocate memory slot for the VM. The returned pointer is page-aligned
 * so the actual requested base address is at ret[baseaddr % pagesize].
 *
 * The first argument is a VM file descriptor created by ioctl(KVM_CREATE_VM)
 *
 * The return value points to a guarded buffer and the user should not attempt
 * to free() it. Any extra space added at the beginning or end for page
 * alignment will be writable.
 */
void *tst_kvm_alloc_memory(int vm, unsigned int slot, uint64_t baseaddr,
	size_t size, unsigned int flags);

/*
 * Find CPUIDs supported by KVM. x86_64 tests must set non-default CPUID,
 * otherwise bootstrap will fail to initialize 64bit mode.
 * Returns NULL if ioctl(KVM_GET_SUPPORTED_CPUID) is not supported.
 *
 * The argument is a file descriptor created by open("/dev/kvm")
 */
struct kvm_cpuid2 *tst_kvm_get_cpuid(int sysfd);

/*
 * Initialize the given KVM instance structure. Creates new KVM virtual machine
 * with 1 virtual CPU, allocates VM RAM (max. 4GB minus one page) and
 * shared result structure. KVM memory slots 0 and 1 will be set by this
 * function.
 */
void tst_kvm_create_instance(struct tst_kvm_instance *inst, size_t ram_size);

/*
 * Execute the given KVM instance and print results.
 */
void tst_kvm_run_instance(struct tst_kvm_instance *inst);

/*
 * Close the given KVM instance.
 */
void tst_kvm_destroy_instance(struct tst_kvm_instance *inst);

#endif /* KVM_HOST_H_ */
