#include "Sunlight_CO₂.hpp"

uint8_t Sunlight_CO₂::I2CWrite(int addr, const void *data, size_t size, unsigned long int timeout)
{
    int err = i2c->write(addr, reinterpret_cast<const uint8_t *>(data), size);
    if (err < 0)
    {
        err = i2c->write(addr, reinterpret_cast<const uint8_t *>(data), size);
    }
    delay->wait_ms(timeout);
    return err;
}

uint8_t Sunlight_CO₂::I2CRead(int addr, void *data, size_t size, unsigned long int timeout)
{
    return i2c->read(addr, reinterpret_cast<uint8_t *>(data), size);
}

void Sunlight_CO₂::sleep()
{
    gpio->set_low(en_pin);
}

void Sunlight_CO₂::wake_up()
{
    gpio->set_high(en_pin);
    delay->wait_ms(35);
}

bool Sunlight_CO₂::is_error_active(Error error) const
{
    return (meas_data.errorstatus & static_cast<uint16_t>(error));
}

bool Sunlight_CO₂::is_little_endian() {
    uint32_t test = 0x01020304;
    uint8_t *ptr = reinterpret_cast<uint8_t*>(&test);

    return *ptr == 0x04; 
}

void Sunlight_CO₂::swap_endianness(void *data, size_t size)
{
    if (size > 1 && little_endian)
    {
        auto *data_ptr = static_cast<uint8_t *>(data);
        std::reverse(data_ptr, data_ptr + size);
    }
}

std::string Sunlight_CO₂::error_to_string(Error error)
{
    switch (error)
    {
    case Error::None:
        return "No error \r\n";
    case Error::Low_internal_err:
        return "Low internal error \r\n";
    case Error::Meas_timeout_err:
        return "Measurement timeout error \r\n";
    case Error::Abnor_signal_err:
        return "Abnormal signal error \r\n";
    case Error::Scale_factor_err:
        return "Scale factor error \r\n";
    case Error::Fatal_err:
        return "Fatal error \r\n";
    case Error::I2C_err:
        return "I2C communication error \r\n";
    case Error::Algorithm_err:
        return "Algorithm error \r\n";
    case Error::Calibration_err:
        return "Calibration error \r\n";
    case Error::Self_diag_err:
        return "Self-diagnostic error \r\n";
    case Error::Out_of_range:
        return "Out of range error \r\n";
    case Error::Memory_err:
        return "Memory error \r\n";
    case Error::No_meas_complete:
        return "No measurement complete \r\n";
    default:
        return "Unknown error \r\n";
    }
}
/**
 * @brief Retrieves the product type information from the sensor.
 *
 * This function reads various registers from the sensor to obtain
 * the firmware type, firmware revision, sensor ID, and product code.
 * It uses I2C communication to perform the read operations.
 *
 * The function defines a structure `RegisterRead` to hold the register
 * address, data pointer, and size of the data to be read. An array of
 * `RegisterRead` structures is created to specify the registers and
 * corresponding data fields to be read.
 *
 * For each register in the array, the function writes the register address
 * to the sensor and then reads the data from the sensor. If the size of the
 * data to be read matches the size of the product code, a burst read is
 * performed; otherwise, a normal read is performed.
 *
 * @note The function assumes that the I2C communication object `i2c` and
 *       the sensor address `SENSOR_ADDRESS` are properly initialized and
 *       available in the scope of the function.
 */
