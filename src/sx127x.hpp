#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include "ISPI.hpp"
#include "IGPIO.hpp"
#include "IDELAY.hpp"

#define SX1278_MAX_PACKET 256
#define SX1278_DEFAULT_TIMEOUT 3000

#define MAX_PACKET_SIZE 255
#define MAX_PACKET_SIZE_FSK_FIXED 2047
#define MAX_NUMBER_OF_REGISTERS 0x71

#define SX127X_OK 0                      /*!< esp_err_t value indicating success (no error) */
#define SX127X_ERR_INVALID_ARG 0x102     /*!< Invalid argument */
#define SX127X_ERR_INVALID_STATE 0x103   /*!< Invalid state. Most likely function is not applicable for the selected modem */
#define SX127X_ERR_NOT_FOUND 0x105       /*!< Requested resource not found */
#define SX127X_ERR_INVALID_VERSION 0x10A /*!< Version was invalid */

#ifndef CONFIG_SX127X_MAX_PACKET_SIZE
#define CONFIG_SX127X_MAX_PACKET_SIZE MAX_PACKET_SIZE_FSK_FIXED
#endif
/*
 * This structure used to change mode
 */
typedef enum
{
  SX127x_MODE_SLEEP = 0b00000000,     // SLEEP
  SX127x_MODE_STANDBY = 0b00000001,   // STDBY
  SX127x_MODE_FSTX = 0b00000010,      // Frequency synthesis TX
  SX127x_MODE_TX = 0b00000011,        // Transmit
  SX127x_MODE_FSRX = 0b00000100,      // Frequency synthesis RX
  SX127x_MODE_RX_CONT = 0b00000101,   // Receive continuous
  SX127x_MODE_RX_SINGLE = 0b00000110, // Receive single
  SX127x_MODE_CAD = 0b00000111        // Channel activity detection
} sx127x_mode_t;

typedef enum
{
  SX127x_MODULATION_LORA = 0b10000000,
  SX127x_MODULATION_FSK = 0b00000000, // default
  SX127x_MODULATION_OOK = 0b00100000
} sx127x_modulation_t;

/**
 * @brief Size of each decrement of the RSSI threshold in the OOK demodulator
 *
 */
typedef enum
{
  SX127X_0_5_DB = 0b00000000, // 0.5db (default)
  SX127X_1_0_DB = 0b00000001, // 1.0db
  SX127X_1_5_DB = 0b00000010, // 1.5db
  SX127X_2_0_DB = 0b00000011, // 2.0db
  SX127X_3_0_DB = 0b00000100, // 3.0db
  SX127X_4_0_DB = 0b00000101, // 4.0db
  SX127X_5_0_DB = 0b00000110, // 5.0db
  SX127X_6_0_DB = 0b00000111  // 6.0db
} sx127x_ook_peak_thresh_step_t;

/**
 * @brief Static offset added to the threshold in average mode in order to reduce glitching activity (OOK only)
 *
 */
typedef enum
{
  SX127X_0_DB = 0b00000000,
  SX127X_2_DB = 0b00000100,
  SX127X_4_DB = 0b00001000,
  SX127X_6_DB = 0b00001100
} sx127x_ook_avg_offset_t;

/**
 * @brief Filter coefficients in average mode of the OOK demodulator
 *
 */
typedef enum
{
  SX127X_32_PI = 0b00000000, // chip rate / 32.π
  SX127X_8_PI = 0b00000001,  // chip rate / 8.π
  SX127X_4_PI = 0b00000010,  // chip rate / 4.π (default)
  SX127X_2_PI = 0b00000011   // chip rate / 2.π
} sx127x_ook_avg_thresh_t;

/**
 * @brief Period of decrement of the RSSI threshold in the OOK demodulator
 *
 */
