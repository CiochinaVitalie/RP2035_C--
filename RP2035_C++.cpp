extern "C" {
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
}
#define EN_PIN   20
#define NRDY_PIN  21

#include "RP2040.hpp"
#include "Sunlight_CO2.hpp"

uint8_t en_pin = 20;
uint8_t nrdy_pin = 21;

int main()
{
    stdio_init_all();

    RP2040I2C i2c(i2c0);
    RP2040Delay delay;
    RP2040GPIO gpio;

    //delay.wait_ms(2500);



    Sunlight_CO₂ *sensor = new Sunlight_CO₂(&i2c,&delay, &gpio, &en_pin, &nrdy_pin);

    sensor->config.abc_period = 10;
    sensor->config.meter_control &= 0xFD;
    sensor->set_config(sensor->config);
    // sensor->CO2_measurement_get(nullptr);

    while (true) {

        sensor->CO2_measurement_get(nullptr);
        // printf("CO2: %d ppm\n", sensor->meas_data.co2);
        // printf("Temperature: %d C\n", sensor->meas_data.temperature);

        sleep_ms(1000);
    }
}