void Sunlight_CO₂::product_type_get()
{

    struct RegisterRead
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterRead reads[] = {
        {static_cast<uint8_t>(Registers::FirmwareType), &product.FirmwareType, sizeof(product.FirmwareType)},
        {static_cast<uint8_t>(Registers::FirmwareRev), &product.FirmwareRev, sizeof(product.FirmwareRev)},
        {static_cast<uint8_t>(Registers::SensorId), &product.SensorId, sizeof(product.SensorId)},
    };

    for (const auto read : reads)
    {

        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, read.data, read.size);
        swap_endianness(read.data, read.size);
    }

    uint8_t main_revision = static_cast<uint8_t>((product.FirmwareRev >> 8) & 0xFF);
    uint8_t sub_revision = static_cast<uint8_t>(product.FirmwareRev & 0xFF);

    if (product.FirmwareType >= 4 && product.FirmwareRev >= 8)
    {
        uint8_t product_code_reg = static_cast<uint8_t>(Registers::ProductCode);
        I2CWrite(SENSOR_ADDRESS, &product_code_reg, sizeof(product_code_reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, product.ProductCode.data(), product.ProductCode.size());
    }
}
/**
 * @brief Reads the configuration settings from the sensor registers and stores them in the config structure.
 *
 * This function reads various configuration parameters from the sensor's EEPROM registers and updates the
 * corresponding fields in the config structure. The parameters include measurement mode, measurement period,
 * number of samples, ABC (Automatic Baseline Correction) period and target, IIR (Infinite Impulse Response) filter
 * settings, meter control, I2C address, nominator, denominator, and scaled ABC target.
 *
 * The function uses the I2C interface to communicate with the sensor. For each configuration parameter, it writes
 * the register address to the sensor and then reads the corresponding data into the config structure.
 */
void Sunlight_CO₂::get_config()
{

    struct RegisterRead
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterRead reads[] = {
        {static_cast<uint8_t>(Registers::MeasurementMode_EE), &config.measurement_mode, sizeof(config.measurement_mode)},
        {static_cast<uint8_t>(Registers::MeasurementPeriod_EE), &config.measurement_period, sizeof(config.measurement_period)},
        {static_cast<uint8_t>(Registers::NumberOfSamples_EE), &config.number_of_samples, sizeof(config.number_of_samples)},
        {static_cast<uint8_t>(Registers::ABC_Period_EE), &config.abc_period, sizeof(config.abc_period)},
        {static_cast<uint8_t>(Registers::ABC_Target_EE), &config.abc_target, sizeof(config.abc_target)},
        {static_cast<uint8_t>(Registers::StaticIIRFilter_EE), &config.iir_filter, sizeof(config.iir_filter)},
        {static_cast<uint8_t>(Registers::MeterControl_EE), &config.meter_control, sizeof(config.meter_control)},
        {static_cast<uint8_t>(Registers::I2C_Address_EE), &config.i2c_address, sizeof(config.i2c_address)},
        {static_cast<uint8_t>(Registers::Nominator_EE), &config.nominator, sizeof(config.nominator)},
        {static_cast<uint8_t>(Registers::Denominator_EE), &config.denominator, sizeof(config.denominator)},
        {static_cast<uint8_t>(Registers::Scale_ABC_Target), &config.scaled_abc_target, sizeof(config.scaled_abc_target)}};

    for (const auto read : reads)
    {
        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, read.data, read.size);
        swap_endianness(read.data, read.size);
    }
}

