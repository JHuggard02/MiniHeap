#include "alloc.h"

extern uint32_t memspace[];
void* heap_base_ptr = (void*)memspace;

/**
 * @brief Recursively find a free block that can accommodate the requested allocation
 * 
 * Traverses the heap to find either:
 * - An uninitialized block (size_words == 0)
 * - A free block with sufficient space (is_allocated == false && size >= needed)
 * 
 * @param current_header Pointer to the current block header being examined
 * @param words_needed Number of words required for the allocation
 * @param words_traversed Total words traversed from heap start (for bounds checking)
 * @return Pointer to suitable block header, or NULL if out of memory
 */
block_header* find_free_block_(block_header* current_header, word words_needed, word words_traversed)
{
    bool is_suitable;
    void* next_block_addr;
    block_header* next_header;
    word new_traversed_count;

    // Check if we've exceeded available heap space
    if ((words_traversed + words_needed) > (MAX_HEAP_WORDS - 2)) {
        RETURN_ERROR(ERR_NO_MEMORY);
    }

    // Determine if this block is suitable
    is_suitable = (!(current_header->size_words)) ? true :
        (!(current_header->is_allocated) && (current_header->size_words >= words_needed)) ? true :
        false;

    if (is_suitable) {
        return current_header;
    } else {
        // Move to next block: current_address + (block_size * 4 bytes) + 4 bytes for header
        next_block_addr = CAST_VOID(current_header) + (current_header->size_words * 4) + 4;
        next_header = CAST_HEADER(next_block_addr);
        new_traversed_count = words_traversed + current_header->size_words;

        return find_free_block_(next_header, words_needed, new_traversed_count);
    }

    RETURN_ERROR(ERR_UNKNOWN);
}

/**
 * @brief Mark a block as allocated and return pointer to usable memory
 * 
 * Sets the block header's size and allocation status, then returns
 * a pointer to the memory immediately after the header.
 * 
 * @param words_to_allocate Number of words to mark as allocated
 * @param target_header Pointer to the block header to initialize
 * @return Pointer to usable memory (4 bytes after header), or NULL on error
 */
void* mark_allocated(word words_to_allocate, block_header* target_header)
{
    void* usable_memory;
    void* offset_from_base;
    uintptr_t bytes_from_base;
    word words_from_base;

    // Calculate how far this block is from heap start
    offset_from_base = CAST_VOID(CAST_VOID(target_header) - heap_base_ptr);
    bytes_from_base = (uintptr_t)offset_from_base;
    words_from_base = (word)(bytes_from_base / 4) + 1;

    // Verify we have enough space remaining
    if (words_to_allocate > MAX_HEAP_WORDS - words_from_base) {
        RETURN_ERROR(ERR_NO_MEMORY);
    }

    // Initialize block header
    target_header->size_words = words_to_allocate;
    target_header->is_allocated = true;
    
    // Return pointer to usable memory (skip 4-byte header)
    usable_memory = CAST_VOID(target_header) + 4;

    return usable_memory;
}

/**
 * @brief Allocate memory from the heap
 * 
 * Finds a suitable free block and marks it as allocated. Converts byte
 * request to word-aligned allocation (rounds up to nearest 4-byte boundary).
 * 
 * @param bytes Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL if allocation fails
 */
void* alloc(int32 bytes) 
{
    word words_needed;
    block_header* target_block;
    void* allocated_memory;

    // Convert bytes to words (round up if not 4-byte aligned)
    words_needed = (!(bytes % 4)) ? bytes / 4 : (bytes / 4) + 1;

    // Find a suitable block
    target_block = FIND_FREE_BLOCK(words_needed);
    if (!target_block) return CAST_VOID(0);

    // Verify request is within heap limits
    if (words_needed > MAX_HEAP_WORDS) {
        RETURN_ERROR(ERR_NO_MEMORY);
    }

    // Mark block as allocated
    allocated_memory = mark_allocated(words_needed, target_block);
    if (!allocated_memory) return CAST_VOID(0);

    return allocated_memory;
}

/**
 * @brief Free previously allocated memory
 * 
 * Marks the block as free by setting is_allocated to false. The memory
 * is not cleared but can be reused by future allocations.
 * 
 * @param ptr Pointer to memory to free (must be from alloc/calloc/realloc)
 */
void free(void* ptr)
{
    block_header* block_hdr;
    
    if (!ptr) return;
    
    // Get header (located 4 bytes before the returned pointer)
    block_hdr = CAST_HEADER(CAST_VOID(ptr) - 4);
    block_hdr->is_allocated = false;
    
    return;
}

/**
 * @brief Allocate and zero-initialize memory for an array
 * 
 * Allocates memory for an array of elements and initializes all bytes to zero.
 * 
 * @param count Number of elements to allocate
 * @param element_size Size of each element in bytes
 * @return Pointer to allocated and zeroed memory, or NULL on failure
 */
