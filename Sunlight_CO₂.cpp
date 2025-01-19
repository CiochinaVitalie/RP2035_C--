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
    int err = i2c->read(addr, reinterpret_cast<uint8_t *>(data), size);
    if (err < 0)
    {
        err = i2c->read(addr, reinterpret_cast<uint8_t *>(data), size);
    }

    return err;
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
        {static_cast<uint8_t>(Registers::FirmwareRev), &product.MainRevision, sizeof(product.MainRevision)},
        {static_cast<uint8_t>(Registers::SensorId), &product.SensorId, sizeof(product.SensorId)},
        {static_cast<uint8_t>(Registers::ProductCode), product.ProductCode.data(), product.ProductCode.size()},
    };

    for (const auto &read : reads)
    {

        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, read.data, read.size);
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

    for (const auto &read : reads)
    {
        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, read.data, read.size);
    }
}

template <typename T>
bool Sunlight_CO₂::is_valid_EE_register(Registers reg, T value)
{
    switch (reg) {
    case Registers::MeasurementMode_EE:
        return std::is_same_v<T, uint8_t>;
    case Registers::I2C_Address_EE:
        return std::is_same_v<T, uint8_t>;
    case Registers::Nominator_EE:
        return std::is_same_v<T, uint16_t>;
    case Registers::Denominator_EE:
        return std::is_same_v<T, uint16_t>;
    case Registers::ABC_Period_EE:
        return std::is_same_v<T, uint16_t>;
    case Registers::ABC_Target_EE:
        return std::is_same_v<T, uint16_t>;
    case Registers::StaticIIRFilter_EE:
        return std::is_same_v<T, uint8_t>;
    case Registers::MeterControl_EE:
        return std::is_same_v<T, uint8_t>;
    case Registers::MeasurementPeriod_EE:
        return std::is_same_v<T, uint16_t>;
    case Registers::NumberOfSamples_EE:
        return std::is_same_v<T, uint16_t>;

    default:
        return false;
    }
}

template <typename T>
bool Sunlight_CO₂::set_config(Registers reg, T value)
{
    if(!is_valid_EE_register(reg, value))
    {
        return false;
    }
    T current_value;

    I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(&reg), 1, DELAY_ZIRO);
    I2CRead(SENSOR_ADDRESS, &current_value, sizeof(current_value));

    if (current_value != value)
    {
        I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(&reg), 1, DELAY_ZIRO);
        I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(&value), sizeof(value), DELAY_SRAM);
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
        {static_cast<uint8_t>(Registers::FiltPar0), &state_data.filt_par0, sizeof(state_data.filt_par0)},
        {static_cast<uint8_t>(Registers::FiltPar1), &state_data.filt_par1, sizeof(state_data.filt_par1)},
        {static_cast<uint8_t>(Registers::FiltPar2), &state_data.filt_par2, sizeof(state_data.filt_par2)},
        {static_cast<uint8_t>(Registers::FiltPar3), &state_data.filt_par3, sizeof(state_data.filt_par3)},
        {static_cast<uint8_t>(Registers::FiltPar4), &state_data.filt_par4, sizeof(state_data.filt_par4)},
        {static_cast<uint8_t>(Registers::FiltPar5), &state_data.filt_par5, sizeof(state_data.filt_par5)},
        {static_cast<uint8_t>(Registers::FiltPar6), &state_data.filt_par6, sizeof(state_data.filt_par6)}

    };
    for (const auto &read : reads)
    {
        I2CWrite(SENSOR_ADDRESS, &read.reg, sizeof(read.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, read.data, read.size);
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
        {static_cast<uint8_t>(Registers::FiltPar0), &state_data.filt_par0, sizeof(state_data.filt_par0)},
        {static_cast<uint8_t>(Registers::FiltPar1), &state_data.filt_par1, sizeof(state_data.filt_par1)},
        {static_cast<uint8_t>(Registers::FiltPar2), &state_data.filt_par2, sizeof(state_data.filt_par2)},
        {static_cast<uint8_t>(Registers::FiltPar3), &state_data.filt_par3, sizeof(state_data.filt_par3)},
        {static_cast<uint8_t>(Registers::FiltPar4), &state_data.filt_par4, sizeof(state_data.filt_par4)},
        {static_cast<uint8_t>(Registers::FiltPar5), &state_data.filt_par5, sizeof(state_data.filt_par5)},
        {static_cast<uint8_t>(Registers::FiltPar6), &state_data.filt_par6, sizeof(state_data.filt_par6)}

    };
    for (const auto &write : writes)
    {
        I2CWrite(SENSOR_ADDRESS, &write.reg, sizeof(write.reg), DELAY_ZIRO);
        I2CWrite(SENSOR_ADDRESS, write.data, write.size,DELAY_SRAM);
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
    I2CRead(SENSOR_ADDRESS, &error, sizeof(error));
}

void Sunlight_CO₂::sensor_get_data(uint16_t *measData)
{

    uint8_t meas_reg = static_cast<uint8_t>(Registers::MeasuredFilteredPc);

    I2CWrite(SENSOR_ADDRESS, &meas_reg, sizeof(meas_reg), DELAY_ZIRO);
    I2CRead(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(measData), 2);
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

uint16_t Sunlight_CO₂::CO2_measurement_get(uint16_t *pressure)
{
    uint16_t CO2_Data;

    gpio->set_high(en_pin);
    delay->wait_ms(35);

    start_mesure();

    if (pressure != nullptr)
    {
        uint8_t pressureValueReg = static_cast<uint8_t>(Registers::PressureValue);
  
        I2CWrite(SENSOR_ADDRESS, &pressureValueReg, sizeof(pressureValueReg), DELAY_ZIRO);
        I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(pressure), 2);
    }

    sensor_state_data_set();

    while (!gpio->read(nrdy_pin))
    {
        delay->wait_ms(1);
    }

    read_error_status();
    sensor_get_data(&CO2_Data);
    sensor_state_data_get();
    gpio->set_low(en_pin);

    return CO2_Data;
}