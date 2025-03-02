#pragma once

#include <cstdint>
#include <cstddef>

class IGPIO
{
public:
    virtual ~IGPIO() = default;

    virtual void set_high(void* pin) = 0;
    virtual void set_low(void* pin) = 0;
    virtual bool read(void* pin) = 0;
    virtual void output_conf(void* pin) = 0;
    virtual void input_conf(void* pin) = 0;
};
