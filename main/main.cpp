#include <Arduino.h>

extern "C" void app_main()
{
    initArduino();

    // Arduino-like setup()
    Serial.begin(115200);
    while (!Serial)
    {
        ; // wait for serial port to connect
    }

    // Arduino-like loop()
    while (true)
    {
        Serial.println("loop");
    }

    // WARNING: if program reaches end of function app_main() the MCU will restart.
}