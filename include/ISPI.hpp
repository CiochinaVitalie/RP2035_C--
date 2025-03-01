#pragma once

#include <cstdint>
#include <cstddef>

class ISPI
{
public:
    virtual ~ISPI() = default;

    virtual void init() = 0;
    virtual int write(const uint8_t *data, size_t length) = 0;
    virtual int read(uint8_t *data, size_t length) = 0;
};