bool Sunlight_CO₂::set_config(Config set_config)
{

    struct RegisterRead
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterRead reads[] = {
        {static_cast<uint8_t>(Registers::MeasurementMode_EE), &set_config.measurement_mode, sizeof(set_config.measurement_mode)},
        {static_cast<uint8_t>(Registers::MeasurementPeriod_EE), &set_config.measurement_period, sizeof(set_config.measurement_period)},
        {static_cast<uint8_t>(Registers::NumberOfSamples_EE), &set_config.number_of_samples, sizeof(set_config.number_of_samples)},
        {static_cast<uint8_t>(Registers::ABC_Period_EE), &set_config.abc_period, sizeof(set_config.abc_period)},
        {static_cast<uint8_t>(Registers::ABC_Target_EE), &set_config.abc_target, sizeof(set_config.abc_target)},
        {static_cast<uint8_t>(Registers::StaticIIRFilter_EE), &set_config.iir_filter, sizeof(set_config.iir_filter)},
        {static_cast<uint8_t>(Registers::MeterControl_EE), &set_config.meter_control, sizeof(set_config.meter_control)},
        {static_cast<uint8_t>(Registers::I2C_Address_EE), &set_config.i2c_address, sizeof(set_config.i2c_address)},
        {static_cast<uint8_t>(Registers::Nominator_EE), &set_config.nominator, sizeof(set_config.nominator)},
        {static_cast<uint8_t>(Registers::Denominator_EE), &set_config.denominator, sizeof(set_config.denominator)},
        {static_cast<uint8_t>(Registers::Scale_ABC_Target), &set_config.scaled_abc_target, sizeof(set_config.scaled_abc_target)}
        };

    for (const auto read : reads)
    {
        uint8_t current_value[read.size];
        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, current_value, read.size);

        if (read.size > 1)
        {
            swap_endianness(current_value, read.size);
            swap_endianness(read.data, read.size);
  
        }

        if (memcmp(current_value, read.data, read.size) != 0) {
            I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
            I2CWrite(SENSOR_ADDRESS, reinterpret_cast<const uint8_t*>(read.data), read.size, DELAY_EEPROM);
        }
    }

    return true;
}

void Sunlight_CO₂::sensor_state_data_get()
{
    struct RegisterRead
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterRead reads[] = {
        {static_cast<uint8_t>(Registers::AbcTime), &state_data.abc_time, sizeof(state_data.abc_time)},
        {static_cast<uint8_t>(Registers::AbcPar0), &state_data.abc_par0, sizeof(state_data.abc_par0)},
        {static_cast<uint8_t>(Registers::AbcPar1), &state_data.abc_par1, sizeof(state_data.abc_par1)},
        {static_cast<uint8_t>(Registers::AbcPar2), &state_data.abc_par2, sizeof(state_data.abc_par2)},
        {static_cast<uint8_t>(Registers::AbcPar3), &state_data.abc_par3, sizeof(state_data.abc_par3)},
        {static_cast<uint8_t>(Registers::FiltPar0), &state_data.filt_par0, sizeof(state_data.filt_par0)},
        {static_cast<uint8_t>(Registers::FiltPar1), &state_data.filt_par1, sizeof(state_data.filt_par1)},
        {static_cast<uint8_t>(Registers::FiltPar2), &state_data.filt_par2, sizeof(state_data.filt_par2)},
        {static_cast<uint8_t>(Registers::FiltPar3), &state_data.filt_par3, sizeof(state_data.filt_par3)},
        {static_cast<uint8_t>(Registers::FiltPar4), &state_data.filt_par4, sizeof(state_data.filt_par4)},
        {static_cast<uint8_t>(Registers::FiltPar5), &state_data.filt_par5, sizeof(state_data.filt_par5)},
        {static_cast<uint8_t>(Registers::FiltPar6), &state_data.filt_par6, sizeof(state_data.filt_par6)}

    };
    for (const auto read : reads)
    {
        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(read.data), read.size);
        swap_endianness(read.data, read.size);

    }
}

void Sunlight_CO₂::sensor_state_data_set()
{
    struct RegisterRead
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterRead writes[] = {
        {static_cast<uint8_t>(Registers::AbcTime), &state_data.abc_time, sizeof(state_data.abc_time)},
        {static_cast<uint8_t>(Registers::AbcPar0), &state_data.abc_par0, sizeof(state_data.abc_par0)},
        {static_cast<uint8_t>(Registers::AbcPar1), &state_data.abc_par1, sizeof(state_data.abc_par1)},
        {static_cast<uint8_t>(Registers::AbcPar2), &state_data.abc_par2, sizeof(state_data.abc_par2)},
        {static_cast<uint8_t>(Registers::AbcPar3), &state_data.abc_par3, sizeof(state_data.abc_par3)},
        {static_cast<uint8_t>(Registers::FiltPar0), &state_data.filt_par0, sizeof(state_data.filt_par0)},
        {static_cast<uint8_t>(Registers::FiltPar1), &state_data.filt_par1, sizeof(state_data.filt_par1)},
        {static_cast<uint8_t>(Registers::FiltPar2), &state_data.filt_par2, sizeof(state_data.filt_par2)},
        {static_cast<uint8_t>(Registers::FiltPar3), &state_data.filt_par3, sizeof(state_data.filt_par3)},
        {static_cast<uint8_t>(Registers::FiltPar4), &state_data.filt_par4, sizeof(state_data.filt_par4)},
        {static_cast<uint8_t>(Registers::FiltPar5), &state_data.filt_par5, sizeof(state_data.filt_par5)},
        {static_cast<uint8_t>(Registers::FiltPar6), &state_data.filt_par6, sizeof(state_data.filt_par6)}

    };
    for (const auto write : writes)
    {
        swap_endianness(write.data, write.size);
        I2CWrite(SENSOR_ADDRESS, &write.reg, sizeof(write.reg), DELAY_ZIRO);
        I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(write.data), write.size, DELAY_SRAM);
        
    }
}

