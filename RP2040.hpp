#pragma once
extern "C"
{
#include "hardware/i2c.h"
#include "pico/time.h"
#include "hardware/gpio.h"
}
#include "II2C.hpp"
#include "IDELAY.hpp"
#include "IGPIO.hpp"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

class RP2040I2C : public II2C
{
private:
    i2c_inst_t *i2c_instance;

public:
    explicit RP2040I2C(i2c_inst_t *instance) : i2c_instance(instance) {}

    void init()
    {
        i2c_init(i2c_instance, 100000);
        gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
        gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_SDA_PIN);
        gpio_pull_up(I2C_SCL_PIN);
    }

    int write(uint8_t address, const uint8_t *data, size_t length) override
    {
        return i2c_write_blocking(i2c_instance, address, data, length, false);
    }

    int read(uint8_t address, uint8_t *data, size_t length) override
    {
        return i2c_read_blocking(i2c_instance, address, data, length, false);
    }

    int write_burst(uint8_t address, const uint8_t *data, size_t length) override
    {
        return i2c_write_burst_blocking(i2c_instance, address, data, length);
    }

    int read_burst(uint8_t address, uint8_t *data, size_t length) override
    {
        return i2c_read_burst_blocking(i2c_instance, address, data, length);
    }
};

class RP2040Delay : public IDelay
{
public:
    void wait_ms(uint32_t milliseconds) override
    {
        sleep_ms(milliseconds);
    }
};

class RP2040GPIO : public IGPIO
{
public:
    void set_high(int pin) override
    {
        gpio_put(pin, true); 
    }

    void set_low(int pin) override
    {
        gpio_put(pin, false); 
 
    }

    void output_conf(int pin) override
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
    }

    void input_conf(int pin) override
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);

    }

    bool read(int pin) override
    {
        return gpio_get(pin);
    }
};
