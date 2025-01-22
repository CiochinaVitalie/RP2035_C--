#pragma once

#include <cstdint>
#include <cstddef>

class IGPIO
{
public:
    virtual ~IGPIO() = default;

    virtual void set_high(int pin) = 0;
    virtual void set_low(int pin) = 0;
    virtual bool read(int pin) = 0;
    virtual void output_conf(int pin) = 0;
    virtual void input_conf(int pin) = 0;
};