typedef enum
{
  SX127X_1_1_CHIP = 0b00000000, // once per chip (default)
  SX127X_1_2_CHIP = 0b00100000, // once every 2 chips
  SX127X_1_4_CHIP = 0b01000000, // once every 4 chips
  SX127X_1_8_CHIP = 0b01100000, // once every 8 chips
  SX127X_2_1_CHIP = 0b10000000, // twice in each chip
  SX127X_4_1_CHIP = 0b10100000, // 4 times in each chip
  SX127X_8_1_CHIP = 0b11000000, // 8 times in each chip
  SX127X_16_1_CHIP = 0b11100000 // 16 times in each chip
} sx127x_ook_peak_thresh_dec_t;

typedef enum
{
  SX127X_RX_TRIGGER_NONE = 0b00000000,
  SX127X_RX_TRIGGER_RSSI = 0b00000001,
  SX127X_RX_TRIGGER_PREAMBLE = 0b00000110, // default
  SX127X_RX_TRIGGER_RSSI_PREAMBLE = 0b00000111
} sx127x_rx_trigger_t;

typedef enum
{
  SX127X_PREAMBLE_55 = 0b00100000,
  SX127X_PREAMBLE_AA = 0b00000000
} sx127x_preamble_type_t;

typedef enum
{
  SX127X_2 = 0b00000000,
  SX127X_4 = 0b00000001,
  SX127X_8 = 0b00000010,
  SX127X_16 = 0b00000011,
  SX127X_32 = 0b00000100,
  SX127X_64 = 0b00000101,
  SX127X_128 = 0b00000110,
  SX127X_256 = 0b00000111
} sx127x_rssi_smoothing_t;

typedef enum
{
  SX127X_NRZ = 0b00000000,
  SX127X_MANCHESTER = 0b00100000,
  SX127X_SCRAMBLED = 0b01000000 // LFSR Polynomial =X9 + X5 + 1
} sx127x_packet_encoding_t;

typedef enum
{
  SX127X_CRC_NONE = 0b00001000,  // CrcOff + Do not clear FIFO
  SX127X_CRC_CCITT = 0b00011000, // CrcOn + CrcWhiteningType. Polynomial X16 + X12 + X5 + 1 Seed Value 0x1D0F
  SX127X_CRC_IBM = 0b00011001    // CrcOn + CrcWhiteningType. Polynomial X16 + X15 + X2 + 1 Seed Value 0xFFFF
} sx127x_crc_type_t;

typedef enum
{
  SX127X_FIXED = 0b00000000,
  SX127X_VARIABLE = 0b10000000
} sx127x_packet_format_t;

typedef enum
{
  SX127X_FILTER_NONE = 0b00000000,
  SX127X_FILTER_NODE_ADDRESS = 0b00000010,
  SX127X_FILTER_NODE_AND_BROADCAST = 0b00000100
} sx127x_address_filtering_t;

typedef enum
{
  SX127x_LNA_GAIN_G1 = 0b00100000, // Maximum gain
  SX127x_LNA_GAIN_G2 = 0b01000000,
  SX127x_LNA_GAIN_G3 = 0b01100000,
  SX127x_LNA_GAIN_G4 = 0b10000000,
  SX127x_LNA_GAIN_G5 = 0b10100000,
  SX127x_LNA_GAIN_G6 = 0b11000000,  // Minimum gain
  SX127x_LNA_GAIN_AUTO = 0b00000000 // Automatic. See 5.5.3. for details
} sx127x_gain_t;

typedef enum
{
  SX127X_FSK_SHAPING_NONE = 0b00000000, // no shaping
  SX127X_BT_1_0 = 0b00100000,           // Gaussian filter BT = 1.0
  SX127X_BT_0_5 = 0b01000000,           // Gaussian filter BT = 0.5
  SX127X_BT_0_3 = 0b01100000            // Gaussian filter BT = 0.3
} sx127x_fsk_data_shaping_t;

