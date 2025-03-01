#include "sx1278.hpp"

void SX1278::cs_set()
{
    gpio->set_high(PIN_NSS);
}
void SX1278::cs_reset()
{
    gpio->set_low(PIN_NSS);
}
void SX1278::reset()
{
    gpio->set_low(PIN_RST);
    delay->wait_ms(1);
    gpio->set_high(PIN_RST);
    delay->wait_ms(10);
}

uint8_t SX1278::SPIRead(uint8_t addr) {
    uint8_t tmp = addr & 0x7F;
    cs_reset();
    spi->write(&tmp, 1);
    spi->read(&tmp, 1);
    cs_set();
    return tmp;
}

void SX1278::SPIWrite(uint8_t addr, uint8_t cmd) {
    cs_reset();
    uint8_t addr_with_cmd = addr | 0x80;
    spi->write(&addr_with_cmd, 1);
    spi->write(&cmd, 1);
    cs_set();
}

void SX1278::sleep()
{
    SPIWrite(LR_RegOpMode, 0x08);
	status = SLEEP;
}

int SX1278::LoRaEntryRx(uint8_t length, uint32_t timeout)
{
    uint8_t data;
    // spi->read(addr, &data, 1);
    return 0;
}
void SX1278::standby() {
	SPIWrite(LR_RegOpMode, 0x09);
	status = STANDBY;
}
void SX1278::entryLoRa() {
	SPIWrite(LR_RegOpMode, 0x88);
}
void SX1278::setFrequency(uint64_t frequency) {
    uint64_t frf = (frequency << 19) / 32000000;
    SPIWrite(LR_RegFrMsb, (uint8_t)(frf >> 16));
    SPIWrite(LR_RegFrMid, (uint8_t)(frf >> 8));
    SPIWrite(LR_RegFrLsb, (uint8_t)(frf));
}
void SX1278::setPower(uint8_t power) {
    if (power > 20) {
        power = 20;
    } else if (power < 5) {
        power = 5;
    }
    power = 0x87 + power;
    SPIWrite(LR_RegPaConfig, power);
}

