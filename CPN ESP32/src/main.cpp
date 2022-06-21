#include <Arduino.h> // used for SPI/UART/GPIO functions
#include <WiFi.h>
#include "app_gui.h" // displayTask
#include "app_ipc.h" // ipcTask
#include "app_manager.h" // managerTask and firebaseTask
#include "app_common.h" // application-specific defines
#include "app_pr.h" // prTask

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
    app2ManagerDeviceReqQueue = xQueueCreate(10, sizeof(FirebaseReq_t));
    APP_LOG_START();
    Serial1.begin(921600, SERIAL_8O1, IPC_RX, IPC_TX);
    xTaskCreatePinnedToCore(
        displayTask,          // Function to be called
        "Display Task",       // Name of task
        16384,                // Stack size
        NULL,                 // Parameter to pass
        0,                    // Task priority
        &display,             // Task handle
        1);                   // CPU Core
    xTaskCreatePinnedToCore(
        managerTask,          // Function to be called
        "Manager Task",       // Name of task
        8192,                // Stack size
        NULL,                 // Parameter to pass
        0,                    // Task priority
        &manager,             // Task handle
        1);                   // CPU Core
    xTaskCreatePinnedToCore(
        ipcTask,              // Function to be called
        "IPC Task",           // Name of task
        16384,                // Stack size
        NULL,                 // Parameter to pass
        1,                    // Task priority
        &ipc,                 // Task handle
        1);                   // CPU Core
    xTaskCreatePinnedToCore(
        prTask,         // Function to be called
        "Power Reserve Task", // Name of task
        1000,                 // Stack size
        NULL,                 // Parameter to pass
        0,                    // Task priority
        &pwrres,              // Task handle
        1);                   // CPU Core
    xTaskCreatePinnedToCore(
        firebaseTask,         // Function to be called
        "Firebase Task",      // Name of task
        32768,                // Stack size
        NULL,                 // Parameter to pass
        1,                    // Task priority
        &firebase,            // Task handle
        0);                   // CPU Core
    vTaskDelete(NULL);
}

void loop(){} // UNUSED