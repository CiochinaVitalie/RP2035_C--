#include "sx127x.hpp"
#include "sx127x_registers.hpp"

#define SX127x_VERSION 0x12

#define SX127x_OSCILLATOR_FREQUENCY 32000000.0f
#define SX127x_FREQ_ERROR_FACTOR ((1 << 24) / SX127x_OSCILLATOR_FREQUENCY)
#define SX127x_FSTEP (SX127x_OSCILLATOR_FREQUENCY / (1 << 19))
#define SX127x_REG_MODEM_CONFIG_3_AGC_ON 0b00000100
#define SX127x_REG_MODEM_CONFIG_3_AGC_OFF 0b00000000

#define SX127x_IRQ_FLAG_RXTIMEOUT 0b10000000
#define SX127x_IRQ_FLAG_RXDONE 0b01000000
#define SX127x_IRQ_FLAG_PAYLOAD_CRC_ERROR 0b00100000
#define SX127x_IRQ_FLAG_VALID_HEADER 0b00010000
#define SX127x_IRQ_FLAG_TXDONE 0b00001000
#define SX127x_IRQ_FLAG_CADDONE 0b00000100
#define SX127x_IRQ_FLAG_FHSSCHANGECHANNEL 0b00000010
#define SX127x_IRQ_FLAG_CAD_DETECTED 0b00000001

#define SX127X_FSK_IRQ_FIFO_FULL 0b10000000
#define SX127X_FSK_IRQ_FIFO_EMPTY 0b01000000
#define SX127X_FSK_IRQ_FIFO_LEVEL 0b00100000
#define SX127X_FSK_IRQ_FIFO_OVERRUN 0b00010000
#define SX127X_FSK_IRQ_PACKET_SENT 0b00001000
#define SX127X_FSK_IRQ_PAYLOAD_READY 0b00000100
#define SX127X_FSK_IRQ_CRC_OK 0b00000010
#define SX127X_FSK_IRQ_LOW_BATTERY 0b00000001
#define SX127X_FSK_IRQ_PREAMBLE_DETECT 0b00000010
#define SX127X_FSK_IRQ_SYNC_ADDRESS_MATCH 0b00000001

#define RF_MID_BAND_THRESHOLD 525000000
#define RSSI_OFFSET_HF_PORT 157
#define RSSI_OFFSET_LF_PORT 164

#define SX127x_MAX_POWER 0b01110000
#define SX127x_LOW_POWER 0b00000000

#define SX127x_HIGH_POWER_ON 0b10000111
#define SX127x_HIGH_POWER_OFF 0b10000100

#define FIFO_TX_BASE_ADDR 0b00000000
#define FIFO_RX_BASE_ADDR 0b00000000

#define FIFO_SIZE_FSK 64
#define MAX_FIFO_THRESHOLD 0b00111111
#define HALF_MAX_FIFO_THRESHOLD (MAX_FIFO_THRESHOLD >> 1)
#define TX_START_CONDITION_FIFO_LEVEL 0b00000000
#define TX_START_CONDITION_FIFO_EMPTY 0b10000000

#define SHADOW_NOT_CACHED 0
#define SHADOW_CACHED 1
#define SHADOW_IGNORE 2

#define ERROR_CHECK(x)             \
    do                             \
    {                              \
        int __err_rc = (x);        \
        if (__err_rc != SX127X_OK) \
        {                          \
            return __err_rc;       \
        }                          \
    } while (0)

#define ERROR_CHECK_NOCODE(x)      \
    do                             \
    {                              \
        int __err_rc = (x);        \
        if (__err_rc != SX127X_OK) \
        {                          \
            return;                \
        }                          \
    } while (0)

#define CHECK_MODULATION(x, y)               \
    do                                       \
    {                                        \
        if (x != y)                          \
        {                                    \
            return SX127X_ERR_INVALID_STATE; \
        }                                    \
    } while (0)

#define CHECK_FSK_OOK_MODULATION(x)                                                               \
    do                                                                                            \
    {                                                                                             \
        if (x->active_modem != SX127x_MODULATION_FSK && x->active_modem != SX127x_MODULATION_OOK) \
        {                                                                                         \
            return SX127X_ERR_INVALID_STATE;                                                      \
        }                                                                                         \
    } while (0)

typedef enum
{
    SX127x_HEADER_MODE_EXPLICIT = 0b00000000,
    SX127x_HEADER_MODE_IMPLICIT = 0b00000001
} sx127x_header_mode_t;