typedef enum
{
  SX127X_OOK_SHAPING_NONE = 0b00000000, // no shaping (default)
  SX127X_1_BIT_RATE = 0b00100000,       // filtering with fcutoff = bit_rate
  SX127X_2_BIT_RATE = 0b01000000        // filtering with fcutoff = 2*bit_rate (for bit_rate < 125 kb/s)
} sx127x_ook_data_shaping_t;

typedef enum
{
  SX127X_PA_RAMP_1 = 0b00000000,  // 3.4 ms
  SX127X_PA_RAMP_2 = 0b00000001,  // 2 ms
  SX127X_PA_RAMP_3 = 0b00000010,  // 1 ms
  SX127X_PA_RAMP_4 = 0b00000011,  // 500 us
  SX127X_PA_RAMP_5 = 0b00000100,  // 250 us
  SX127X_PA_RAMP_6 = 0b00000101,  // 125 us
  SX127X_PA_RAMP_7 = 0b00000110,  // 100 us
  SX127X_PA_RAMP_8 = 0b00000111,  // 62 us
  SX127X_PA_RAMP_9 = 0b00001000,  // 50 us
  SX127X_PA_RAMP_10 = 0b00001001, // Default. 40 us
  SX127X_PA_RAMP_11 = 0b00001010, // 31 us
  SX127X_PA_RAMP_12 = 0b00001011, // 25 us
  SX127X_PA_RAMP_13 = 0b00001100, // 20 us
  SX127X_PA_RAMP_14 = 0b00001101, // 15 us
  SX127X_PA_RAMP_15 = 0b00001110, // 12 us
  SX127X_PA_RAMP_16 = 0b00001111  // 10 us
} sx127x_pa_ramp_t;

/**
 * @brief Signal bandwidth.
 *
 * @note In the lower band (169MHz), signal bandwidths 8&9 (250k and 500k) are not supported
 *
 */
typedef enum
{
  SX127x_BW_7800 = 0b00000000,
  SX127x_BW_10400 = 0b00010000,
  SX127x_BW_15600 = 0b00100000,
  SX127x_BW_20800 = 0b00110000,
  SX127x_BW_31250 = 0b01000000,
  SX127x_BW_41700 = 0b01010000,
  SX127x_BW_62500 = 0b01100000,
  SX127x_BW_125000 = 0b01110000, // default
  SX127x_BW_250000 = 0b10000000,
  SX127x_BW_500000 = 0b10010000
} sx127x_bw_t;

typedef enum
{
  SX127x_CR_4_5 = 0b00000010, // default
  SX127x_CR_4_6 = 0b00000100,
  SX127x_CR_4_7 = 0b00000110,
  SX127x_CR_4_8 = 0b00001000
} sx127x_cr_t;

/**
 * @brief SF rate (expressed as a base-2 logarithm)
 *
 */
typedef enum
{
  SX127x_SF_6 = 0b01100000,  // 64 chips / symbol
  SX127x_SF_7 = 0b01110000,  // 128 chips / symbol
  SX127x_SF_8 = 0b10000000,  // 256 chips / symbol
  SX127x_SF_9 = 0b10010000,  // 512 chips / symbol
  SX127x_SF_10 = 0b10100000, // 1024 chips / symbol
  SX127x_SF_11 = 0b10110000, // 2048 chips / symbol
  SX127x_SF_12 = 0b11000000  // 4096 chips / symbol
} sx127x_sf_t;

/**
 * @brief Imlicit header for TX or RX.
 *
 */
typedef struct
{
  uint8_t length;          // payload length. Cannot be more than 256 bytes.
  bool enable_crc;         // Enable or disable CRC.
  sx127x_cr_t coding_rate; // Coding rate
} sx127x_implicit_header_t;

typedef struct
{
  bool enable_crc;
  sx127x_cr_t coding_rate;
} sx127x_tx_header_t;

/**
 * @brief Type of interrupt. Same interrupts can happen on different digital pins.
 *
 */
