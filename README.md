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

