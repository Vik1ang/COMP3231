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
int
vm_fault(int faulttype, vaddr_t faultaddress)
{
    // // (void) faulttype;
    // (void) faultaddress;


    // get the pagenumber section 
    faultaddress &= PAGE_FRAME;
    // TODO: offset and index
    // offset
    // paddr_t offset = faultaddress & 0xfff;
    // thanks week9 tutorial, get idea from there
    // get highest 10 bits (32 -10 = 22)
    // level1 page table index
    paddr_t LV1_PT = faultaddress >> 22;
    // get next 10 bits
    // level2 page table index
    paddr_t LV2_PT = (faultaddress >> 12) & 0x3ff;

    // TODO: if VM_FAULT_READONLY
    if(faulttype == VM_FAULT_READONLY) return EFAULT;

    // if it is not READONLY, lookup pagetable

    if(curproc == NULL) return EFAULT;
    // check whether the faulting address is a valid user space address.
    struct addrspace *as;
    as = proc_getas();
    if(as == NULL) return EFAULT;

    // if valid translation
    // if translation is valid load tlb
    if(as->pagetable[LV1_PT][LV2_PT] != 0){
        int spl;
        // disable interrupts
        spl = splhigh();
        uint32_t Ads = as->pagetable[LV1_PT][LV2_PT];
        tlb_random(faultaddress, Ads);
        splx(spl);
        return 0;
    }

    // else translation is invalid 
    // therefore, Lookup region 
    struct region *reg = as->region;
    // search region
    while (reg != NULL) {
        if(faultaddress >= reg->Addr && faultaddress <= reg->Addr + STACK_SIZE){
            break;
        }
        reg = reg->next;
    }
    // if region is invalid return EFAULT
    if(reg == NULL) return EFAULT;

    // else 
    // allocate frame
    // zero-fill
    // insert page table entry
    
    // allocate frame
    vaddr_t frame_no = alloc_kpages(1);
    // zero fill 
    bzero((void *)frame_no,PAGE_SIZE);

    paddr_t Ads = KVADDR_TO_PADDR(frame_no);
    // insert
    as->pagetable[LV1_PT][LV2_PT] = faultaddress;

    // 
    int spl;
    spl = splhigh();
    tlb_random(faultaddress & PAGE_FRAME, Ads);
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

