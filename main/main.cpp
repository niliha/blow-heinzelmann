#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include "esp_log.h"

constexpr int MOSFET_PIN = 6;
constexpr int BLOW_FOLDER_NUM = 1;
constexpr int HEINZELMANN_FOLDER_NUM = 2;
constexpr int VOLUME_PERCENT = 50;

RTC_DATA_ATTR int bootCount = 0;
DFRobotDFPlayerMini player;

void enterDeepSleep()
{
    // Configure pull-up resistor on wakeup pin that stays enabled during deep sleep
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    rtc_gpio_pulldown_dis(WAKEUP_PIN);
    rtc_gpio_pullup_en(WAKEUP_PIN);

    ESP_LOGI("Entering deep sleep...");
    esp_sleep_enable_ext1_wakeup(1ULL << WAKEUP_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
    esp_deep_sleep_start();
}

void initializePlayer()
{
    ESP_LOGI("Initializing DFPlayer...");
    if (!player.begin(Serial1, /*isACK = */ true, /*doReset = */ true))
    {
        ESP_LOGE("Could not initialize DFPlayer. Restarting...");
        ESP.restart();
    }

    player.volume(map(VOLUME_PERCENT, 0, 100, 0, 30));
}

void awaitPlaybackFinished(int timeout_seconds)
{
    int timeoutMillis = timeout_seconds * 1000;
    auto startMillis = millis();

    ESP_LOGI("Waiting for track to finish...");
    while (millis() - startMillis < timeoutMillis)
    {
        if (player.available() && player.readType() == DFPlayerPlayFinished)
        {
            ESP_LOGI("Track finished");
            return;
        }
        delay(100);
    }

    ESP_LOGE("Timed out waiting %d seconds for track to finish", timeout_seconds);
}

awaitPlaybackFinished()
{
    awaitPlaybackFinished(30);
}

extern "C" void app_main()
{
    initArduino();

    ESP_LOGI("Boot count: %d", ++bootCount);
    if (bootCount == 1)
    {
        ESP_LOGI("Entering deep sleep on first boot...");
        enterDeepSleep();
    }

    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, HIGH);

    Serial0.begin(115200); // Serial log
    Serial1.begin(9600);   // DFPlayer Mini
    while (!Serial0 || !Serial1)
    {
        delay(100);
    }

    initializePlayer();

    int blowCount = player.readFileCountsInFolder(BLOW_FOLDER_NUM);
    int heinzelmannCount = player.readFileCountsInFolder(HEINZELMANN_FOLDER_NUM);
    if (blowCount <= 0 || heinzelmannCount <= 0)
    {
        ESP_LOGE("Could not find sound samples on SD card. Restarting...");
        ESP.restart();
    }

    ESP_LOGI("Playing blow...");
    player.playFolder(BLOW_FOLDER_NUM, random(1, blowCount + 1));
    awaitPlaybackFinished();

    ESP_LOGI("Playing heinzelmann...");
    player.playFolder(HEINZELMANN_FOLDER_NUM, random(1, heinzelmannCount + 1));
    awaitPlaybackFinished();

    enterDeepSleep();
}