typedef enum
{
  SX127x_DIO0_RX_DONE = 0b00000000,             // Packet reception complete
  SX127x_DIO0_TX_DONE = 0b01000000,             // FIFO Payload transmission complete
  SX127x_DIO0_CAD_DONE = 0b10000000,            // CAD complete
  SX127x_DIO1_RXTIMEOUT = 0b00000000,           // RX timeout interrupt. Used in RX single mode
  SX127x_DIO1_FHSS_CHANGE_CHANNEL = 0b00010000, // FHSS change channel
  SX127x_DIO1_CAD_DETECTED = 0b00100000,        // Valid Lora signal detected during CAD operation
  SX127x_DIO2_FHSS_CHANGE_CHANNEL = 0b00000000, // FHSS change channel on digital pin 2
  SX127x_DIO3_CAD_DONE = 0b00000000,            // CAD complete on digital pin 3
  SX127x_DIO3_VALID_HEADER = 0b00000001,        // Valid header received in Rx
  SX127x_DIO3_PAYLOAD_CRC_ERROR = 0b00000010,   // Payload CRC error
} sx127x_dio_mapping1_t;

typedef enum
{
  SX127x_FSK_DIO0_PAYLOAD_READY = 0b00000000,
  SX127x_FSK_DIO0_PACKET_SENT = 0b00000000,
  SX127x_FSK_DIO0_CRC_OK = 0b01000000,
  SX127x_FSK_DIO1_FIFO_LEVEL = 0b00000000,
  SX127x_FSK_DIO1_FIFO_EMPTY = 0b00010000,
  SX127x_FSK_DIO1_FIFO_FULL = 0b00100000,
  SX127x_FSK_DIO2_FIFO_FULL = 0b00000000,
  SX127x_FSK_DIO2_SYNCADDRESS = 0b00001100,
  SX127x_FSK_DIO3_FIFO_EMPTY = 0b00000000
} sx127x_fsk_ook_dio_mapping1_t;

/**
 * @brief Type of interrupt. Same interrupts can happen on different digital pins.
 *
 */
typedef enum
{
  SX127x_DIO4_CAD_DETECTED = 0b00000000, // Valid Lora signal detected during CAD operation
  SX127x_DIO4_PLL_LOCK = 0b01000000,     // PLL lock
  SX127x_DIO5_MODE_READY = 0b00000000,   // Mode ready
  SX127x_DIO5_CLK_OUT = 0b01000000       // clock out
} sx127x_dio_mapping2_t;

typedef enum
{
  SX127x_FSK_DIO4_TEMP_CHANGE = 0b00000000,
  SX127x_FSK_DIO4_PLL_LOCK = 0b01000000,
  SX127x_FSK_DIO4_TIMEOUT = 0b10000000,
  SX127x_FSK_DIO4_PREAMBLE_DETECT = 0b11000000,
  SX127x_FSK_DIO5_CLK_OUT = 0b00000000,
  SX127x_FSK_DIO5_PLL_LOCK = 0b00010000,
  SX127x_FSK_DIO5_DATA = 0b00100000,
  SX127x_FSK_DIO5_MODE_READY = 0b00110000
} sx127x_fsk_ook_dio_mapping2_t;