void SX127x::reset()
{
    gpio->set_low(rst);
    delay->wait_ms(1);
    gpio->set_high(rst);
    delay->wait_ms(10);
}
uint8_t SX127x::read_buffer(uint8_t reg, uint8_t *buffer, size_t buffer_length)
{
    uint8_t addr = reg & 0x7F;
    gpio->set_low(nss);
    spi->write(&addr, 1);
    uint8_t ret = spi->read(buffer, buffer_length);
    gpio->set_high(nss);
    return ret;
}
uint8_t SX127x::write_buffer(uint8_t reg, uint8_t *buffer, size_t buffer_length)
{
    uint8_t addr = reg | 0x80;
    gpio->set_low(nss);
    spi->write(&addr, 1);
    uint8_t ret = spi->write(buffer, buffer_length);
    gpio->set_high(nss);
    return ret;
}
uint8_t SX127x::read_reg(uint8_t addr, uint8_t *value)
{
    uint8_t tmp = addr & 0x7F;
    gpio->set_low(nss);
    uint8_t ret = spi->write_read(&tmp, value, 1);
    gpio->set_high(nss);
    return ret;
}

uint8_t SX127x::write_reg(uint8_t addr, uint8_t cmd)
{
    gpio->set_low(nss);
    uint8_t addr_with_cmd = addr | 0x80;
    spi->write(&addr_with_cmd, 1);
    uint8_t ret = spi->write(&cmd, 1);
    gpio->set_high(nss);
    return ret;
}
int SX127x::append_register(int reg, uint8_t value, uint8_t mask)
{
    uint8_t previous = 0;
    ERROR_CHECK(read_reg(reg, &previous));
    uint8_t data = {(previous & mask) | value};
    return write_reg(reg, data);
}

int SX127x::lora_set_low_datarate_optimization(bool enable)
{
    CHECK_MODULATION(active_modem, SX127x_MODULATION_LORA);
    uint8_t value = (enable ? 0b00001000 : 0b00000000);
    return append_register(REGMODEMCONFIG3, value, 0b11110111);
}

int SX127x::lora_get_bandwidth(uint32_t *bandwidth)
{
    CHECK_MODULATION(active_modem, SX127x_MODULATION_LORA);
    uint8_t config = 0;
    ERROR_CHECK(read_reg(REGMODEMCONFIG1, &config));
    config = (config >> 4) & 0x0F;
    static const uint32_t bandwidths[] = {
        7800, 10400, 15600, 20800, 31250, 41700, 62500, 125000, 250000, 500000};
    if (config < sizeof(bandwidths) / sizeof(bandwidths[0]))
    {
        *bandwidth = bandwidths[config];
        return SX127X_OK;
    }
    return SX127X_ERR_INVALID_ARG;
}

int SX127x::reload_low_datarate_optimization()
{
    uint32_t bandwidth;
    ERROR_CHECK(lora_get_bandwidth(&bandwidth));
    uint8_t spreading_factor = 0;
    ERROR_CHECK(read_reg(REGMODEMCONFIG2, &spreading_factor));
    spreading_factor = (spreading_factor >> 4);

    // Section 4.1.1.5
    uint32_t symbol_duration = 1000 / (bandwidth / (1L << spreading_factor));
    if (symbol_duration > 16)
    {
        // force low data rate optimization
        return lora_set_low_datarate_optimization(true);
    }
    else
    {
        return lora_set_low_datarate_optimization(false);
    }
}
int SX127x::lora_rx_read_payload()
{
    CHECK_MODULATION(active_modem, SX127x_MODULATION_LORA);
    uint8_t length;
    if (expected_packet_length == 0)
    {
        ERROR_CHECK(read_reg(REGRXNBBYTES, &length));
    }
    else
    {
        length = (uint8_t)expected_packet_length;
    }
    expected_packet_length = length;

    uint8_t current;
    ERROR_CHECK(read_reg(REGFIFORXCURRENTADDR, &current));
    ERROR_CHECK(write_reg(REGFIFOADDRPTR, current));
    return read_buffer(REGFIFO, packet, expected_packet_length);
}
void SX127x::lora_cad_set_callback(void (*cad_callback)(int))
{
    this->cad_callback = cad_callback;
}
void SX127x::tx_set_callback(void (*tx_callback)())
{
    this->tx_callback = tx_callback;
}
void SX127x::rx_set_callback(void (*rx_callback)(uint8_t *, uint16_t))
{
    this->rx_callback = rx_callback;
}
int SX127x::set_frequency(uint64_t frequency)
{
    uint64_t adjusted = (frequency << 19) / SX127x_OSCILLATOR_FREQUENCY;
    uint8_t data[] = {(uint8_t)(adjusted >> 16), (uint8_t)(adjusted >> 8), (uint8_t)(adjusted >> 0)};
    ERROR_CHECK(write_buffer(REGFRFMSB, data, 3));
    return SX127X_OK;
}

