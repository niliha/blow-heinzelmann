#include <Arduino.h>
#include <DFPlayer.h>
#include "driver/rtc_io.h"

#define MP3_SERIAL_SPEED 9600   // DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT 1000 // average DFPlayer response timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

DFPlayer mp3; // connect DFPlayer RX-pin to GPIO15(TX) & DFPlayer TX-pin to GPIO13(RX)
constexpr auto WAKEUP_PIN = GPIO_NUM_2;
constexpr auto BUSY_PIN = 15;

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

void enter_deep_sleep()
{
    // Configure Pull-Up on wakeup pin
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    rtc_gpio_pulldown_dis(WAKEUP_PIN);
    rtc_gpio_pullup_en(WAKEUP_PIN);

    esp_sleep_enable_ext1_wakeup(1ULL << WAKEUP_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
    printf("Entering deep sleep\n");
    esp_deep_sleep_start();
    printf("Should never be printed\n");
}

extern "C" void app_main()
{
    initArduino();

    Serial.begin(115200);
    Serial1.begin(9600);
    while (!Serial || !Serial1)
    {
        delay(100);
    }

    pinMode(BUSY_PIN, INPUT_PULLUP);

    // Increment boot number and print it every reboot
    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    print_wakeup_reason();

    printf("Initializing MP3 player...\n");
    mp3.begin(Serial1, MP3_SERIAL_TIMEOUT, DFPLAYER_MINI, true);
    // mp3.wakeup(2);
    // delay(1000);
    // mp3.stop();
    // delay(1000);
    // mp3.reset();
    // delay(1000);
    // mp3.setSource(2);
    // delay(1000);
    // mp3.setEQ(0);
    // delay(1000);
    // mp3.setVolume(20);
    // delay(1000);

    auto numFolders = mp3.getTotalFolders();
    Serial.print("Folders: ");
    Serial.println(numFolders);
    delay(2000);

    for (int i = 1; i <= numFolders; i++)
    {
        auto numTracks = mp3.getTotalTracksFolder(i);
        Serial.print(numTracks);
        Serial.print(" tracks in folder ");
        Serial.println(i);
        delay(2000);
    }

    for (int i = 0; i < 999; i++)
    {

        Serial.println("Playing blow");
        mp3.playFolder(1, random(1, 4));
        delay(2000);
        while (digitalRead(BUSY_PIN) == LOW)
        {
            Serial.println("Waiting for track to finish...");
            delay(500);
        }

        Serial.println("Playing heinzelmann");
        mp3.playFolder(2, random(1, 4));
        delay(2000);
        while (digitalRead(BUSY_PIN) == LOW)
        {
            Serial.println("Waiting for track to finish...");
            delay(500);
        }
    }

    enter_deep_sleep();
}