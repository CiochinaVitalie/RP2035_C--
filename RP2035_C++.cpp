extern "C" {
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
}
#define EN_PIN   20
#define NRDY_PIN  21

#include "RP2040.hpp"
#include "Sunlight_CO2.hpp"

bool isLittleEndian() {
    uint16_t number = 0x1; // 16-битное число, где младший байт равен 0x01
    uint8_t *bytePointer = reinterpret_cast<uint8_t*>(&number);

    return *bytePointer == 0x1; // Если младший байт равен 0x01, то little-endian
}

int main()
{
    stdio_init_all();

    RP2040I2C i2c(i2c0);
    RP2040Delay delay;
    RP2040GPIO gpio;

    //delay.wait_ms(2500);



    Sunlight_CO₂ *sensor = new Sunlight_CO₂(&i2c,&delay, &gpio, EN_PIN, NRDY_PIN);

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
