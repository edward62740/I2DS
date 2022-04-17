#include <Arduino.h>
#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "app_common.h"



/* Task Handles */
static TaskHandle_t ipc;
static TaskHandle_t firebase;
static TaskHandle_t display;
static TaskHandle_t manager;
void setup()
{
    ipc2ManagerDeviceInfoQueue = xQueueCreate(MAX_PENDING_DEVICEINFO_QUEUE, sizeof(DeviceInfoExt));
    manager2GuiDeviceIndexQueue = xQueueCreate(MAX_PENDING_DEVICEINFO_QUEUE, sizeof(uint8_t));
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, IPC_RX, IPC_TX);
    xTaskCreatePinnedToCore( // Use xTaskCreate()
        managerTask,         // Function to be called
        "Manager Task",      // Name of task
        32768,               // Stack size
        NULL,                // Parameter to pass
        0,                   // Task priority
        &manager,            // Task handle
        1);                  // CPU Core

    xTaskCreatePinnedToCore( // Use xTaskCreate()
        ipcTask,             // Function to be called
        "IPC Task",          // Name of task
        32768,               // Stack size
        NULL,                // Parameter to pass
        1,                   // Task priority
        &ipc,                // Task handle
        1);                  // CPU Core

    xTaskCreatePinnedToCore( // Use xTaskCreate()
        firebaseTask,        // Function to be called
        "Firebase Task",     // Name of task
        32768,               // Stack size
        NULL,                // Parameter to pass
        1,                   // Task priority
        &firebase,           // Task handle
        0);                  // CPU Core

    xTaskCreatePinnedToCore( // Use xTaskCreate()
        displayTask,         // Function to be called
        "Display Task",      // Name of task
        32768,               // Stack size
        NULL,                // Parameter to pass
        0,                   // Task priority
        &display,            // Task handle
        0);                  // CPU Core
    vTaskDelete(NULL);
}

void loop()
{
}