// RFM98 Internal registers Address
/********************LoRa mode***************************/
#define LR_RegFifo 0x00
// Common settings
#define LR_RegOpMode 0x01
#define LR_RegFrMsb 0x06
#define LR_RegFrMid 0x07
#define LR_RegFrLsb 0x08
// Tx settings
#define LR_RegPaConfig 0x09
#define LR_RegPaRamp 0x0A
#define LR_RegOcp 0x0B
// Rx settings
#define LR_RegLna 0x0C
// LoRa registers
#define LR_RegFifoAddrPtr 0x0D
#define LR_RegFifoTxBaseAddr 0x0E
#define LR_RegFifoRxBaseAddr 0x0F
#define LR_RegFifoRxCurrentaddr 0x10
#define LR_RegIrqFlagsMask 0x11
#define LR_RegIrqFlags 0x12
#define LR_RegRxNbBytes 0x13
#define LR_RegRxHeaderCntValueMsb 0x14
#define LR_RegRxHeaderCntValueLsb 0x15
#define LR_RegRxPacketCntValueMsb 0x16
#define LR_RegRxPacketCntValueLsb 0x17
#define LR_RegModemStat 0x18
#define LR_RegPktSnrValue 0x19
#define LR_RegPktRssiValue 0x1A
#define LR_RegRssiValue 0x1B
#define LR_RegHopChannel 0x1C
#define LR_RegModemConfig1 0x1D
#define LR_RegModemConfig2 0x1E
#define LR_RegSymbTimeoutLsb 0x1F
#define LR_RegPreambleMsb 0x20
#define LR_RegPreambleLsb 0x21
#define LR_RegPayloadLength 0x22
#define LR_RegMaxPayloadLength 0x23
#define LR_RegHopPeriod 0x24
#define LR_RegFifoRxByteAddr 0x25
#define LR_RegModemConfig3 0x26
#define LR_RegInvertIQ 0x33
#define LR_RegSyncWord 0x39
#define REG_LR_DIOMAPPING1 0x40
#define REG_LR_DIOMAPPING2 0x41
// Version
#define REG_LR_VERSION 0x42
// Additional settings
#define REG_LR_PLLHOP 0x44
#define REG_LR_TCXO 0x4B
#define REG_LR_PADAC 0x4D
#define REG_LR_FORMERTEMP 0x5B
#define REG_LR_AGCREF 0x61
#define REG_LR_AGCTHRESH1 0x62
#define REG_LR_AGCTHRESH2 0x63
#define REG_LR_AGCTHRESH3 0x64

/********************FSK/ook mode***************************/
#define RegFIFO 0x00
#define RegOpMode 0x01
#define RegBitRateMsb 0x02
#define RegBitRateLsb 0x03
#define RegFdevMsb 0x04
#define RegFdevLsb 0x05
#define RegFreqMsb 0x06
#define RegFreqMid 0x07
#define RegFreqLsb 0x08
#define RegPaConfig 0x09
#define RegPaRamp 0x0a
#define RegOcp 0x0b
#define RegLna 0x0c
#define RegRxConfig 0x0d
#define RegRssiConfig 0x0e
#define RegRssiCollision 0x0f
#define RegRssiThresh 0x10
#define RegRssiValue 0x11
#define RegRxBw 0x12
#define RegAfcBw 0x13
#define RegOokPeak 0x14
#define RegOokFix 0x15
#define RegOokAvg 0x16
#define RegAfcFei 0x1a
#define RegAfcMsb 0x1b
#define RegAfcLsb 0x1c
#define RegFeiMsb 0x1d
#define RegFeiLsb 0x1e
#define RegPreambleDetect 0x1f
#define RegRxTimeout1 0x20
#define RegRxTimeout2 0x21
#define RegRxTimeout3 0x22
#define RegRxDelay 0x23
#define RegOsc 0x24
#define RegPreambleMsb 0x25
#define RegPreambleLsb 0x26
#define RegSyncConfig 0x27
#define RegSyncValue1 0x28
#define RegSyncValue2 0x29
#define RegSyncValue3 0x2a
#define RegSyncValue4 0x2b
#define RegSyncValue5 0x2c
#define RegSyncValue6 0x2d
#define RegSyncValue7 0x2e
#define RegSyncValue8 0x2f
#define RegPacketConfig1 0x30
#define RegPacketConfig2 0x31
#define RegPayloadLength 0x32
#define RegNodeAdrs 0x33
#define RegBroadcastAdrs 0x34
#define RegFifoThresh 0x35
#define RegSeqConfig1 0x36
#define RegSeqConfig2 0x37
#define RegTimerResol 0x38
#define RegTimer1Coef 0x39
#define RegSyncWord 0x39
#define RegTimer2Coef 0x3a
#define RegImageCal 0x3b
#define RegTemp 0x3c
#define RegLowBat 0x3d
#define RegIrqFlags1 0x3e
#define RegIrqFlags2 0x3f
#define RegDioMapping1 0x40
#define RegDioMapping2 0x41
#define RegVersion 0x42
#define RegPllHop 0x44
#define RegPaDac 0x4d
#define RegBitRateFrac 0x5d

