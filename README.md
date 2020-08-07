# Assignment 3

## Coding Assignment
Designing and implementing a number of data-structures and the functions
that manipulate them.

Before start, you should work out what data you need to keep track of 
and what opreations are required.

### Address Space Management
    OS/161 has address space type that encapsulates the book-keeping needed 
    to describe an address space: the **struct addrspace**.
    To enable OS/161 to interact with your VM implementation, you need to implement
    the functions in kern/vm/addrespace.c and potentialy modify the type.
    The semantics of these functions is documented in kern/include/addrespace.h

    ==You may use a fixed-size stack region (say 16 pages) for each process.==

### Address Translation
    The main goal for this assignemnt is to provide virtual memory translation
    for user programs. To do this, you need to implement a TLB refill handler.
    You will also need to implement a page table this is used to store the 
    translations used to refill the TLB.

    You will implement a 2-level hierarchical page table. The first level of 
    the page table is to be indexed using the 12 most significant bits of the
    page number, the second level nodes of the page table are to be indexed using 
    the 8 least significant bits of the page number. Thus the first-level node
    will have 4096(2^12) entries and the second-level nodes will have 256(^8) entries.	

	Note that a hierarchical page table is a lazy data-structure. This means that the contents
	of the page table, including the second-level nodes in the hierarchy, are only allocated when
	they are needed. You may find allocating the required pages to load time helps you start your
	assignment, however, your final solution should allocate pages only when a page-fault occurs.

	The following questions may assist you in designing the contents of your page table
		* What information do you need to store for each page?
		* How does the page table get populated?
		* Is the data structure global or a per-process data structure?
	Note: 	Applications expect pages to contain zeros when first used. This implies that newly allocated 
			frames that are used to back pages should be zero-filled prior to mapping

### Hints
	To implement a page table, have a close look at the dumbvm implmentaton, espcially vm_fault().
	Although it is simple, you should get an idea on how to apporach the rest of the assignment.
 	
	One approach to implementing the assignment is in the following order:
		* Review how the specified page table works form the lectures, and understand its 
		  relationship with the TLB
		* Review the assignment specification and its relationship with the supplied code.
			** dumbvm is not longer complied into the OS/161 kernel for this assignment(kern/arch/mips/vm/dumbvm.c).
				but you can review it as an example implementatuon within the interface/framework you will be 
				working within.
		* Work out a basic design for your page table implementation
		* Modify kern/vm/vm.c to insert, lookup, and update page table entries, and keep the TLB consistent
			with the page table
		* Implement the TLB exception handler vm_fault() in vm.c using your page table.
		* Implement the functions in kern/vm/addrspace.c that are required for basic functionality (e.g. as_create(),
			as_prepare_load(),etc.). Allocating user pages in as_define_region() may also simplify your 
			assignment, however good solution allocate pages in vm_fault().
		* Test and debug this. Use the debugger!
	
	Note: Interrupts should be disabled when writing to the TLB, see dumbvm for an example. 
			Otherwise, unexpected concurrency issues can occur.
	
	as_activate() and as_deactivate() can be copied from dumbvm.).


