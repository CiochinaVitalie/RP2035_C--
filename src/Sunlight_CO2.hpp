#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include "II2C.hpp"
#include "IDELAY.hpp"
#include "IGPIO.hpp"

#define DELAY_WAKEUP 35UL
#define DELAY_TIMEOUT 15UL
#define DELAY_SRAM 1UL
#define DELAY_EEPROM 25UL
#define DELAY_ZIRO 0UL

enum class Registers : uint8_t
{
    ErrorStatus = 0x00,
    MeasuredFilteredPc = 0x06,
    Temperature = 0x08,
    MeasurementCount = 0x0D,
    MeasurCycleTime = 0x0E,
    MeasUnfPressCompens = 0x10,
    MeasFiltered = 0x12,
    MeasuredUnfiltered = 0x14,
    FirmwareType = 0x2F,
    FirmwareRev = 0x38,
    SensorId = 0x3A,
    ProductCode = 0x70,
    CalibrationStatus = 0x81,
    CalibrationCommand = 0x82,
    CalibrationTarget = 0x84,
    MeasurementMode_EE = 0x95,
    MeasurementPeriod_EE = 0x96,
    NumberOfSamples_EE = 0x98,
    ABC_Period_EE = 0x9A,
    ABC_Target_EE = 0x9E,
    StaticIIRFilter_EE = 0xA1,
    MeterControl_EE = 0xA5,
    I2C_Address_EE = 0xA7,
    Nominator_EE = 0xA8,
    Denominator_EE = 0xAA,
    Scale_ABC_Target = 0xB0,
    StartMesurement = 0xC3,
    PressureValue = 0xDC,
    ClearErrorStatus = 0x9D,
    Src = 0xA3,

    AbcTime = 0xC4,
    AbcPar0 = 0xC6,
    AbcPar1 = 0xC8,
    AbcPar2 = 0xCA,
    AbcPar3 = 0xCC,
    FiltPar0 = 0xCE,
    FiltPar1 = 0xD0,
    FiltPar2 = 0xD2,
    FiltPar3 = 0xD4,
    FiltPar4 = 0xD6,
    FiltPar5 = 0xD8,
    FiltPar6 = 0xDA,
    BarAirPress = 0xDC,
    ABCBarPress = 0xDE,

};

enum class Error : uint16_t
{
    None = 0,
    Low_internal_err = 1 << 15,
    Meas_timeout_err = 1 << 14,
    Abnor_signal_err = 1 << 13,
    Scale_factor_err = 1 << 8,

    Fatal_err = 1 << 7,
    I2C_err = 1 << 6,
    Algorithm_err = 1 << 5,
    Calibration_err = 1 << 4,
    Self_diag_err = 1 << 3,
    Out_of_range = 1 << 2,
    Memory_err = 1 << 1,
    No_meas_complete = 1 << 0,
};
struct MeasurementData
/**
 * @brief Structure to hold sensor data and status information.
 *
 * This structure contains various fields to store the status and
 * measurement data from a CO₂ sensor, including error status, CO₂
 * concentration, temperature, and measurement counts.
 *
 * @var uint16_t errorstatus
 * Status of the sensor indicating any errors.
 *
 * @var uint16_t co2
 * Measured CO₂ concentration.
 *
 * @var uint16_t temperature
 * Measured temperature.
 *
 * @var uint8_t measurementCount
 * Number of measurements taken.
 *
 * @var uint16_t cycleTime
 * Time taken for one measurement cycle.
 *
 * @var uint16_t measUnfiCompens
 * Unfiltered and uncompensated measurement value.
 *
 * @var uint16_t measuredFiltered
 * Filtered measurement value.
 *
 * @var uint16_t measuredUnfiltered
 * Unfiltered measurement value.
 */
{
    uint16_t errorstatus;
    uint16_t co2;
    uint16_t temperature;
    uint8_t measurementCount;
    uint16_t cycleTime;
    uint16_t measUnfiCompens;
    uint16_t measuredFiltered;
    uint16_t measuredUnfiltered;
};
struct ProductType
/**
 * @brief A structure representing the Sunlight CO₂ sensor data.
 *
 * This structure contains information about the firmware type, firmware revision,
 * sensor ID, and product code of the Sunlight CO₂ sensor.
 *
 * @var uint8_t FirmwareType
 * The type of firmware used by the sensor.
 *
 * @var uint16_t FirmwareRev
 * The revision number of the firmware.
 *
 * @var uint32_t SensorId
 * The unique identifier of the sensor.
 *
 * @var std::array<char, 15> ProductCode
 * The product code of the sensor, stored as a character array.
 */
{
    uint8_t FirmwareType;
    uint16_t FirmwareRev;
    uint32_t SensorId;
    std::array<char, 15> ProductCode;
};

