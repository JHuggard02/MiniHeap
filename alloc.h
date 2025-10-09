#ifndef ALLOC_H
#define ALLOC_H

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

#define packed __attribute__((__packed__))
#define unused __attribute__((__unused__))
#define MAX_HEAP_WORDS ((1024*1024*1024/4)-1)

#define ERR_NO_MEMORY 1
#define ERR_UNKNOWN 2

typedef unsigned char int8;
typedef unsigned short int int16;
typedef unsigned int int32;
typedef unsigned long long int int64;
typedef void heap;
typedef int32 word;

/**
 * @brief Header structure for memory blocks in the heap
 * 
 * Each allocated or free block has a header that tracks:
 * - size_words: Size of the block in 4-byte words (30 bits)
 * - is_allocated: Whether the block is currently allocated (1 bit)
 * - reserved: Reserved for future use (1 bit)
 */
struct packed block_header {
    word size_words:30;      
    bool is_allocated:1;     
    bool unused reserved:1; 
};
typedef struct packed block_header block_header;

#define CAST_INT8(x)((int8 *)(x))
#define CAST_INT16(x)((int16)(x))
#define CAST_INT32(x)((int32)(x))
#define CAST_INT64(x)((int64)(x))
#define CAST_CHAR(x)((char *)(x))
#define CAST_INT(x)((int)(x))
#define CAST_UINT32(x) ((uint32_t)(x))
#define CAST_VOID(x) ((void *)(x))
#define CAST_HEADER(x)((block_header *)(x))

#define RETURN_ERROR(err_code) do { errno = (err_code); return NULL; } while(0)
#define FIND_FREE_BLOCK(words) find_free_block_((block_header*)heap_base_ptr, (words), 0)
#define SHOW_HEAP() show_heap_(CAST_HEADER(heap_base_ptr))

/**
 * @brief Find a free block in the heap that can fit the requested allocation
 * 
 * @param current_header Pointer to current block header being examined
 * @param words_needed Number of words needed for allocation
 * @param words_traversed Number of words traversed so far
 * @return Pointer to suitable block header, or NULL if none found
 */
block_header* find_free_block_(block_header* current_header, word words_needed, word words_traversed);

/**
 * @brief Mark a block as allocated and return pointer to usable memory
 * 
 * @param words_to_allocate Number of words to allocate
 * @param target_header Pointer to the block header to mark as allocated
 * @return Pointer to usable memory (after header), or NULL on error
 */
void* mark_allocated(word words_to_allocate, block_header* target_header);

/**
 * @brief Allocate memory from the heap
 * 
 * @param bytes Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* alloc(int32 bytes);

/**
 * @brief Free previously allocated memory
 * 
 * @param ptr Pointer to memory to free
 */
void free(void *ptr);

/**
 * @brief Allocate and zero-initialize memory for an array
 * 
 * @param count Number of elements
 * @param size Size of each element in bytes
 * @return Pointer to allocated and zeroed memory, or NULL on failure
 */
void* calloc(int32 count, int32 size);

/**
 * @brief Resize a previously allocated memory block
 * 
 * @param ptr Pointer to memory to resize (or NULL to allocate new)
 * @param new_bytes New size in bytes
 * @return Pointer to resized memory, or NULL on failure
 */
void* realloc(void *ptr, int32 new_bytes);

/**
 * @brief Get the size of an allocated block
 * 
 * @param ptr Pointer to allocated memory
 * @return Size of the block in bytes, or 0 if ptr is NULL
 */
int32 get_block_size(void *ptr);

/**
 * @brief Gather statistics about heap usage
 * 
 * @param total_words Output: total words in all blocks
 * @param allocated_words Output: words currently allocated
 * @param free_words Output: words currently free
 * @param num_blocks Output: total number of blocks
 */
void heap_stats(int32 *total_words, int32 *allocated_words, int32 *free_words, int32 *num_blocks);

/**
 * @brief Display heap structure for debugging
 * 
 * @param start_header Pointer to first block header
 */
void show_heap_(block_header* start_header);

/**
 * @brief Main function for testing the allocator
 */
int main(int argc, char** argv);

#endif 