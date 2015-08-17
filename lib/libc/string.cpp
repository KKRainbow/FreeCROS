extern "C" void *memcpy(void *dest, const void *src, int n) {
    __asm__ ("cld\n\t"
            "rep\n\t"
            "movsb"
    ::"c" (n), "S" (src), "D" (dest)
    );
    return dest;
}