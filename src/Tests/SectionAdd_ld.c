


#include "LIB/stdtypes.h"
#include "test.h"

// Non volatile so it will be 10 
volatile uint32_t __attribute__((section(".led_cfg_flash_section")))  LED_CFG_Flash =10;
// volatile so it will be garbage
uint32_t __attribute__((section(".led_cfg_ram_section")))  LED_CFG_Ram = 10 ;


void testLinkerScript(){
    printf("%x",&LED_CFG_Flash);
    printf("%d",LED_CFG_Ram);
    // ignore editing
    LED_CFG_Flash = 30;
    // editing successfully
    LED_CFG_Ram   = 30;
}