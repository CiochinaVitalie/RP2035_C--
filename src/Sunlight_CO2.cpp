#include "Sunlight_CO2.hpp"

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

/**
 * @brief Checks if the system is little-endian.
 *
 * This function determines the endianness of the system by creating a
 * 32-bit integer with a known byte pattern (0x01020304) and then
 * examining the first byte. If the first byte is 0x04, the system is
 * little-endian. Otherwise, it is big-endian.
 *
 * @return true if the system is little-endian, false otherwise.
 */
bool Sunlight_CO₂::is_little_endian()
{
    uint32_t test = 0x01020304;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&test);

    return *ptr == 0x04;
}

/**
 * @brief Swaps the endianness of the given data if the system is little-endian.
 *
 * This function reverses the byte order of the data pointed to by `data` if the
 * size of the data is greater than 1 and the system is little-endian.
 *
 * @param data Pointer to the data whose endianness is to be swapped.
 * @param size Size of the data in bytes.
 */
void Sunlight_CO₂::swap_endianness(void *data, size_t size)
{
    if (size > 1 && little_endian)
    {
        auto *data_ptr = static_cast<uint8_t *>(data);
        std::reverse(data_ptr, data_ptr + size);
    }
}

/**
 * @brief Converts an Error enum value to its corresponding string representation.
 *
 * This function takes an Error enum value and returns a string that describes
 * the error. The returned string includes a description of the error followed
 * by a carriage return and newline characters.
 *
 * @param error The Error enum value to be converted to a string.
 * @return A string representation of the provided Error enum value.
 *         Possible return values are:
 *         - "No error \r\n"
 *         - "Low internal error \r\n"
 *         - "Measurement timeout error \r\n"
 *         - "Abnormal signal error \r\n"
 *         - "Scale factor error \r\n"
 *         - "Fatal error \r\n"
 *         - "I2C communication error \r\n"
 *         - "Algorithm error \r\n"
 *         - "Calibration error \r\n"
 *         - "Self-diagnostic error \r\n"
 *         - "Out of range error \r\n"
 *         - "Memory error \r\n"
 *         - "No measurement complete \r\n"
 *         - "Unknown error \r\n"
 */
const char* Sunlight_CO₂::error_to_string(Error error)
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

void Sunlight_CO₂::print_errors()
{

    for (uint16_t bit = 0; bit < 16; ++bit)
    {
        uint16_t error_flag = (1 << bit);
        if (meas_data.errorstatus & error_flag)
        { 
            printf("- %s\n", error_to_string(static_cast<Error>(error_flag)));
        }
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

/**
 * @brief Sets the configuration for the Sunlight_CO₂ sensor.
 *
 * This function writes the provided configuration settings to the sensor's registers.
 * If any of the critical configuration values differ from the current values in the sensor,
 * the sensor will be reset to apply the changes.
 *
 * @param set_config The configuration settings to be applied to the sensor.
 * @return true if the configuration was successfully applied.
 */
bool Sunlight_CO₂::set_config(Config set_config)
{
    bool reset_required = false;

    if(set_config.i2c_address < 0x08 || set_config.measurement_mode > 0x78)
    {
        return false;
    }

    struct RegisterWrite
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    RegisterWrite writes[] = {
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
        {static_cast<uint8_t>(Registers::Scale_ABC_Target), &set_config.scaled_abc_target, sizeof(set_config.scaled_abc_target)}};

    for (const auto write : writes)
    {
        uint8_t current_value[write.size];
        I2CWrite(SENSOR_ADDRESS, &write.reg, sizeof(write.reg), DELAY_ZIRO);
        I2CRead(SENSOR_ADDRESS, current_value, write.size);

        // swap_endianness(current_value, read.size);
        // swap_endianness(read.data, read.size);

        if (memcmp(current_value, write.data, write.size) != 0)
        {            
            I2CWrite(SENSOR_ADDRESS, &write.reg, sizeof(write.reg), DELAY_ZIRO);
            I2CWrite(SENSOR_ADDRESS, reinterpret_cast<const uint8_t *>(write.data), write.size, DELAY_EEPROM);

            switch (write.reg)
            {
            case static_cast<uint8_t>(Registers::MeasurementMode_EE):
            case static_cast<uint8_t>(Registers::MeasurementPeriod_EE):
            case static_cast<uint8_t>(Registers::NumberOfSamples_EE):
            case static_cast<uint8_t>(Registers::I2C_Address_EE):
            case static_cast<uint8_t>(Registers::Nominator_EE):
            case static_cast<uint8_t>(Registers::Denominator_EE):
                reset_required = true;
                break;

            default:
                break;
            }
        }
    }
    if (reset_required)
    {
        reset_sensor();
        get_config();
    }

    return true;
}

/**
 * @brief Retrieves sensor state data from the Sunlight CO₂ sensor.
 *
 * This function reads multiple registers from the Sunlight CO₂ sensor and stores the data
 * in the corresponding fields of the state_data structure. The data is read via I2C communication,
 * and the endianness of the data is swapped after reading.
 *
 * The function performs the following steps for each register:
 * 1. Writes the register address to the sensor.
 * 2. Reads the data from the sensor into the corresponding field in the state_data structure.
 * 3. Swaps the endianness of the read data.
 *
 * The registers read are:
 * - AbcTime
 * - AbcPar0
 * - AbcPar1
 * - AbcPar2
 * - AbcPar3
 * - FiltPar0
 * - FiltPar1
 * - FiltPar2
 * - FiltPar3
 * - FiltPar4
 * - FiltPar5
 * - FiltPar6
 *
 * @note Ensure that the I2C communication functions (I2CWrite and I2CRead) and the
 * swap_endianness function are properly implemented and available.
 */
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
    }
}

