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
{
    uint8_t FirmwareType;
    uint16_t FirmwareRev;
    uint32_t SensorId;
    std::array<char, 15> ProductCode;
};

struct Config
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

    static constexpr uint8_t SENSOR_ADDRESS = 0x68;
    uint8_t state_buffer[24];

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
    void sleep();
    void wake_up();
    void clear_error_status();
    void read_error_status();
    void product_type_get();
    void get_config();
    bool is_error_active(Error error) const;

public:
    Sunlight_CO₂(II2C *i2c_context, IDelay *delay_context, IGPIO *gpio_context, int en_pin, int nrdy_pin)
        : i2c(i2c_context), delay(delay_context), gpio(gpio_context), en_pin(en_pin), nrdy_pin(nrdy_pin)
    {
        i2c->init();
        gpio->output_conf(en_pin);
        gpio->input_conf(nrdy_pin);
        little_endian = is_little_endian();

        wake_up();
        clear_error_status();
        product_type_get();
        sensor_state_data_get();
        read_error_status();
        sleep();
    }
    Config config;
    MeasurementData meas_data;
    std::string error_to_string(Error error);
  
    bool set_config(Config config);
    void reset_sensor();
    void CO2_measurement_get(uint16_t *pressure);

    ~Sunlight_CO₂() {};
};
