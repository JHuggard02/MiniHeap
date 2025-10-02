// File: heap.s
    .section    __DATA,__data   // Read/write data section on macOS
    .global     _memspace        // Make symbol visible to linker

    .equ        Heapsize, 1024*1024*10/4  // Number of 4-byte words

_memspace:                     // Label pointing to start of heap
    .skip       Heapsize*4           // Reserve Heapsize*4 bytes (1 MB)