/**
 * @brief Sets the sensor state data by writing to specific registers.
 *
 * This function initializes an array of RegisterRead structures, each containing
 * a register address, a pointer to the data to be written, and the size of the data.
 * It then iterates over the array, swapping the endianness of the data, and writes
 * the register address and data to the sensor using I2C communication.
 *
 * Registers involved:
 * - AbcTime
 * - AbcPar0, AbcPar1, AbcPar2, AbcPar3
 * - FiltPar0, FiltPar1, FiltPar2, FiltPar3, FiltPar4, FiltPar5, FiltPar6
 * @note The function assumes that the swap_endianness and I2CWrite functions are defined elsewhere.
 */
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
        I2CWrite(SENSOR_ADDRESS, &write.reg, sizeof(write.reg), DELAY_ZIRO);
        I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(write.data), write.size, DELAY_SRAM);
    }
}

/**
 * @brief Clears the error status of the Sunlight CO₂ sensor.
 *
 * This function sends a command to the sensor to clear any existing error status.
 * It uses the I2C protocol to communicate with the sensor.
 *
 * @note The function sends a specific command to the sensor's register to clear the error status.
 */
void Sunlight_CO₂::clear_error_status()
{
    uint8_t buff[2] = {static_cast<uint8_t>(Registers::ClearErrorStatus), 0x01};

    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_SRAM);
}

/**
 * @brief Reads the error status from the sensor.
 *
 * This function writes the error status register address to the sensor and then reads the error status
 * into the `meas_data.errorstatus` member variable.
 *
 * @note This function uses I2C communication to interact with the sensor.
 */
void Sunlight_CO₂::read_error_status()
{
    uint8_t errorStatusReg = static_cast<uint8_t>(Registers::ErrorStatus);
    uint16_t raw_errorstatus;

    I2CWrite(SENSOR_ADDRESS, &errorStatusReg, sizeof(errorStatusReg), DELAY_ZIRO);
    I2CRead(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(raw_errorstatus), sizeof(raw_errorstatus));
    meas_data.errorstatus = raw_errorstatus;
}
/**
 * @brief Retrieves the calibration target value from the Sunlight CO₂ sensor.
 *
 * This function communicates with the Sunlight CO₂ sensor over I2C to read the
 * calibration target value. It first writes the CalibrationTarget register address
 * to the sensor, then reads the 2-byte target value from the sensor. The endianness
 * of the target value is swapped before returning it.
 *
 * @return uint16_t The calibration target value.
 */
uint16_t Sunlight_CO₂::GetCalibrationTarget()
{
    uint16_t target_value;
    uint8_t CalibrationTarget = static_cast<uint8_t>(Registers::CalibrationTarget);
    I2CWrite(SENSOR_ADDRESS, &CalibrationTarget, sizeof(CalibrationTarget), DELAY_ZIRO);
    I2CRead(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(&target_value), 2);
    swap_endianness(&target_value, sizeof(target_value));
    return target_value;
}
/**
 * @brief Sets the calibration target for the Sunlight CO₂ sensor.
 *
 * This function writes the calibration target value to the sensor's registers
 * using I2C communication. It first writes the calibration status register,
 * then the calibration target register, and finally the target value itself.
 *
 * @param val The calibration target value to be set.
 */
