#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <esp_log.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

constexpr int MOSFET_PIN = 6;
constexpr int GATE_DRIVER_PIN = 3;
constexpr int BLOW_FOLDER_NUM = 1;
constexpr int HEINZELMANN_FOLDER_NUM = 2;
constexpr int VOLUME_PERCENT = 60;
constexpr gpio_num_t WAKEUP_PIN = GPIO_NUM_2;

RTC_DATA_ATTR int bootCount = 0;
DFRobotDFPlayerMini player;

constexpr char *TAG = "main";

void enterDeepSleep()
{
    ESP_LOGI(TAG, "Enabled pull-up resistor on wakeup pin during deep sleep");
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    rtc_gpio_pulldown_dis(WAKEUP_PIN);
    rtc_gpio_pullup_en(WAKEUP_PIN);

    ESP_LOGI(TAG, "Turning off power supply for MP3 player...");
    pinMode(GATE_DRIVER_PIN, OUTPUT);
    digitalWrite(GATE_DRIVER_PIN, HIGH);
    rtc_gpio_hold_en((gpio_num_t)GATE_DRIVER_PIN); // Required to hold pin state during deep sleep

    ESP_LOGI(TAG, "Entering deep sleep...");
    esp_sleep_enable_ext1_wakeup(1ULL << WAKEUP_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
    esp_deep_sleep_start();
}

void initializePlayer()
{
    ESP_LOGI(TAG, "Turning on power supply for MP3 player...");
    rtc_gpio_hold_dis((gpio_num_t)GATE_DRIVER_PIN); // Required to change pin state after deep sleep
    pinMode(GATE_DRIVER_PIN, OUTPUT);
    digitalWrite(GATE_DRIVER_PIN, LOW);

    ESP_LOGI(TAG, "Initializing MP3 player...");
    if (!player.begin(Serial1, /*isACK = */ true, /*doReset = */ true))
    {
        ESP_LOGE(TAG, "Could not initialize DFPlayer. Restarting...");
        ESP.restart();
    }
    ESP_LOGI(TAG, "Successfully initialized MP3 player");

    ESP_LOGI(TAG, "Setting volume to %d%%...", VOLUME_PERCENT);
    player.volume(map(VOLUME_PERCENT, 0, 100, 0, 30));
    ESP_LOGI(TAG, "Performing dummy call...");
    player.readFolderCounts(); // For some reason a dummy read is required, otherwise the next command will fail
}

void awaitPlaybackFinished(int timeout_seconds)
{
    int timeoutMillis = timeout_seconds * 1000;
    auto startMillis = millis();

    ESP_LOGI(TAG, "Waiting for track to finish...");
    while (millis() - startMillis < timeoutMillis)
    {
        if (player.available() && player.readType() == DFPlayerPlayFinished)
        {
            ESP_LOGI(TAG, "Track finished");
            return;
        }
        delay(100);
    }

    ESP_LOGE(TAG, "Timed out waiting %d seconds for track to finish", timeout_seconds);
}

void awaitPlaybackFinished()
{
    awaitPlaybackFinished(30);
}

extern "C" void app_main()
{
    initArduino();

    ESP_LOGI(TAG, "Boot count: %d", ++bootCount);

    Serial0.begin(115200); // Serial log
    Serial1.begin(9600);   // DFPlayer Mini
    while (!Serial0 || !Serial1)
    {
        delay(100);
    }

    initializePlayer();

    int blowCount = player.readFileCountsInFolder(BLOW_FOLDER_NUM);
    delay(100);
    blowCount = player.readFileCountsInFolder(BLOW_FOLDER_NUM); // Read twice since first read gives wrong result
    int heinzelmannCount = player.readFileCountsInFolder(HEINZELMANN_FOLDER_NUM);
    if (blowCount <= 0 || heinzelmannCount <= 0)
    {
        ESP_LOGE(TAG, "Could not find sound samples on SD card (%d blows, %d heinzelmanns). Restarting...", blowCount, heinzelmannCount);
        ESP.restart();
    }
    ESP_LOGI("TAG", "Found %d blow and %d heinzelmann sound samples", blowCount, heinzelmannCount);

    ESP_LOGI(TAG, "Playing blow...");
    player.playFolder(BLOW_FOLDER_NUM, random(1, blowCount + 1));
    awaitPlaybackFinished();

    ESP_LOGI(TAG, "Playing heinzelmann...");
    player.playFolder(HEINZELMANN_FOLDER_NUM, random(1, heinzelmannCount + 1));
    awaitPlaybackFinished();

    enterDeepSleep();
}
