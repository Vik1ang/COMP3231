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