/**********************************************************
 **Parameter table define
 **********************************************************/

#define SX1278_POWER_20DBM 0
#define SX1278_POWER_17DBM 1
#define SX1278_POWER_14DBM 2
#define SX1278_POWER_11DBM 3

static const uint8_t SX1278_Power[4] = {
    0xFF, // 20dbm
    0xFC, // 17dbm
    0xF9, // 14dbm
    0xF6, // 11dbm
};

#define SX1278_LORA_SF_6 0
#define SX1278_LORA_SF_7 1
#define SX1278_LORA_SF_8 2
#define SX1278_LORA_SF_9 3
#define SX1278_LORA_SF_10 4
#define SX1278_LORA_SF_11 5
#define SX1278_LORA_SF_12 6

static const uint8_t SX1278_SpreadFactor[7] = {6, 7, 8, 9, 10, 11, 12};

#define SX1278_LORA_BW_7_8KHZ 0
#define SX1278_LORA_BW_10_4KHZ 1
#define SX1278_LORA_BW_15_6KHZ 2
#define SX1278_LORA_BW_20_8KHZ 3
#define SX1278_LORA_BW_31_2KHZ 4
#define SX1278_LORA_BW_41_7KHZ 5
#define SX1278_LORA_BW_62_5KHZ 6
#define SX1278_LORA_BW_125KHZ 7
#define SX1278_LORA_BW_250KHZ 8
#define SX1278_LORA_BW_500KHZ 9

static const uint8_t SX1278_LoRaBandwidth[10] = {
    0, //   7.8KHz,
    1, //  10.4KHz,
    2, //  15.6KHz,
    3, //  20.8KHz,
    4, //  31.2KHz,
    5, //  41.7KHz,
    6, //  62.5KHz,
    7, // 125.0KHz,
    8, // 250.0KHz,
    9  // 500.0KHz
};

// Coding rate
#define SX1278_LORA_CR_4_5 0
#define SX1278_LORA_CR_4_6 1
#define SX1278_LORA_CR_4_7 2
#define SX1278_LORA_CR_4_8 3

static const uint8_t SX1278_CodingRate[4] = {0x01, 0x02, 0x03, 0x04};

// CRC Enable
#define SX1278_LORA_CRC_EN 0
#define SX1278_LORA_CRC_DIS 1

static const uint8_t SX1278_CRC_Sum[2] = {0x01, 0x00};

#define SPI_RX_PIN 4
#define SPI_TX_PIN 5
#define SPI_SCK_PIN 6
#define SPI_CSN_PIN 7
#define PIN_NSS 10
#define PIN_RST 11
#define PIN_DIO0 12

class SX127x
{
public:
  enum SX127x_STATUS
  {
    SLEEP,
    STANDBY,
    TX,
    RX
  };
  SX127x(ISPI *spi_context, IDelay *delay_context, IGPIO *gpio_context,
         void *rst, void *dio0, void *clk, void *miso, void *mosi, void *nss) : spi(spi_context), delay(delay_context), gpio(gpio_context),
                                                                                rst(rst), dio0(dio0), clk(clk), miso(miso), mosi(mosi), nss(nss)
  {
    spi->init(clk, mosi, miso);
    gpio->output_conf(nss);
    gpio->output_conf(rst);
    gpio->output_conf(dio0);
  }

