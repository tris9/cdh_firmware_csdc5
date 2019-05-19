#include "hardware/clock.h"
#include "hardware/init.h"
#include "hardware/uart.h"
#include "os/os.h"
#include "hardware/spi.h"

UART uart{UART::U3, 9600, GPIO::D, 8, 9};
SPI spi{SPI::S1, 9600, GPIO::A, 6, 7, 5, 4, SPI_MODE_MASTER, SPI_DIRECTION_2LINES, SPI_NSS_HARD_OUTPUT};
GPIO led{GPIO::B, 7, GPIO::OutputPP, GPIO::None, 0};

void init_func();
StaticTask<128> init_task{"INIT", 0, init_func};

void spi_test();
StaticTask<128> spi_task{"SPI", 0, spi_test};

/**
 * @brief Main entry point for Trillium's firmware.
 *
 * This is the main entry point of the firmware, where execution starts.  Note
 * that the scheduler isn't running when this function starts, so that all the
 * low-level initializatoin of the clock, hardware, and other things may happen
 * undisturbed.
 *
 * Before this function is called, a few things happen in @file
 * startup/startup.s:
 * - The C runtime is initialized:
 *  - Zero-initialized globals are zeroed out in RAM.
 *  - Initialized globals' starting values are copied out of FLASH.
 * - Global C++ constructors are called (important).
 *
 * @return int This function never returns, because the scheduler starts.
 */
int main() {
    clock_init();
    hardware_init();

    init_task.start();
    spi_task.start();
    
    // Does not return.
    os_init();
}

/**
 * The task that runs immediately after the processor starts.  Unlike
 * @ref main, it is safe to use asynchronous or blocking calls here.
 */
void init_func() {
    spi.init();

    uart.init();
    uart.transmit("init complete\n\r")
        .map([](UART::SendStatus x) { return 2; })
        .bind([](int x){ return uart.transmit("hello"); })
        .block();

    led.init();
    led.set(true);

    init_task.stop();
}

void spi_test() {
    char c[] = "8";
    while(1)
    {
        spi.transmit("a")
           .map([](SPI::SendStatus x) { return 2; });

        spi.receive(c)
           .map([](SPI::SendStatus x) { return 2; });
        uart.transmit(c).map([](UART::SendStatus x) { return 2; });
        vTaskDelay(500 / portTICK_PERIOD_MS);

    }
}