void Sunlight_CO₂::SetCalibrationTarget(uint16_t val)
{

    uint8_t buff[2] = {static_cast<uint8_t>(Registers::CalibrationStatus), 0x00};
    uint8_t CalibrationTarget = static_cast<uint8_t>(Registers::CalibrationTarget);
    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_SRAM);

    I2CWrite(SENSOR_ADDRESS, &CalibrationTarget, sizeof(CalibrationTarget), DELAY_ZIRO);
    I2CWrite(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(&val), 2, DELAY_SRAM);
}
/**
 * @brief Performs a background calibration for the Sunlight CO₂ sensor.
 *
 * This function initiates a background calibration process for the Sunlight CO₂ sensor.
 * It writes specific values to the sensor's registers to trigger the calibration process.
 *
 * @note The function uses the I2CWrite function to communicate with the sensor.
 */
bool Sunlight_CO₂::background_calibration()
{

    uint8_t cal_status_val = 0x00;
    uint16_t cal_cmd_val = 0x7C06;
    struct Register
    {
        uint8_t reg;
        void *data;
        size_t size;
    };

    Register writes[] = {
        {static_cast<uint8_t>(Registers::CalibrationStatus), &cal_status_val, sizeof(cal_status_val)},
        {static_cast<uint8_t>(Registers::CalibrationCommand), &cal_cmd_val, sizeof(cal_cmd_val)},
    };

    for (const auto write : writes)
    {
        I2CWrite(SENSOR_ADDRESS, &write.reg, sizeof(write.reg), DELAY_SRAM);
    }

    if (config.measurement_mode == 0x01)
    {
        sensor_state_data_set();
        // while (!gpio->read(nrdy_pin))
        // {
        //     delay->wait_ms(1);
        // }
        sensor_state_data_get();
    }

    I2CWrite(SENSOR_ADDRESS, &writes[0].reg, sizeof(writes[0].reg), DELAY_ZIRO);
    I2CRead(SENSOR_ADDRESS, reinterpret_cast<uint8_t *>(writes[0].data), writes[0].size);

    uint8_t status = *reinterpret_cast<uint8_t *>(writes[0].data);
    return (status == 0x20);
}
/**
 * @brief Initiates the measurement process for the Sunlight CO₂ sensor.
 *
 * This function sends a command to the sensor to start the measurement process.
 * It writes a specific command to the sensor's I2C address to trigger the measurement.
 *
 * @note The function uses the I2CWrite function to communicate with the sensor.
 *
 * @param None
 * @return void
 */
void Sunlight_CO₂::start_mesure()
{
    uint8_t buff[2] = {static_cast<uint8_t>(Registers::StartMesurement), 0x01};
    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_SRAM);
}

/**
 * @brief Resets the CO₂ sensor by writing a reset command to the sensor's register.
 *
 * This function sends a reset command to the CO₂ sensor using the I2C protocol.
 * It writes a specific value to the sensor's register to initiate the reset process.
 *
 * @note The function uses a predefined sensor address and a delay for the wakeup process.
 */
void Sunlight_CO₂::reset_sensor()
{

    uint8_t buff[2] = {static_cast<uint8_t>(Registers::Src), 0xFF};
    I2CWrite(SENSOR_ADDRESS, buff, sizeof(buff), DELAY_WAKEUP);
}

/**
 * @brief Measures and retrieves CO2 and other related sensor data.
 *
 * This function performs the following steps:
 * 1. Initializes a list of registers to read various sensor data.
 * 2. Puts the sensor to sleep.
 * 3. Starts the measurement process.
 * 4. If a pressure pointer is provided, writes the pressure value to the sensor.
 * 5. Sets the sensor state data.
 * 6. Waits for the sensor to be ready (commented out) or waits for a fixed delay.
 * 7. Reads data from the sensor registers and swaps the endianness of the data.
 * 8. Puts the sensor back to sleep.
 *
 * @param pressure Pointer to a uint16_t variable where the pressure value will be stored.
 *                 If nullptr, pressure measurement is skipped.
 */
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