void SX1278::setLoRaMode() {
    SPIWrite(LR_RegOpMode, 0x88);
}
void SX1278::setLoRa() {
    setLoRaMode();
    SPIWrite(LR_RegModemConfig1, 0x72);
    SPIWrite(LR_RegModemConfig2, 0x74);
    SPIWrite(LR_RegModemConfig3, 0x04);
    SPIWrite(LR_RegSymbTimeoutLsb, 0xFF);
    SPIWrite(LR_RegPreambleMsb, 0x00);
    SPIWrite(LR_RegPreambleLsb, 6);
    SPIWrite(LR_RegPayloadLength, 0xFF);
    SPIWrite(LR_RegFifoRxBaseAddr, 0);
    SPIWrite(LR_RegFifoTxBaseAddr, 0);
    SPIWrite(LR_RegOpMode, 0x88);
}
void SX1278::setLoRaTx() {
    SPIWrite(LR_RegOpMode, 0x8B);
}
void SX1278::setLoRaRx() {
    SPIWrite(LR_RegOpMode, 0x8D);
}
void SX1278::setLoRaTxPacket(uint8_t *buffer, uint8_t size) {
    SPIWrite(LR_RegIrqFlags, 0xFF);
    SPIWrite(LR_RegPayloadLength, size);
    SPIWrite(LR_RegFifoAddrPtr, 0);
    for (int i = 0; i < size; i++) {
        SPIWrite(LR_RegFifo, buffer[i]);
    }
}
void SX1278::getLoRaRxPacket(uint8_t *buffer, uint8_t *size) {
    uint8_t irqFlags = SPIRead(LR_RegIrqFlags);
    SPIWrite(LR_RegIrqFlags, 0xFF);
    if ((irqFlags & 0x20) && (irqFlags & 0x40)) {
        *size = 0;
    } else {
        uint8_t currentAddr = SPIRead(LR_RegFifoRxCurrentaddr);
        uint8_t receivedCount = SPIRead(LR_RegRxNbBytes);
        *size = receivedCount;
        SPIWrite(LR_RegFifoAddrPtr, currentAddr);
        for (int i = 0; i < receivedCount; i++) {
            buffer[i] = SPIRead(LR_RegFifo);
        }
    }
}
void SX1278::clearLoRaIrq() {
    SPIWrite(LR_RegIrqFlags, 0xFF);
}
void SX1278::setLoRaIrq() {
    SPIWrite(REG_LR_DIOMAPPING1, 0x01);
    SPIWrite(REG_LR_DIOMAPPING2, 0x00);
}
void SX1278::setLoRaRxTimeout(uint32_t timeout) {
    uint32_t symbolTimeout = timeout * 1000 / 31.25;
    SPIWrite(LR_RegSymbTimeoutLsb, symbolTimeout);
}
void SX1278::setLoRaRxSingle() {
    SPIWrite(LR_RegOpMode, 0x8D);
}
void SX1278::setLoRaRxContinuous() {
    SPIWrite(LR_RegOpMode, 0x8D);
}
void SX1278::setLoRaBW(uint8_t LoRa_BW) {
    uint8_t tmp;
    tmp = SPIRead(LR_RegModemConfig1);
    tmp &= 0x0F;
    LoRa_BW <<= 4;
    tmp |= LoRa_BW;
    SPIWrite(LR_RegModemConfig1, tmp);
}
void SX1278::setLoRaSF(uint8_t LoRa_SF) {
    uint8_t tmp;
    switch (LoRa_SF) {
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
    SPIWrite(LR_RegModemConfig2, tmp);
}
void SX1278::setLoRaCR(uint8_t LoRa_CR) {
    uint8_t tmp;
    tmp = SPIRead(LR_RegModemConfig1);
    tmp &= 0xF1;
    LoRa_CR <<= 1;
    tmp |= LoRa_CR;
    SPIWrite(LR_RegModemConfig1, tmp);
}
void SX1278::setLoRaCRC(uint8_t LoRa_CRC) {
    uint8_t tmp;
    tmp = SPIRead(LR_RegModemConfig2);
    tmp &= 0xFB;
    LoRa_CRC <<= 2;
    tmp |= LoRa_CRC;
    SPIWrite(LR_RegModemConfig2, tmp);
}
void SX1278::setLoRaPreambleLength(uint16_t LoRa_PreambleLength) {
    SPIWrite(LR_RegPreambleMsb, (uint8_t)(LoRa_PreambleLength >> 8));
    SPIWrite(LR_RegPreambleLsb, (uint8_t)(LoRa_PreambleLength));
}
void SX1278::setLoRaHeaderMode(uint8_t LoRa_HeaderMode) {
    uint8_t tmp;
    tmp = SPIRead(LR_RegModemConfig1);
    tmp &= 0xFE;
    LoRa_HeaderMode &= 0x01;
    tmp |= LoRa_HeaderMode;
    SPIWrite(LR_RegModemConfig1, tmp);
}
void SX1278::setLoRaIQInverted(uint8_t LoRa_IQInverted) {
    uint8_t tmp;
    tmp = SPIRead(LR_RegInvertIQ);
    tmp &= 0xBF;
    LoRa_IQInverted <<= 6;
    tmp |= LoRa_IQInverted;
    SPIWrite(LR_RegInvertIQ, tmp);
}
void SX1278::setLoRaSymbTimeout(uint16_t LoRa_SymbTimeout) {
    SPIWrite(LR_RegSymbTimeoutLsb, (uint8_t)(LoRa_SymbTimeout));
}
void SX1278::setLoRaSyncWord(uint8_t sw) {
    SPIWrite(LR_RegSyncWord, sw);
}
