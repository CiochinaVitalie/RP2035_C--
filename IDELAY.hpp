#pragma once

#include <cstdint>
#include <cstddef>

class IDelay
{
public:
    virtual ~IDelay() = default;

    virtual void wait_ms(uint32_t milliseconds) = 0;
};