/* 19.10.2024
GoControl - A GoPro remote control camera V.1
*/

#include <BLEDevice.h>
#include <ezButton.h>
#include "driver/rtc_io.h"

#define CONTROL_QUERY_SERVICE_UUID "0000fea6-0000-1000-8000-00805f9b34fb";
uint16_t CONTROL_SERVICE_UUID  = 0xFEA6;
#define INTERNAL_UUID "00002a19-0000-1000-8000-00805f9b34fb"

#define WIFI_AP_SSID_UUID "b5f90002-aa8d-11e3-9046-0002a5d5c51b"
#define WIFI_AP_PASSWORD_UUID "b5f90003-aa8d-11e3-9046-0002a5d5c51b"

#define COMMAND_REQ_UUID "b5f90072-aa8d-11e3-9046-0002a5d5c51b"
#define COMMAND_RSP_UUID "b5f90073-aa8d-11e3-9046-0002a5d5c51b"
#define SETTINGS_REQ_UUID "b5f90074-aa8d-11e3-9046-0002a5d5c51b"
#define SETTINGS_RSP_UUID "b5f90075-aa8d-11e3-9046-0002a5d5c51b"
#define QUERY_REQ_UUID "b5f90076-aa8d-11e3-9046-0002a5d5c51b"
#define QUERY_RSP_UUID "b5f90077-aa8d-11e3-9046-0002a5d5c51b"

#define NETWORK_MANAGEMENT_REQ_UUID "b5f90091-aa8d-11e3-9046-0002a5d5c51b"
#define NETWORK_MANAGEMENT_RSP_UUID "b5f90092-aa8d-11e3-9046-0002a5d5c51b"

bool GoProFound = false;
bool IsConnected = false;
BLEScan *pBLEScan;
int scanTime = 5;  //In seconds
static BLEAdvertisedDevice *GoProAddress;
static BLERemoteCharacteristic *pRemoteCharacteristicREQ; //Request UUID
static BLERemoteCharacteristic *pRemoteCharacteristicRSP; // Responce UUID
byte ActionStart[] = {0x03, 0x01, 0x01, 0x01};  
byte ActionStop[] = {0x03, 0x01, 0x01, 0x00};  

byte PhotoMode[] = {0x04, 0x3E, 0x02, 0x03, 0xE9};  
byte VideoMode[] = {0x04, 0x3E, 0x02, 0x03, 0xE8};  
byte TimelapsMode[] = {0x04, 0x3E, 0x02, 0x03, 0xEA};  
int CamMode = 0; // Camera mode - 0 = Photo, 1 = Video, 2 = Timelaps
int DisconnectCount = 0; //Count no of disconnections
int NoOfScans = 3; // BLE scans before going to deep sleep
int ScanDone = 0; // Number of BLE scans that done

ezButton Button_1(1,INPUT_PULLUP);
ezButton Button_2(10,INPUT_PULLUP);

static BLEUUID serviceUUID(CONTROL_SERVICE_UUID);
static BLEUUID charUUID_REQ(COMMAND_REQ_UUID);
static BLEUUID charUUID_RSP(COMMAND_RSP_UUID);

// Deep sleep parameters
#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)  // 2 ^ GPIO_NUMBER in hex
#define USE_EXT0_WAKEUP 1               // 1 = EXT0 wakeup, 0 = EXT1 wakeup
#define WAKEUP_GPIO GPIO_NUM_7     // Wateup pin
bool Wakup_RTC_CNTL = false;

// Wakeup reason
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      Wakup_RTC_CNTL = true;
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
}


class ClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {
    Serial.println("onCOnnect");
  }

  void onDisconnect(BLEClient *pclient) {
    Serial.println("onDisconnect");
    IsConnected = false;
  }
};