void Sunlight_CO₂::clear_error_status()
{
    uint8_t buff[2] = {static_cast<uint8_t>(Registers::ClearErrorStatus), 0x01};

    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_SRAM);
}

void Sunlight_CO₂::read_error_status()
{
    uint8_t errorStatusReg = static_cast<uint8_t>(Registers::ErrorStatus);

    I2CWrite(SENSOR_ADDRESS, &errorStatusReg, sizeof(errorStatusReg), DELAY_ZIRO);
    I2CRead(SENSOR_ADDRESS, &meas_data.errorstatus, sizeof(meas_data.errorstatus));
}

void Sunlight_CO₂::start_mesure()
{
    uint8_t buff[2] = {static_cast<uint8_t>(Registers::StartMesurement), 0x01};
    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_SRAM);
}

void Sunlight_CO₂::reset_sensor()
{

    uint8_t buff[2] = {static_cast<uint8_t>(Registers::Src), 0xFF};
    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_WAKEUP);
}

void Sunlight_CO₂::CO2_measurement_get(uint16_t *pressure)
{
    struct RegisterRead
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterRead reads[] = {

        {static_cast<uint8_t>(Registers::MeasuredFilteredPc), &meas_data.co2, sizeof(meas_data.co2)},
        {static_cast<uint8_t>(Registers::Temperature), &meas_data.temperature, sizeof(meas_data.temperature)},
        {static_cast<uint8_t>(Registers::MeasurementCount), &meas_data.measurementCount, sizeof(meas_data.measurementCount)},
        {static_cast<uint8_t>(Registers::MeasurCycleTime), &meas_data.cycleTime, sizeof(meas_data.cycleTime)},
        {static_cast<uint8_t>(Registers::MeasUnfPressCompens), &meas_data.measUnfiCompens, sizeof(meas_data.measUnfiCompens)},
        {static_cast<uint8_t>(Registers::MeasFiltered), &meas_data.measuredFiltered, sizeof(meas_data.measuredFiltered)},
        {static_cast<uint8_t>(Registers::MeasuredUnfiltered), &meas_data.measuredUnfiltered, sizeof(meas_data.measuredUnfiltered)},
        {static_cast<uint8_t>(Registers::ErrorStatus), &meas_data.errorstatus, sizeof(meas_data.errorstatus)},

    };

    sleep();

    start_mesure();

    if (pressure != nullptr)
    {
        uint8_t pressureValueReg = static_cast<uint8_t>(Registers::PressureValue);

        I2CWrite(SENSOR_ADDRESS, &pressureValueReg, sizeof(pressureValueReg), DELAY_ZIRO);
        I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(pressure), 2, DELAY_SRAM);
    }

    sensor_state_data_set();

    // while (!gpio->read(nrdy_pin))
    // {
    //     delay->wait_ms(1);
    // }
    delay->wait_ms(2400);

    for (const auto read : reads)
    {
        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(read.data), read.size);
        swap_endianness(read.data, read.size);
    }

    // sensor_state_data_get();
    sleep();
}