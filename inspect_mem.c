#include <malloc.h>

wint_t getTotalHeap(void) {
   extern char __StackLimit, __bss_end__;
   
   return &__StackLimit  - &__bss_end__;
}

wint_t getFreeHeap(void) {
   struct mallinfo m = mallinfo();

   return getTotalHeap() - m.uordblks;
}