void* calloc(int32 count, int32 element_size)
{
    void* ptr;
    int32 total_bytes;
    
    total_bytes = count * element_size;
    ptr = alloc(total_bytes);
    
    if (!ptr) return CAST_VOID(0);
    
    // Zero out the allocated memory
    memset(ptr, 0, total_bytes);
    
    return ptr;
}

/**
 * @brief Resize a previously allocated memory block
 * 
 * Changes the size of the memory block. If the new size is smaller than
 * or equal to the old size, returns the same pointer. Otherwise, allocates
 * a new block, copies the old data, and frees the old block.
 * 
 * @param ptr Pointer to memory to resize (NULL to allocate new block)
 * @param new_bytes New size in bytes (0 to free the block)
 * @return Pointer to resized memory, or NULL on failure
 */
void* realloc(void* ptr, int32 new_bytes)
{
    block_header* old_header;
    void* new_ptr;
    word old_size_words;
    word new_size_words;
    int32 bytes_to_copy;
    
    if (!ptr) {
        return alloc(new_bytes);
    }
    
    if (new_bytes == 0) {
        free(ptr);
        return CAST_VOID(0);
    }
    
    // Get old block information
    old_header = CAST_HEADER(CAST_VOID(ptr) - 4);
    old_size_words = old_header->size_words;
    new_size_words = (!(new_bytes % 4)) ? new_bytes / 4 : (new_bytes / 4) + 1;
    
    // If new size fits in old block, reuse it
    if (new_size_words <= old_size_words) {
        return ptr;
    }
    
    new_ptr = alloc(new_bytes);
    if (!new_ptr) return CAST_VOID(0);
    
    bytes_to_copy = old_size_words * 4;
    memcpy(new_ptr, ptr, bytes_to_copy);
    
    free(ptr);
    
    return new_ptr;
}

/**
 * @brief Get the size of an allocated block in bytes
 * 
 * Returns the size of the memory block pointed to by ptr. This is the
 * actual allocated size (in words * 4), which may be larger than requested.
 * 
 * @param ptr Pointer to allocated memory
 * @return Size of the block in bytes, or 0 if ptr is NULL
 */
int32 get_block_size(void* ptr)
{
    block_header* block_hdr;
    
    if (!ptr) return 0;
    
    block_hdr = CAST_HEADER(CAST_VOID(ptr) - 4);
    
    return block_hdr->size_words * 4;
}

/**
 * @brief Gather statistics about heap usage
 * 
 * Traverses all blocks in the heap and accumulates statistics about
 * memory usage. All output parameters are optional (can be NULL).
 * 
 * @param total_words Output: total words in all blocks (can be NULL)
 * @param allocated_words Output: words currently allocated (can be NULL)
 * @param free_words Output: words currently free (can be NULL)
 * @param num_blocks Output: total number of blocks (can be NULL)
 */
void heap_stats(int32* total_words, int32* allocated_words, int32* free_words, int32* num_blocks)
{
    block_header* current_block;
    void* next_addr;
    int32 total = 0;
    int32 allocated = 0;
    int32 free_space = 0;
    int32 blocks = 0;
    
    // Traverse all blocks until we hit an uninitialized block (size_words == 0)
    for (current_block = CAST_HEADER(heap_base_ptr); 
         current_block->size_words; 
         next_addr = CAST_VOID(current_block) + ((current_block->size_words + 1) * 4), 
         current_block = CAST_HEADER(next_addr)) {
        
        total += current_block->size_words;
        blocks++;
        
        if (current_block->is_allocated) {
            allocated += current_block->size_words;
        } else {
            free_space += current_block->size_words;
        }
    }
    
    // Set output parameters if provided
    if (total_words) *total_words = total;
    if (allocated_words) *allocated_words = allocated;
    if (free_words) *free_words = free_space;
    if (num_blocks) *num_blocks = blocks;
    
    return;
}

/**
 * @brief Display heap structure for debugging
 * 
 * Prints information about each block in the heap including its number,
 * size, allocation status, and memory address.
 * 
 * @param start_header Pointer to the first block header in the heap
 */
void show_heap_(block_header* start_header) {
    block_header* current_block;
    void* next_addr;
    int32 block_number;
    
    for (block_number = 1, current_block = start_header; 
         current_block->size_words; 
         next_addr = CAST_VOID(current_block) + ((current_block->size_words + 1) * 4), 
         current_block = next_addr, 
         block_number++) {
        
        printf("Block %d = %d %s words, addr = 0x%lx\n",
            block_number, 
            current_block->size_words, 
            (current_block->is_allocated) ? "allocated" : "free", 
            (uintptr_t)current_block);
    }

    return;
}

/**
 * @brief Test function demonstrating the heap allocator
 * 
 * Allocates three blocks of memory and displays the heap structure.
 */
int main(int argc, char* argv[])
{
    int8* ptr1;
    int8* ptr2;
    int8* ptr3;

    printf("Memory Space Base Address = 0x%lx\n", (uintptr_t)memspace);

    ptr1 = alloc(7);
    ptr2 = alloc(10);
    ptr3 = alloc(3);

    SHOW_HEAP();

    return 0;
}