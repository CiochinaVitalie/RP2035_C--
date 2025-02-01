extern "C"
{
#include "unity.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
}

#include "RP2040.hpp"
#include "Sunlight_CO2.hpp"

#define EN_PIN 20
#define NRDY_PIN 21

void setUp(void)
{
 
}

void tearDown(void)
{
    // Эта функция выполняется после каждого теста
}

void test_addition(void)
{
    TEST_ASSERT_EQUAL_INT(2, 1 + 1); // Проверка суммы
    TEST_ASSERT_EQUAL_INT(4, 2 + 2); // Еще одна проверка
}

void test_string(void)
{
    TEST_ASSERT_EQUAL_STRING("hello", "hello"); // Проверка строк
}

int main(void)
{
    stdio_init_all();



    UNITY_BEGIN();           // Инициализация тестов
    RUN_TEST(test_addition); // Запуск тестов
    RUN_TEST(test_string);
    return UNITY_END(); // Завершение тестирования
}