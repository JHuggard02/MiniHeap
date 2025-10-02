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