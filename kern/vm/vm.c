

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/tlb.h>
#include <spl.h>
#include <proc.h>
#include <current.h>

/* Place your page table functions here */
// #define STACK_SIZE PAGE_SIZE*16

void vm_bootstrap(void)
{
    /* Initialise any global components of your VM sub-system here.  
     *  
     * You may or may not need to add anything here depending what's
     * provided or required by the assignment spec.
     */
}

/*
 * TLB misses, writes to read-only pages, and accesses to invalid pages. 
 * It is responsible for resolving the fault by either returning an error, 
 * or loading an appropriate TLB entry for the application to continue.
 */ 

// update after get idea for dumbvm.c and
// rewrite after looking VM Fault Approximate Flow Chart
// followed by the chart in asst3.pdf
// thanks asst3 workthrough
int
vm_fault(int faulttype, vaddr_t faultaddress)
{
    // No process
    if(curproc == NULL) 
        return EFAULT;
    
    // No addrspace set up.
    struct addrspace *as = proc_getas();
    if(as == NULL)
        return EFAULT;

    // if VM_FAULT_READONLY 
    switch (faulttype){
        // error on read only
        case VM_FAULT_READONLY:
            return EFAULT;
        case VM_FAULT_READ:
        case VM_FAULT_WRITE:
            break;
        default:
            return EINVAL;
    }
    // get the address(page section)
    faultaddress &= PAGE_FRAME;

    
    /* Flow Chart
     * vm_fault:
     * if(VM_FAULT_READONLY)
     *      return EFAULT;
     * else
     *      lookup pagetable
     *
     *      if(valid translation)
     *          load tlb 
     *      else 
     *          look up region 
     *
     *          if(valid region)
     *              allocate frame, zero-fill, insert page table entry
     *              load tlb 
     *          else 
     *              return EFAULT 
     * end
     */ 


    // before we look up the pagetable
    // we need find the each entry and go on
    // level 1 page table index, highest 10 bits (32 - 22 = 10) so shift 22
    vaddr_t LV1_PT_Index = faultaddress >> 22;
    // level 2 page table index, get next 10 bits
    // get this one from tutriol solution, but still some confused
    vaddr_t LV2_PT_Index = (faultaddress >> 12) & 0x3ff;
    // usr for tlb loading
    uint32_t Entry_HI;
    uint32_t Entry_LO;

    // look up pagetable
    // first lookup there is valid translation
    if((as->pagetable[LV1_PT_Index] != NULL) && (as->pagetable[LV1_PT_Index][LV2_PT_Index] != 0)){
            // if valid translation load tlb
            int spl; 
            // disable interrupts
            spl = splhigh();
            Entry_HI = (uint32_t)(faultaddress & PAGE_FRAME);
            Entry_LO =  (uint32_t)as->pagetable[LV1_PT_Index][LV2_PT_Index];
            tlb_random(Entry_HI, Entry_LO);
            splx(spl);
            return 0;
    } 
	
	// invalid translation
	// look up region
	struct region *region = as->region;
    // search region
    while (region != NULL)
    {
        // it will failed the crash test if dont have this one!!!!!
        // fault address must >= region address and <= region->size + region->addrs
        if(faultaddress >= region->addrs && faultaddress <= region->size * PAGE_SIZE + region->addrs){
            // if find the valid region break
            break;
        }
        region = region->next;
    }

    // if invalid
	if(region == NULL)
		return EFAULT;
	
	// else valid region
    // first if dont have entry create a pagetable entry  size is 4096
    if(as->pagetable[LV1_PT_Index] == NULL){
		as->pagetable[LV1_PT_Index] = kmalloc(PAGE_SIZE);
        KASSERT(as->pagetable[LV1_PT_Index] != NULL);
        // zero the new entry
		for(int i = 0; i < 1024; i++){
		as->pagetable[LV1_PT_Index][i] = 0;
	    }
        
	}
	
	// allocate frame
	vaddr_t NewFrame = alloc_kpages(1);
	// bezero the frame
    // if not zero the frame it still works, i am so confused!!!
    // my think, zero fill is zero the new entry(second page table)
	// bzero((void *)NewFrame, PAGE_SIZE);

	// insert page table entry
	paddr_t New_PAddr = KVADDR_TO_PADDR(NewFrame & PAGE_FRAME);
    // insert!!! thanks tutorial material. It helps me lot!
	as->pagetable[LV1_PT_Index][LV2_PT_Index] = (New_PAddr & PAGE_FRAME) | TLBLO_VALID | TLBLO_DIRTY;

	
    // Load tlb
    Entry_HI = (uint32_t)(faultaddress & PAGE_FRAME) & TLBHI_VPAGE;
	Entry_LO = (uint32_t)as->pagetable[LV1_PT_Index][LV2_PT_Index];
	int spl;
    // disable interrupts
	spl = splhigh();
	tlb_random(Entry_HI, Entry_LO);
	splx(spl);
	return 0;
}

/*
 * SMP-specific functions.  Unused in our UNSW configuration.
 */

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("vm tried to do tlb shootdown?!\n");
}