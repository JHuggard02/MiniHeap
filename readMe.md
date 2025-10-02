# miniHeap

A lightweight 10MB memory allocator implemented in C with ARM assembly for heap storage management.

## Overview

miniHeap is a custom memory allocation system that provides dynamic memory management through a simple malloc-like interface. The allocator uses a 10MB heap space defined in ARM assembly and implements a first-fit allocation strategy with block metadata tracking.

## Features

- **10MB Heap Space**: Pre-allocated contiguous memory block defined in ARM assembly
- **First-Fit Allocation**: Efficient block searching algorithm
- **Compact Headers**: 4-byte packed headers with allocation status and size tracking
- **Memory Visualization**: Built-in heap state inspection with `show()` function
- **Error Handling**: Errno-based error reporting for allocation failures

## Architecture

### Components

1. **heap.s** - ARM assembly file that reserves 10MB of heap space in the data section
2. **alloc.c** - Core allocator implementation with block management logic
3. **alloc.h** - Header file with type definitions, macros, and function declarations

### Memory Layout

Each allocated block consists of:
```
[4-byte header][payload...]
```

**Header Structure** (packed, 32 bits):
- `w` (30 bits): Size in 4-byte words
- `allocated` (1 bit): Allocation status flag
- `reserved` (1 bit): Unused

## Building

### Prerequisites
- `clang` compiler
- ARM-based macOS system (Apple Silicon)

### Compilation

```bash
make              # Build the project
make run          # Build and execute
make clean        # Remove build artifacts
```

## Usage

### Basic API

```c
void *alloc(int32 bytes);    // Allocate memory
void show();                  // Display heap state
```

### Example

```c
#include "alloc.h"

int main() {
    int8 *p1 = alloc(7);   // Allocate 7 bytes
    int8 *p2 = alloc(10);  // Allocate 10 bytes
    int8 *p3 = alloc(3);   // Allocate 3 bytes
    
    show();  // Visualize heap state
    
    return 0;
}
```

### Sample Output

```
Memory Space = 0x100008000
Alloc 1 = 2 allocated words, addr = 0x100008000
Alloc 2 = 3 allocated words, addr = 0x10000800c
Alloc 3 = 1 allocated words, addr = 0x10000801c
```

## Implementation Details

### Allocation Algorithm

1. **findblock_()**: Recursively searches for the first available block that fits the requested size
2. **mkalloc()**: Initializes block header and marks memory as allocated
3. **alloc()**: Public interface that converts byte requests to word-aligned allocations

### Memory Alignment

All allocations are rounded up to 4-byte (word) boundaries for ARM compatibility.

### Error Codes

- `ErrNoMem` (1): Insufficient memory available
- `ErrUnknown` (2): Unknown allocation error

Errors are reported through `errno`.

## Limitations

- **No deallocation**: Currently implements allocation only (no `free()` equivalent)
- **No coalescing**: Adjacent free blocks are not merged
- **Fixed heap size**: 10MB maximum, defined at compile time
- **Platform-specific**: Designed for ARM macOS systems
- **No thread safety**: Not suitable for concurrent access

## Technical Notes

### Macro Definitions

The codebase uses type-casting macros for brevity:
- `$v` - void pointer cast
- `$h` - header pointer cast
- `$1/$2/$4/$8` - integer type casts

### Heap Bounds

Maximum allocatable words: `(1GB/4)-1` words (theoretical limit, constrained by 10MB heap)

