extern "C" {
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
}
#define EN_PIN   20
#define NRDY_PIN  21

#include "RP2040.hpp"
#include "Sunlight_CO₂.hpp"



int main()
{
    stdio_init_all();

    RP2040I2C i2c(i2c0);
    RP2040Delay delay;
    RP2040GPIO gpio;

    delay.wait_ms(2500);

    Sunlight_CO₂ *sensor = new Sunlight_CO₂(&i2c,&delay, &gpio, EN_PIN, NRDY_PIN);
    sensor->CO2_measurement_get(nullptr);

    while (true) {
        sleep_ms(1000);
    }
}
