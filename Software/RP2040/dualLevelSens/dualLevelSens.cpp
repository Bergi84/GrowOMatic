#include <stdio.h>
#include "pico/stdlib.h"
#include "dualLevelSens.h"
#include "capSens.h"

capSens<gpio_capSens_chNo> gCapSens;

extern "C"
{
    int main() 
    {
        gCapSens.init(pio0, gpio_capSensExc_base, gpio_capSens_base, 0x000007FF, 62500, 125);
    }
}