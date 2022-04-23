#include <Arduino.h>
#include <WiFi.h>

#include "app_gui.h"
#include "app_ipc.h"
#include "app_manager.h"
#include "app_common.h"
#include "app_pr.h"

/* Task Handles */
static TaskHandle_t ipc;
static TaskHandle_t firebase;
static TaskHandle_t display;
static TaskHandle_t manager;
static TaskHandle_t pwrres;
void setup()
{
    ipc2ManagerDeviceInfoQueue = xQueueCreate(MAX_PENDING_DEVICEINFO_QUEUE, sizeof(DeviceInfoExt));
    manager2GuiDeviceIndexQueue = xQueueCreate(MAX_PENDING_DEVICEINFO_QUEUE, sizeof(uint8_t));
    APP_LOG_START();
    Serial1.begin(921600, SERIAL_8O1, IPC_RX, IPC_TX);
    xTaskCreatePinnedToCore( // Use xTaskCreate()
        displayTask,         // Function to be called
        "Display Task",      // Name of task
        32768,               // Stack size
        NULL,                // Parameter to pass
        0,                   // Task priority
        &display,            // Task handle
        0);                  // CPU Core
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
    xTaskCreatePinnedToCore(  // Use xTaskCreate()
        powerReserve,         // Function to be called
        "Power Reserve Task", // Name of task
        1000,                 // Stack size
        NULL,                 // Parameter to pass
        0,                    // Task priority
        &pwrres,              // Task handle
        1);                   // CPU Core
    xTaskCreatePinnedToCore(  // Use xTaskCreate()
        firebaseTask,         // Function to be called
        "Firebase Task",      // Name of task
        32768,                // Stack size
        NULL,                 // Parameter to pass
        1,                    // Task priority
        &firebase,            // Task handle
        0);                   // CPU Core
    vTaskDelete(NULL);
}

void loop()
{
}