#include "alloc.h"

extern uint32_t memspace[];
void* heap_ptr = (void*)memspace;

header *findblock_(header *hdr, word allocation, word n)
{
    bool ok;
    void *mem;
    header *hdr_;
    word n_;

    if ((n + allocation) > (Maxwords - 2)){
        reterr(ErrNoMem);
    }

    ok = (!(hdr->w)) ? true :
        (!(hdr->allocated) && (hdr->w >= allocation)) ? true :
        false;

    if (ok){
        return hdr;
    }else{
        mem = $v hdr + (hdr->w * 4) + 4;
        hdr_ = $h mem;
        n_ = n + hdr->w;

        return findblock_(hdr_, allocation, n_);
    }

    reterr(ErrUnknown);
}

void *mkalloc(word words, header *hdr)
{
    void *ret;
    void *bytesin;
    word wordsin;

    bytesin = ($v (($v hdr) - heap_ptr));
    uintptr_t bytesin_int = (uintptr_t)bytesin;
    wordsin = (word)((bytesin_int)/4) + 1;

    if (words > Maxwords - wordsin) {
        reterr(ErrNoMem);
    }

    hdr->w = words;
    hdr->allocated = true;
    ret = ($v hdr) + 4;

    return ret;
}

void *alloc(int32 bytes) 
{
    word words;
    header *hdr;
    void *mem;

    words = (!(bytes %4)) ? bytes / 4 : (bytes / 4) + 1;

    hdr = findblock(words);
    if (!hdr) return $v 0;

    if (words > Maxwords) {
        reterr(ErrNoMem);
    }

    mem = mkalloc(words, hdr);
    if (!mem) return $v 0;

    return mem;

}

void free(void *ptr)
{
    header *hdr;
    
    if (!ptr) return;
    
    hdr = $h (($v ptr) - 4);
    hdr->allocated = false;
    
    return;
}

void *calloc(int32 count, int32 size)
{
    void *ptr;
    int32 total_bytes;
    
    total_bytes = count * size;
    ptr = alloc(total_bytes);
    
    if (!ptr) return $v 0;
    
    memset(ptr, 0, total_bytes);
    
    return ptr;
}

void *realloc(void *ptr, int32 new_bytes)
{
    header *old_hdr;
    void *new_ptr;
    word old_words;
    word new_words;
    int32 copy_bytes;
    
    if (!ptr) {
        return alloc(new_bytes);
    }
    
    if (new_bytes == 0) {
        free(ptr);
        return $v 0;
    }
    
    old_hdr = $h (($v ptr) - 4);
    old_words = old_hdr->w;
    new_words = (!(new_bytes % 4)) ? new_bytes / 4 : (new_bytes / 4) + 1;
    
    if (new_words <= old_words) {
        return ptr;
    }
    
    new_ptr = alloc(new_bytes);
    if (!new_ptr) return $v 0;
    
    copy_bytes = old_words * 4;
    memcpy(new_ptr, ptr, copy_bytes);
    
    free(ptr);
    
    return new_ptr;
}

int32 get_block_size(void *ptr)
{
    header *hdr;
    
    if (!ptr) return 0;
    
    hdr = $h (($v ptr) - 4);
    
    return hdr->w * 4;
}

void heap_stats(int32 *total_words, int32 *allocated_words, int32 *free_words, int32 *num_blocks)
{
    header *p;
    void *mem;
    int32 total = 0;
    int32 allocated = 0;
    int32 free_space = 0;
    int32 blocks = 0;
    
    for (p = $h heap_ptr; p->w; mem = $v p + ((p->w+1)*4), p = $h mem) {
        total += p->w;
        blocks++;
        
        if (p->allocated) {
            allocated += p->w;
        } else {
            free_space += p->w;
        }
    }
    
    if (total_words) *total_words = total;
    if (allocated_words) *allocated_words = allocated;
    if (free_words) *free_words = free_space;
    if (num_blocks) *num_blocks = blocks;
    
    return;
}

void show_(header *hdr) {
    header *p;
    void *mem;
    int32 n;
    
    for (n = 1, p = hdr; p->w; mem = $v p + ((p->w+1)*4), p=mem, n++){
        printf("Alloc %d = %d %s words, addr = 0x%lx\n",
            n, p->w, (p->allocated) ? "allocated" : "free", (uintptr_t) p);
    }

    return;
}

int main(int argc, char *argv[])
{
    int8 *p1;
    int8 *p2;
    int8 *p3;

    printf("Memory Space = 0x%lx\n", (uintptr_t) memspace);

    p1 = alloc(7);
    p2 = alloc(10);
    p3 = alloc(3);

    show();

    return 0;
}