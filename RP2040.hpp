#pragma once
extern "C"
{
#include "hardware/i2c.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
}
#include "II2C.hpp"
#include "IDELAY.hpp"
#include "IGPIO.hpp"
#include "ISPI.hpp"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

class RP2040SPI : public ISPI
{
private:
    spi_inst_t *spi_instance;

public:
    explicit RP2040SPI(spi_inst_t *instance) : spi_instance(instance) {}

    void init()
    {
        spi_init(spi_instance, 1000 * 1000);
        gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    }

    int write(const uint8_t *data, size_t length) override
    {
        int result = spi_write_blocking(spi_instance, data, length);
        return result < 0 ? result : 0;
    }

    int read(uint8_t *data, size_t length) override
    {
        int result = spi_read_blocking(spi_instance, 0, data, length);
        return result < 0 ? result : 0;
    }
};
/**
 * @class RP2040I2C
 * @brief A class to handle I2C communication for the RP2040 microcontroller.
 *
 * This class provides methods to initialize the I2C interface and perform
 * read and write operations.
 *
 * @note This class implements the II2C interface.
 */

/**
 * @brief Constructs an RP2040I2C object with the given I2C instance.
 *
 * @param instance A pointer to the I2C instance to be used.
 */

/**
 * @brief Initializes the I2C interface with a default baud rate of 100000.
 *
 * This method sets the I2C pins to their appropriate functions and enables
 * pull-up resistors.
 */

/**
 * @brief Writes data to the specified I2C address.
 *
 * @param address The I2C address to write to.
 * @param data A pointer to the data to be written.
 * @param length The number of bytes to write.
 * @return int The result of the write operation.
 */

/**
 * @brief Reads data from the specified I2C address.
 *
 * @param address The I2C address to read from.
 * @param data A pointer to the buffer to store the read data.
 * @param length The number of bytes to read.
 * @return int The result of the read operation.
 */

/**
 * @brief Writes a burst of data to the specified I2C address.
 *
 * @param address The I2C address to write to.
 * @param data A pointer to the data to be written.
 * @param length The number of bytes to write.
 * @return int The result of the write burst operation.
 */

/**
 * @brief Reads a burst of data from the specified I2C address.
 *
 * @param address The I2C address to read from.
 * @param data A pointer to the buffer to store the read data.
 * @param length The number of bytes to read.
 * @return int The result of the read burst operation.
 */
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
        int result = i2c_write_blocking(i2c_instance, address, data, length, false);
        return result < 0 ? result : 0; // i2c_write_blocking(i2c_instance, address, data, length, false);
    }

    int read(uint8_t address, uint8_t *data, size_t length) override
    {
        int result = i2c_read_blocking(i2c_instance, address, data, length, false);
        return result < 0 ? result : 0;
    }
};

/**
 * @class RP2040Delay
 * @brief A class that implements the IDelay interface for the RP2040 microcontroller.
 *
 * This class provides a method to introduce a delay in milliseconds using the RP2040's sleep function.
 *
 * @note This class is specific to the RP2040 microcontroller.
 */
class RP2040Delay : public IDelay
{
public:
    void wait_ms(uint32_t milliseconds) override
    {
        sleep_ms(milliseconds);
    }
};

/**
 * @class RP2040GPIO
 * @brief A class to manage GPIO operations on the RP2040 microcontroller.
 *
 * This class provides methods to configure and control GPIO pins on the RP2040.
 * It implements the IGPIO interface.
 *
 * @method void set_high(int pin)
 * Sets the specified GPIO pin to a high state.
 *
 * @param pin The GPIO pin number to set high.
 *
 * @method void set_low(int pin)
 * Sets the specified GPIO pin to a low state.
 *
 * @param pin The GPIO pin number to set low.
 *
 * @method void output_conf(int pin)
 * Configures the specified GPIO pin as an output.
 *
 * @param pin The GPIO pin number to configure as output.
 *
 * @method void input_conf(int pin)
 * Configures the specified GPIO pin as an input and enables the pull-up resistor.
 *
 * @param pin The GPIO pin number to configure as input.
 *
 * @method bool read(int pin)
 * Reads the current state of the specified GPIO pin.
 *
 * @param pin The GPIO pin number to read.
 * @return true if the pin is high, false if the pin is low.
 */
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
