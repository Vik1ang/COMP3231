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
    as->Region = NULL;

	return as;
}

/*
 * create a new address space that is an exact copy of an old one.
 * 
 */
int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */
    
    // copy region 
    struct region *OldRegion;
    OldRegion = old->Region;

    // copy to new address space 
    while(OldRegion != NULL){
        struct region *NewRegion = kmalloc(sizeof(struct Region));
        NewRegion->Addr = OldRegion->Addr;
        NewRegion->FileSize = OldRegion->FileSize;
        NewRegion->Perm = OldRegion->Perm;
        NewRegion->PrePerm = OldRegion->PrePerm;
        
        newas->Region = NewRegion;
        OldRegion = OldRegion->next;
    }

	/** (void)old; */

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
    if(as == NULL)
        return NULL;

    struct region *reg = as->Region;
    while(reg != NULL){
        struct region *tmp;
        tmp = reg;
        kfree(tmp);
        
        reg = reg->next;
    }

    
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
    /*
     * code
     */ 
    for(int i = 0;i < NUM_TLB ;i++){
        //write the TLB entry specified by ENTRYHI and ENTRYLO
        //into TLB slot chosen by the processor.
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        
    }
    splx(s);

}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
	
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
	if (as == NULL) return EFAULT;
	// idea from dumbvm.c
	// not very sure
	memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;
	
	// current length
	memsize += (memsize + PAGE_SIZE - 1) & PAGE_FRAME;
	
	// new region
	struct region *NewRg = kmalloc(sizeof(struct region));
	if(NewRg == NULL) return EFAULT;
	NewRg->Addr = vaddr;
	NewRg->Filesize = memsize;
	NewRg->readable = readable;
	NewRg->writeable = writeable;
	NewRg->executable = executable;
	
	// now add to the end of region list
	struct region *tmp;
	tmp = as->region;
	// make sure as->region is the last one in the list
	while(tmp->next != NULL){
		tmp = tmp->next;
	}
	tmp->next = NewRg;
	
	// (void)as;
	// (void)vaddr;
	// (void)memsize;
	// (void)readable;
	// (void)writeable;
	// (void)executable;
	// return ENOSYS; /* Unimplemented */
	return 0;
}

// this is called before actually loading from an executable into the address space.
int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	if(as == NULL) return EFAULT;
	struct region *CurrReg;
	CurrReg = as->region;
	
	// change all to writeable
	while(CurrReg != NULL){
		CurrReg->writeable = true;
		CurrReg = CurrReg->next;
	}

	// (void)as;
	return 0;
}

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
	
	// change to !writeable
	while(CurrReg != NULL){
		CurrReg->writeable = false;
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
	size_t size = USER_STACKPAGES * PAGE_SIZE;	// stakcsize
	vaddr_t vaddr = USERSTACK - size;			// virtual address
	// not sure the permission on the stack 
	// set permission to 1
	as_define_region(as, vaddr, size, true, true, true);
	
	// (void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}

