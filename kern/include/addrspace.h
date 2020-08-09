/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 *
 * UNSW: If you use ASST3 config as required, then this file forms
 * part of the VM subsystem.
 *
 */

/*
 * create a new empty addrspace 
 * Return NULL on out of memory error 
 */ 
struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}

	/*
	 * Initialize as needed.
	 */
    as->region = NULL;
	// allocate and initialize page table 
	as->pagetable = (paddr_t **)alloc_kpages(1);
	for(int i = 0; i < PAGE_SIZE; i++){
		as->pagetable[i] = NULL;
	}
	// emmmmmmmmmm, it was wrong
	// memset(as->pagetable, NULL, PAGE_SIZE);

	return as;
}

/*
 * allocate a new address space
 * add all the same region as source
 * for each paged in souce
 * 	* allocate a frame in dest
 * 	* copy contents from source frame to dest frame
 *  * add PT entry for dest
 * 
 */


// TODO:get struggle on here 
/*
 * copy contents 
 * and
 * copy page table (allocate a new one)
 */
int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if(newas == NULL){
		return ENOMEM;
	}

	/**
	 * Write this
	 */ 
	// old region
	struct region *OldRegion = old->region;
	// new region
	struct region *NewRegion = NULL;
	// copy in new region

	// copy contents
	while (OldRegion != NULL)
	{
		// it was wrong with below but cannot figure out where i get wrong

		struct region *tempRegion = kmalloc(sizeof(struct region));

		tempRegion->addrs = OldRegion->addrs;
		tempRegion->size = OldRegion->size;
		tempRegion->readable = OldRegion->readable;
		tempRegion->writeable = OldRegion->writeable;
		tempRegion->executable = OldRegion->executable;
		// DO NOT FORGET THIS
		// IT TAKES ME A LOT TIME
		// !!!!!!!!!!!!!!!!!!!
		tempRegion->next = NULL;

		if(NewRegion == NULL){
			// maybe there is another better solution
			// i try a lot and finally solve the bad memory problem
			newas->region = tempRegion;
			NewRegion = tempRegion;
			OldRegion = OldRegion->next;
			continue;
			
		}
		NewRegion->next = tempRegion;
		NewRegion = tempRegion;
		OldRegion = OldRegion->next;
		
	}

	// copy page table
	// allocate a two-level radix tree whose lower tier is made up of blocks of size PAGE_SIZE/4
	// PAGE_SIZE is commonly 4096, each of these blocks holds 1024 pointers (on a 32-bit machine
	// each blocks 1024
	for(int i = 0; i < 1024; i++){
		if(old->pagetable[i] == NULL){
			continue;
		}
		// allocate page table, each page size is 4096
		newas->pagetable[i] = kmalloc(PAGE_SIZE);
		// second page table
		// each page bloack is 1024
		for(int j = 0; j < 1024; j++){
			// if old entry is 0, make new entry 0
			if(old->pagetable[i][j] == 0){
				newas->pagetable[i][j] = 0;
				continue;
			}
			// follow by asst3.pdf but still feel wired about it
			// allocate a new frame in dest
			vaddr_t NewFrame = alloc_kpages(1);
			// not zero the block is also right FEEL WIRED!!!
			// zero the block
			// bzero((void *)NewFrame, PAGE_SIZE)
			
			// Change kernel virtual address to physical to store
			
			// DONOT forget & PAGE_FRAME
			// using bit operation can get the address
			vaddr_t dest = PADDR_TO_KVADDR(old->pagetable[i][j] & PAGE_FRAME);
			// using kernel address => using vaddr_t 
			// like dumbvm.c using memove to copy a block of memory, handling overlapping
			memmove((void *) NewFrame, (const void*)dest, PAGE_SIZE);
			paddr_t NewAddr = KVADDR_TO_PADDR(NewFrame) & PAGE_FRAME;
			// looking for internet for a long while 
			// thanks tutorial pdf, it helps me to solve this
			newas->pagetable[i][j] = NewAddr | TLBLO_VALID | TLBLO_DIRTY;

		}
		
	}

	*ret = newas;
	return 0;
}

// free all region 

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */

	// free region
	if(as == NULL) return;

	struct region *ToFree = NULL;
	struct region *region = as->region;
	while (region != NULL)
	{
		ToFree = region;
		region = region->next;
		kfree(ToFree);
		
	}
	

	// free pagetable

	kfree(as);
}

void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	/*
	 * Write this.
	 */

    // get idea from spl.h and dumbvm.c
     
    // sets IPL to the highest value, disabling all interrupts.
    int spl = splhigh();
    // flush TLB
    for(int i = 0;i < NUM_TLB ;i++){
        //write the TLB entry specified by ENTRYHI and ENTRYLO
        //into TLB slot chosen by the processor.
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        
    }
    splx(spl);

}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */

	// same like as_activate() for disable interrupt
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	/*
	 * Write this.
	 */

    // get idea from spl.h and dumbvm.c
     
    // sets IPL to the highest value, disabling all interrupts.
    int spl = splhigh();
    // flush TLB
    for(int i = 0;i < NUM_TLB ;i++){
        //write the TLB entry specified by ENTRYHI and ENTRYLO
        //into TLB slot chosen by the processor.
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        
    }
    splx(spl);
	
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */
	// size_t napges;
	if (as == NULL) return EFAULT;
	// idea from dumbvm.c
	// not very sure but it is right
	// current length
	memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	struct region *region = kmalloc(sizeof(struct region));
	KASSERT(region != NULL);

	region->addrs = vaddr;
	region->size = memsize;
	region->readable = readable;
	region->writeable = writeable;
	region->executable = executable;

	// add region to addrspace
	region->next = as->region; 
	as->region = region;
	return 0;

}

// this is called before actually loading from an executable into the address space.
// as_prepare_load() enable writing to the code segment while the OS loads the code associated with the process

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	// make everything write
	struct region *curr = as->region;

	// store current write flag
	// set write 1
	while (curr != NULL) {
		// curr->dirty = curr->writeable;
		curr->writeable = true;
		curr = curr->next; 
	}

	return 0;
}

// as_complete_load() then removes write permission to the code segment to revert it back to read-only.
int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	// similar like as_prepare_load
	// set all to cannot writeable
	if(as == NULL) return EFAULT;
	struct region *CurrReg;
	CurrReg = as->region;
	
	// change to !writeable
	while(CurrReg != NULL){
		// 
		CurrReg->writeable = 1;
		CurrReg->readable = 1;

		CurrReg = CurrReg->next;
	}
	

	// (void)as;
	return 0;
}

/*
 * set up the stack region in the address space.
 * Hands back the initial stack pointer for the new process.
 */
int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */
	if(as == NULL) return EFAULT;
	
	// using the as_define_region to set up the stack region
	size_t size = 16 * PAGE_SIZE;
	vaddr_t vaddr = USERSTACK - size;			// virtual address
	// not sure the permission on the stack 
	// so set all permission to 1
	as_define_region(as, vaddr, size, true, true, true);
	
	// (void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}