void SX127x::lora_handle_interrupt()
{
    uint8_t value;
    ERROR_CHECK_NOCODE(read_reg(REGIRQFLAGS, &value));
    ERROR_CHECK_NOCODE(write_reg(REGIRQFLAGS, value));
    if ((value & SX127x_IRQ_FLAG_CADDONE) != 0)
    {
        if (cad_callback != NULL)
        {
            cad_callback(value & SX127x_IRQ_FLAG_CAD_DETECTED);
        }
        return;
    }
    if ((value & SX127x_IRQ_FLAG_PAYLOAD_CRC_ERROR) != 0)
    {
        current_frequency = 0;
        return;
    }
    if ((value & SX127x_IRQ_FLAG_RXDONE) != 0)
    {
        ERROR_CHECK_NOCODE(lora_rx_read_payload());
        if (rx_callback != NULL)
        {
            rx_callback(packet, expected_packet_length);
        }
        expected_packet_length = 0;
        current_frequency = 0;
        return;
    }
    if ((value & SX127x_IRQ_FLAG_TXDONE) != 0)
    {
        current_frequency = 0;
        if (tx_callback != NULL)
        {
            tx_callback();
        }
        return;
    }
    // always last because if message was sent or received,
    // then no need to change freq
    if ((value & SX127x_IRQ_FLAG_FHSSCHANGECHANNEL) != 0)
    {
        if (current_frequency >= frequencies_length)
        {
            current_frequency = 0;
        }
        ERROR_CHECK_NOCODE(set_frequency(frequencies[current_frequency]));
        current_frequency++;
        return;
    }
}
int SX127x::create()
{
#ifndef CONFIG_SX127X_DISABLE_SPI_CACHE
    this->shadow_registers_sync[REGFIFO] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGFIFORXCURRENTADDR] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGRSSIVALUE_FSK] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGIRQFLAGS] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGRXNBBYTES] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGPKTSNRVALUE] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGPKTRSSIVALUE] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGFEIMSB] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGAFCMSB] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGSEQCONFIG1] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGIMAGECAL] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGTEMP] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGIRQFLAGS1] = SHADOW_IGNORE;
    this->shadow_registers_sync[REGIRQFLAGS2] = SHADOW_IGNORE;
#endif

    uint8_t version;
    int code = read_reg(REGVERSION, &version);
    if (code != SX127X_OK)
    {
        return code;
    }
    if (version != SX127x_VERSION)
    {
        return SX127X_ERR_INVALID_VERSION;
    }
    this->active_modem = SX127x_MODULATION_LORA;
    this->fsk_ook_format = SX127X_VARIABLE;
    this->fsk_rssi_available = false;
    this->opmod = SX127x_MODE_STANDBY;
    this->fsk_crc_type = SX127X_CRC_CCITT;
    this->use_implicit_header = false;
    this->expected_packet_length = 0;
    return SX127X_OK;
}
int SX127x::set_opmod(sx127x_mode_t opmod, sx127x_modulation_t modulation)
{
    // enforce DIO mappings for RX and TX
    if (modulation == SX127x_MODULATION_LORA)
    {
        if (opmod == SX127x_MODE_RX_CONT || opmod == SX127x_MODE_RX_SINGLE)
        {
            uint8_t data = (SX127x_DIO0_RX_DONE | SX127x_DIO1_RXTIMEOUT | SX127x_DIO2_FHSS_CHANGE_CHANNEL | SX127x_DIO3_CAD_DONE);
            ERROR_CHECK(write_reg(REGDIOMAPPING1, data));
        }
        else if (opmod == SX127x_MODE_TX)
        {
            uint8_t data = (SX127x_DIO0_TX_DONE | SX127x_DIO1_FHSS_CHANGE_CHANNEL | SX127x_DIO2_FHSS_CHANGE_CHANNEL | SX127x_DIO3_CAD_DONE);
            ERROR_CHECK(write_reg(REGDIOMAPPING1, data));
        }
        else if (opmod == SX127x_MODE_CAD)
        {
            ERROR_CHECK(append_register(REGDIOMAPPING1, SX127x_DIO0_CAD_DONE, 0b00111111));
        }
    }
    else if (modulation == SX127x_MODULATION_FSK || modulation == SX127x_MODULATION_OOK)
    {
        if (opmod == SX127x_MODE_RX_CONT || opmod == SX127x_MODE_RX_SINGLE)
        {
            ERROR_CHECK(append_register(REGDIOMAPPING1, SX127x_FSK_DIO0_PAYLOAD_READY | SX127x_FSK_DIO1_FIFO_LEVEL | SX127x_FSK_DIO2_SYNCADDRESS, 0b00000011));
            ERROR_CHECK(append_register(REGDIOMAPPING2, SX127x_FSK_DIO4_PREAMBLE_DETECT | 0b00000001, 0b00111110));
            // configure fifo level threshold for rx
            uint8_t data = HALF_MAX_FIFO_THRESHOLD;
            ERROR_CHECK(write_reg(REGFIFOTHRESH, data));
        }
        else if (opmod == SX127x_MODE_TX)
        {
            uint8_t data = (SX127x_FSK_DIO0_PACKET_SENT | SX127x_FSK_DIO1_FIFO_LEVEL | SX127x_FSK_DIO2_FIFO_FULL | SX127x_FSK_DIO3_FIFO_EMPTY);
            ERROR_CHECK(write_reg(REGDIOMAPPING1, data));
            // start tx as soon as first byte in FIFO available
            data = (TX_START_CONDITION_FIFO_EMPTY | HALF_MAX_FIFO_THRESHOLD);
            ERROR_CHECK(write_reg(REGFIFOTHRESH, data));
            // use sequencer to send single packet and stop carrier
            uint8_t value = 0b10010000;
            ERROR_CHECK(write_reg(REGSEQCONFIG1, value));
            this->active_modem = modulation;
            this->opmod = opmod;
            return SX127X_OK;
        }
    }
    else
    {
        return SX127X_ERR_INVALID_ARG;
    }
    uint8_t value = (opmod | modulation);
    int result = write_reg(REGOPMODE, value);
    if (result == SX127X_OK)
    {
        this->active_modem = modulation;
        this->opmod = opmod;
    }
    return result;
}