  ~SX127x()
  {
  }
  int LoRaEntryRx(uint8_t length, uint32_t timeout);
  int append_register(int reg, uint8_t value, uint8_t mask);
  int lora_set_low_datarate_optimization(bool enable);
  int lora_get_bandwidth(uint32_t *bandwidth);
  int reload_low_datarate_optimization();
  int lora_rx_read_payload();
  void lora_cad_set_callback(void (*cad_callback)(int));
  void tx_set_callback(void (*tx_callback)());
  void rx_set_callback(void (*rx_callback)(uint8_t *, uint16_t));
  void lora_handle_interrupt();
  int create();
  int set_opmod(sx127x_mode_t opmod, sx127x_modulation_t modulation);

private:
  ISPI *spi;
  IDelay *delay;
  IGPIO *gpio;
  void *rst;
  void *dio0;
  void *clk;
  void *miso;
  void *mosi;
  void *nss;
  uint8_t read_reg(uint8_t addr, uint8_t *value);
  uint8_t write_reg(uint8_t addr, uint8_t cmd);
  uint8_t read_buffer(uint8_t reg, uint8_t *buffer, size_t buffer_length);
  uint8_t write_buffer(uint8_t reg, uint8_t *buffer, size_t buffer_length);
  uint64_t frequency;
  uint8_t power;
  uint8_t LoRa_SF;
  uint8_t LoRa_BW;
  uint8_t LoRa_CR;
  uint8_t LoRa_CRC_sum;
  uint8_t packetLength;

  SX127x_STATUS status;

  uint8_t rxBuffer[SX1278_MAX_PACKET];
  uint8_t readBytes;
  ////////////////////////////////////////////////////////
#ifndef CONFIG_SX127X_DISABLE_SPI_CACHE
  uint8_t shadow_registers[MAX_NUMBER_OF_REGISTERS];
  uint8_t shadow_registers_sync[MAX_NUMBER_OF_REGISTERS];
#endif
  uint8_t packet[CONFIG_SX127X_MAX_PACKET_SIZE];
  uint16_t expected_packet_length;
  uint16_t fsk_ook_packet_sent_received;
  bool fsk_rssi_available;
  bool use_implicit_header;
  int16_t fsk_rssi;

  sx127x_modulation_t active_modem;
  sx127x_mode_t opmod;
  sx127x_packet_format_t fsk_ook_format;
  sx127x_crc_type_t fsk_crc_type;

  uint64_t *frequencies;
  uint8_t frequencies_length;
  uint8_t current_frequency;

  void sleep();
  void standby();
  void entryLoRa();
  void clearLoRaIrq();

  void reset();
  void setFrequency(uint64_t frequency);
  void setPower(uint8_t power);
  void setLoRaMode();
  void setLoRa();
  void setLoRaBW(uint8_t LoRa_BW);
  void setLoRaSF(uint8_t LoRa_SF);
  void setLoRaCR(uint8_t LoRa_CR);
  void setLoRaCRC(uint8_t LoRa_CRC_sum);
  void setLoRaPreambleLength(uint16_t length);
  void setLoRaPacketLength(uint8_t length);
  void setLoRaIQ();
  void setLoRaSyncWord(uint8_t sw);
  void setLoRaRx();
  void setLoRaTx();
  void setLoRaIrq();
  void setLoRaRxTimeout(uint32_t timeout);
  void setLoRaRxSingle();
  void setLoRaRxContinuous();
  void setLoRaTxPacket(uint8_t *buffer, uint8_t size);
  void getLoRaRxPacket(uint8_t *buffer, uint8_t *size);
  void setLoRaHeaderMode(uint8_t LoRa_HeaderMode);
  void setLoRaIQInverted(uint8_t LoRa_IQInverted);
  void setLoRaSymbTimeout(uint16_t LoRa_SymbTimeout);

  void (*cad_callback)(int);
  void (*tx_callback)();
  void (*rx_callback)(uint8_t *, uint16_t);
  int set_frequency(uint64_t frequency);
};