// Notify callback
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
  Serial.print("[*] Notify callback, data length: ");
  Serial.print(length);
  Serial.print(" ,data: ");
  for (int i = 0; i < length ; i++){
    if(pData[i] < 9){
      Serial.print("0");
    }
    Serial.print(pData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

// Connect to BLE Server
void ConnectToBLE() {
  Serial.print("[*] Connecting to: ");
  Serial.println(GoProAddress->getAddress().toString().c_str());
  BLEClient *pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new ClientCallback());
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  pClient->connect(GoProAddress); 
  if (!pClient->isConnected()) { // Check if connected
    Serial.println("[X] Client not connected.");
    return;
  }
  pClient->setMTU(517);  //set client to request maximum MTU from server (default is 23 otherwise)
  Serial.println("[V] Connected to GoPro camera");
  RGBLed('g');
  Serial.println("[*] Attempting to connect to service...");
    
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);  // Obtain a reference to the service we are after in the remote BLE server.
  if (pRemoteService == nullptr) {
    Serial.print("[X] Failed to find SERVICE UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
  } else {
    Serial.println("[V] Connected to SERVICE UUID");
    // CONNECTING TO REQUEST uuid
      pRemoteCharacteristicREQ = pRemoteService->getCharacteristic(charUUID_REQ); //request UUID
      if (pRemoteCharacteristicREQ == nullptr) {
        Serial.println("[X] Failed to find REQUEST Characteristic UUID: ");
        pClient->disconnect();
      } else{
        Serial.println("[V] Connected to REQUEST Characteristic UUID");
        IsConnected = true;
      }
    // CONNECTING TO RESPONCE uuid
      pRemoteCharacteristicRSP = pRemoteService->getCharacteristic(charUUID_RSP); //responce UUID
      if (pRemoteCharacteristicRSP == nullptr) {
        Serial.println("[X] Failed to find RESPONCE Characteristic UUID: ");
        pClient->disconnect();
      } else{
        pRemoteCharacteristicRSP->registerForNotify(notifyCallback);
        Serial.println("[V] Registering RESPONCE UUID");
      }
  }
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("[*] Device Name: ");
      Serial.print(advertisedDevice.getName().c_str());
      Serial.print(", MAC Address: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) { //search for service UUID instead of name
      pBLEScan->stop(); //stop scan
      Serial.println("[V] device found !");
      GoProFound = true;
      GoProAddress = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

// Scan BLE devices
void scanBLEDevices() {
  Serial.println("[*] Scanning for BLE devices...");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);  // less or equal setInterval value
  pBLEScan->setActiveScan(true);
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("[*] Devices found: ");
  Serial.println(foundDevices->getCount());
  Serial.println("[*] Scan complete !"); 
  }

void setup() {

  Serial.begin(115200);
  Serial.println("-- GoControl --");
  RGBLed('o');
    //Print the wakeup reason for ESP32
    print_wakeup_reason();
    //deep sleep setup
    esp_sleep_enable_ext1_wakeup_io(BUTTON_PIN_BITMASK(WAKEUP_GPIO), ESP_EXT1_WAKEUP_ANY_LOW); 
    rtc_gpio_pulldown_en(WAKEUP_GPIO);  // GPIO7 is tie to GND in order to wake up in HIGH
    rtc_gpio_pullup_dis(WAKEUP_GPIO);   // Disable PULL_UP in order to allow it to wakeup on HIGH
  BLEDevice::init("");

  while  (!GoProFound){
    scanBLEDevices(); // Scan devices
    if (GoProFound){ // Device found after scaning    
      ConnectToBLE();
      ScanDone == 0;
    } else {
      Serial.print("[X] No GoPro camera found, Scans done: ");
      Serial.println(ScanDone);
      delay(500); 
      if (ScanDone == NoOfScans ){ //Go to sleep
        Serial.println("[*] Device going to deep sleep");
        esp_deep_sleep_start();
      }
      ScanDone ++; //TODO fix first button press after awake
    }
  }
}

void loop() {
  Button_1.loop();
  Button_2.loop();
  if (IsConnected){
    DisconnectCount == 0;
    // Button 1
    if(Button_1.isPressed()){
      Serial.println("[*] Button_1 pressed");
      pRemoteCharacteristicREQ->writeValue(ActionStart,sizeof(ActionStart)); //write value to camera
      RGBLed('b');
    }

    if(Button_1.isReleased()){
      Serial.println("[*] Button_1 released");
      if (Wakup_RTC_CNTL){ // Check if button pressed to wake up controller
        Wakup_RTC_CNTL = false;
      } else {
        pRemoteCharacteristicREQ->writeValue(ActionStop,sizeof(ActionStop)); //write value to camera
        delay(200);
        RGBLed('g');
      }
    }

    // Burron 2
    if(Button_2.isPressed()){
      Serial.println("[*] Button_2 pressed");
      switch (CamMode) {
      case 0:
        CamMode = 1;
        pRemoteCharacteristicREQ->writeValue(VideoMode,sizeof(VideoMode)); //write value to camera
        break;
      case 1:
        CamMode = 2;
        pRemoteCharacteristicREQ->writeValue(TimelapsMode,sizeof(TimelapsMode)); //write value to camera
        break;
      case 2:
        CamMode = 0; 
        pRemoteCharacteristicREQ->writeValue(PhotoMode,sizeof(PhotoMode)); //write value to camera
        break;
      }
    Serial.print("[*] CamMode :");
    Serial.println(CamMode);
    RGBLed('y');
    }

    if(Button_2.isReleased()){
      Serial.println("[*] Button_2 released");
      delay(200);
      RGBLed('g');
    }
  } else {
      Serial.print("[X] Camera not connected, scan count :");
      Serial.println(DisconnectCount);
      RGBLed('r');
      DisconnectCount ++;
      if (DisconnectCount < 5) { // Retry to connect 5 times, then go to sleep
        ConnectToBLE();
      } else {
        RGBLed('o');
        delay(200);
        Serial.println("[*] Device going to deep sleep");
        esp_deep_sleep_start();
      }
  }

  delay(200);
}