/// @brief /////////////////////////////////////////////////////////////////////////////
void SX127x::sleep()
{
    write_reg(LR_RegOpMode, 0x08);
    status = SLEEP;
}

int SX127x::LoRaEntryRx(uint8_t length, uint32_t timeout)
{
    uint8_t data;
    // spi->read(addr, &data, 1);
    return 0;
}
void SX127x::standby()
{
    write_reg(LR_RegOpMode, 0x09);
    status = STANDBY;
}
void SX127x::entryLoRa()
{
    write_reg(LR_RegOpMode, 0x88);
}
void SX127x::setFrequency(uint64_t frequency)
{
    uint64_t frf = (frequency << 19) / 32000000;
    write_reg(LR_RegFrMsb, (uint8_t)(frf >> 16));
    write_reg(LR_RegFrMid, (uint8_t)(frf >> 8));
    write_reg(LR_RegFrLsb, (uint8_t)(frf));
}
void SX127x::setPower(uint8_t power)
{
    if (power > 20)
    {
        power = 20;
    }
    else if (power < 5)
    {
        power = 5;
    }
    power = 0x87 + power;
    write_reg(LR_RegPaConfig, power);
}

void SX127x::setLoRaMode()
{
    write_reg(LR_RegOpMode, 0x88);
}
void SX127x::setLoRa()
{
    setLoRaMode();
    write_reg(LR_RegModemConfig1, 0x72);
    write_reg(LR_RegModemConfig2, 0x74);
    write_reg(LR_RegModemConfig3, 0x04);
    write_reg(LR_RegSymbTimeoutLsb, 0xFF);
    write_reg(LR_RegPreambleMsb, 0x00);
    write_reg(LR_RegPreambleLsb, 6);
    write_reg(LR_RegPayloadLength, 0xFF);
    write_reg(LR_RegFifoRxBaseAddr, 0);
    write_reg(LR_RegFifoTxBaseAddr, 0);
    write_reg(LR_RegOpMode, 0x88);
}
void SX127x::setLoRaTx()
{
    write_reg(LR_RegOpMode, 0x8B);
}
void SX127x::setLoRaRx()
{
    write_reg(LR_RegOpMode, 0x8D);
}
void SX127x::setLoRaTxPacket(uint8_t *buffer, uint8_t size)
{
    write_reg(LR_RegIrqFlags, 0xFF);
    write_reg(LR_RegPayloadLength, size);
    write_reg(LR_RegFifoAddrPtr, 0);
    for (int i = 0; i < size; i++)
    {
        write_reg(LR_RegFifo, buffer[i]);
    }
}
void SX127x::getLoRaRxPacket(uint8_t *buffer, uint8_t *size)
{
    uint8_t irqFlags = read_reg(LR_RegIrqFlags);
    write_reg(LR_RegIrqFlags, 0xFF);
    if ((irqFlags & 0x20) && (irqFlags & 0x40))
    {
        *size = 0;
    }
    else
    {
        uint8_t currentAddr = read_reg(LR_RegFifoRxCurrentaddr);
        uint8_t receivedCount = read_reg(LR_RegRxNbBytes);
        *size = receivedCount;
        write_reg(LR_RegFifoAddrPtr, currentAddr);
        for (int i = 0; i < receivedCount; i++)
        {
            buffer[i] = read_reg(LR_RegFifo);
        }
    }
}
void SX127x::clearLoRaIrq()
{
    write_reg(LR_RegIrqFlags, 0xFF);
}
void SX127x::setLoRaIrq()
{
    write_reg(REG_LR_DIOMAPPING1, 0x01);
    write_reg(REG_LR_DIOMAPPING2, 0x00);
}
void SX127x::setLoRaRxTimeout(uint32_t timeout)
{
    uint32_t symbolTimeout = timeout * 1000 / 31.25;
    write_reg(LR_RegSymbTimeoutLsb, symbolTimeout);
}
void SX127x::setLoRaRxSingle()
{
    write_reg(LR_RegOpMode, 0x8D);
}
void SX127x::setLoRaRxContinuous()
{
    write_reg(LR_RegOpMode, 0x8D);
}
void SX127x::setLoRaBW(uint8_t LoRa_BW)
{
    uint8_t tmp;
    tmp = read_reg(LR_RegModemConfig1);
    tmp &= 0x0F;
    LoRa_BW <<= 4;
    tmp |= LoRa_BW;
    write_reg(LR_RegModemConfig1, tmp);
}
void SX127x::setLoRaSF(uint8_t LoRa_SF)
{
    uint8_t tmp;
    switch (LoRa_SF)
    {
    case 6:
        tmp = 0x60;
        break;
    case 7:
        tmp = 0x70;
        break;
    case 8:
        tmp = 0x80;
        break;
    case 9:
        tmp = 0x90;
        break;
    case 10:
        tmp = 0xA0;
        break;
    case 11:
        tmp = 0xB0;
        break;
    case 12:
        tmp = 0xC0;
        break;
    default:
        tmp = 0x70;
        break;
    }
    write_reg(LR_RegModemConfig2, tmp);
}
void SX127x::setLoRaCR(uint8_t LoRa_CR)
{
    uint8_t tmp;
    tmp = read_reg(LR_RegModemConfig1);
    tmp &= 0xF1;
    LoRa_CR <<= 1;
    tmp |= LoRa_CR;
    write_reg(LR_RegModemConfig1, tmp);
}
void SX127x::setLoRaCRC(uint8_t LoRa_CRC)
{
    uint8_t tmp;
    tmp = read_reg(LR_RegModemConfig2);
    tmp &= 0xFB;
    LoRa_CRC <<= 2;
    tmp |= LoRa_CRC;
    write_reg(LR_RegModemConfig2, tmp);
}
void SX127x::setLoRaPreambleLength(uint16_t LoRa_PreambleLength)
{
    write_reg(LR_RegPreambleMsb, (uint8_t)(LoRa_PreambleLength >> 8));
    write_reg(LR_RegPreambleLsb, (uint8_t)(LoRa_PreambleLength));
}
void SX127x::setLoRaHeaderMode(uint8_t LoRa_HeaderMode)
{
    uint8_t tmp;
    tmp = read_reg(LR_RegModemConfig1);
    tmp &= 0xFE;
    LoRa_HeaderMode &= 0x01;
    tmp |= LoRa_HeaderMode;
    write_reg(LR_RegModemConfig1, tmp);
}
void SX127x::setLoRaIQInverted(uint8_t LoRa_IQInverted)
{
    uint8_t tmp;
    tmp = read_reg(LR_RegInvertIQ);
    tmp &= 0xBF;
    LoRa_IQInverted <<= 6;
    tmp |= LoRa_IQInverted;
    write_reg(LR_RegInvertIQ, tmp);
}
void SX127x::setLoRaSymbTimeout(uint16_t LoRa_SymbTimeout)
{
    write_reg(LR_RegSymbTimeoutLsb, (uint8_t)(LoRa_SymbTimeout));
}
void SX127x::setLoRaSyncWord(uint8_t sw)
{
    write_reg(LR_RegSyncWord, sw);
}
