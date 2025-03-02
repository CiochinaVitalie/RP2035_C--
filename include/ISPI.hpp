#pragma once

#include <cstdint>
#include <cstddef>

class ISPI
{
public:
    virtual ~ISPI() = default;

    virtual void init(void* clk,void* mosi,void* miso) = 0;
    virtual uint8_t write(const uint8_t *data, size_t length) = 0;
    virtual uint8_t read(uint8_t *data, size_t length) = 0;
    virtual uint8_t write_read(const uint8_t *tx_data, uint8_t *rx_data, size_t length) = 0;
};