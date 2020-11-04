#include "types.h"
#include "mini_uart.h"
#include "printf.h"
#include "irq.h"


void kernel_main() 
{
    uart_init();
    init_printf(0, uart_putc);

    printf("\nRaspberry Pi 3 OS!\nWe are running in EL %d\n\n", get_el());

    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable();

#define pause 50000000

    uart_putc(0, '\r');
    while(1)
    {
        uart_putc(0, 'R');
        delay(pause);
        uart_putc(0, 'u');
        delay(pause);

        uart_putc(0, 'n');
        delay(pause);
        uart_putc(0, 'n');
        delay(pause);

        uart_putc(0, 'i');
        delay(pause);
        uart_putc(0, 'n');
        delay(pause);

        uart_putc(0, 'g');
        delay(pause);
        uart_putc(0, '\r');
        delay(pause);
        printf("       ");
        uart_putc(0, '\r');

    }
    
}