struct Config
/**
 * @brief Configuration structure for Sunlight CO₂ sensor.
 *
 * This structure holds various configuration parameters for the Sunlight CO₂ sensor.
 *
 * @param measurement_mode Mode of measurement (e.g., continuous, single-shot).
 * @param measurement_period Period between measurements in seconds.
 * @param number_of_samples Number of samples to average per measurement.
 * @param abc_period Automatic Baseline Correction (ABC) period in hours.
 * @param abc_target Target CO₂ concentration for ABC in ppm.
 * @param iir_filter IIR filter setting for noise reduction.
 * @param meter_control Control settings for the meter.
 * @param i2c_address I2C address of the sensor.
 * @param nominator Nominator for scaling calculations.
 * @param denominator Denominator for scaling calculations.
 * @param scaled_abc_target Scaled target CO₂ concentration for ABC.
 */
{
    uint8_t measurement_mode;
    uint16_t measurement_period;
    uint16_t number_of_samples;
    uint16_t abc_period;
    uint16_t abc_target;
    uint8_t iir_filter;
    uint8_t meter_control;
    uint8_t i2c_address;
    uint16_t nominator;
    uint16_t denominator;
    uint16_t scaled_abc_target;
};

struct StateData
/**
 * @brief Structure to hold various parameters for ABC (Automatic Baseline Correction) and filtering.
 *
 * This structure contains parameters related to ABC and filtering processes.
 *
 * @var uint16_t abc_time
 *      Time parameter for ABC.
 * @var uint16_t abc_par0
 *      Parameter 0 for ABC.
 * @var uint16_t abc_par1
 *      Parameter 1 for ABC.
 * @var uint16_t abc_par2
 *      Parameter 2 for ABC.
 * @var uint16_t abc_par3
 *      Parameter 3 for ABC.
 * @var uint16_t filt_par0
 *      Parameter 0 for filtering.
 * @var uint16_t filt_par1
 *      Parameter 1 for filtering.
 * @var uint16_t filt_par2
 *      Parameter 2 for filtering.
 * @var uint16_t filt_par3
 *      Parameter 3 for filtering.
 * @var uint16_t filt_par4
 *      Parameter 4 for filtering.
 * @var uint16_t filt_par5
 *      Parameter 5 for filtering.
 * @var uint16_t filt_par6
 *      Parameter 6 for filtering.
 */
{
    uint16_t abc_time;
    uint16_t abc_par0;
    uint16_t abc_par1;
    uint16_t abc_par2;
    uint16_t abc_par3;
    uint16_t filt_par0;
    uint16_t filt_par1;
    uint16_t filt_par2;
    uint16_t filt_par3;
    uint16_t filt_par4;
    uint16_t filt_par5;
    uint16_t filt_par6;
};

class Sunlight_CO₂
{
private:
    II2C *i2c;
    IDelay *delay;
    IGPIO *gpio;
    int en_pin;
    int nrdy_pin;

    uint8_t SENSOR_ADDRESS;

    StateData state_data;
    ProductType product;
    bool little_endian;

    uint8_t I2CWrite(int addr, const void *data, size_t size, unsigned long int timeout = DELAY_SRAM);
    uint8_t I2CRead(int addr, void *result, size_t size, unsigned long int timeout = DELAY_TIMEOUT);
    void swap_endianness(void *data, size_t size);
    bool is_little_endian();

    void sensor_state_data_get();
    void sensor_state_data_set();
    void start_mesure();

    void clear_error_status();
    void read_error_status();
    void product_type_get();
    void get_config();

public:
    Sunlight_CO₂(II2C *i2c_context, IDelay *delay_context, IGPIO *gpio_context, int en_pin, int nrdy_pin)
        : i2c(i2c_context), delay(delay_context), gpio(gpio_context), en_pin(en_pin), nrdy_pin(nrdy_pin), SENSOR_ADDRESS(0x68)
    {
        i2c->init();
        gpio->output_conf(en_pin);
        gpio->input_conf(nrdy_pin);

        little_endian = is_little_endian();

        wake_up();
        clear_error_status();
        product_type_get();
        get_config();
        sensor_state_data_get();
        read_error_status();
        sleep();
    }
    Config config;
    MeasurementData meas_data;

    const char* error_to_string(Error error);
    bool set_config(Config config);
    void sleep();
    void wake_up();
    void reset_sensor();
    uint16_t GetCalibrationTarget();
    void SetCalibrationTarget(uint16_t val);
    bool background_calibration();

    void print_errors();
    void CO2_measurement_get(uint16_t *pressure);

    ~Sunlight_CO₂() {};
};
