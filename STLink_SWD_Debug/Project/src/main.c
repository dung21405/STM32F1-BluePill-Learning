#include <stdint.h>
#include <stdio.h>

volatile uint32_t known_var;

__attribute__((noinline))

void checkpoint(void)
{
    __asm volatile ("nop");
}

int main(void)
{
    known_var = 0x12345678u;
    checkpoint();

    for(;;)
    {
        __asm volatile ("nop");
    }
}