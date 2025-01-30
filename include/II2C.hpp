#pragma once

#include <cstdint>
#include <cstddef>

class II2C
{
public:
    virtual ~II2C() = default;

    virtual void init() = 0;
    virtual int write(uint8_t address, const uint8_t *data, size_t length) = 0;
    virtual int read(uint8_t address, uint8_t *data, size_t length) = 0;
};