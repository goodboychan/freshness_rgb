// https://rootsaid.com/arduino-ble-example/
// Characteristic info.
// https://www.arduino.cc/en/Reference/ArduinoBLEBLECharacteristicBLECharacteristic

#include <ArduinoBLE.h>

#define s0  2
#define s1  3
#define s2  4
#define s3  5
#define out 6

// Device name
const char* nameOfPeripheral = "ColorMonitor";
const char* uuidOfService = "0000181a-0000-1000-8000-00805f9b34fb";
const char* uuidOfRxChar = "00002A3D-0000-1000-8000-00805f9b34fb";
const char* uuidOfTxChar = "00002A58-0000-1000-8000-00805f9b34fb";

// BLE Service
BLEService colorService(uuidOfService);

// RX / TX Characteristics
BLEStringCharacteristic txChar(uuidOfTxChar, BLERead | BLENotify | BLEBroadcast, 12);

// Buffer to read samples into, each sample is 16-bits
volatile int samplesRead_R;
volatile int samplesRead_G;
volatile int samplesRead_B;

// function prototype to define default timeout value
static unsigned int newPulseIn(const byte pin, const byte state, const unsigned long timeout = 1000000L);

// using a macro to avoid function call overhead
#define WAIT_FOR_PIN_STATE(state) \
  while (digitalRead(pin) != (state)) { \
    if (micros() - timestamp > timeout) { \
      return 0; \
    } \
  }

static unsigned int newPulseIn(const byte pin, const byte state, const unsigned long timeout) {
  unsigned long timestamp = micros();
  WAIT_FOR_PIN_STATE(!state);
  WAIT_FOR_PIN_STATE(state);
  timestamp = micros();
  WAIT_FOR_PIN_STATE(!state);
  return micros() - timestamp;
}


/*
 *  MAIN
 */
void setup() {

  // Start serial.
  Serial.begin(9600);

  // Ensure serial port is ready.
  while (!Serial);

  // Prepare LED pins.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);

  //TCS3200
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out,  INPUT);

  digitalWrite(s0,  HIGH);
  digitalWrite(s1,  HIGH);

  // Start BLE.
  startBLE();

  // Create BLE service and characteristics.
  BLE.setDeviceName("Arduino Nano 33 BLE");
  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(colorService);
  colorService.addCharacteristic(txChar);
  BLE.addService(colorService);

  // Bluetooth LE connection handlers.
  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
  
  // Let's tell devices about us.
  BLE.advertise();
  
  // Print out full UUID and MAC address.
  Serial.println("Peripheral advertising info: ");
  Serial.print("Name: ");
  Serial.println(nameOfPeripheral);
  Serial.print("MAC: ");
  Serial.println(BLE.address());
  Serial.print("Service UUID: ");
  Serial.println(colorService.uuid());
  Serial.print("txCharacteristics UUID: ");
  Serial.println(uuidOfTxChar);

  Serial.println("Bluetooth device active, waiting for connections...");
}


void loop()
{
  BLEDevice central = BLE.central();
  
  if (central)
  {
    // Only send data if we are connected to a central device.
    while (central.connected()) {
      connectedLight();

      // Send the RGB values to the central device.
      digitalWrite(s2, LOW);
      digitalWrite(s3, LOW);
      samplesRead_R = newPulseIn(out, LOW);
      samplesRead_R = map(samplesRead_R,60,15,0,100);

      digitalWrite(s2, LOW);
      digitalWrite(s3, HIGH);
      samplesRead_B = newPulseIn(out, LOW);
      samplesRead_B = map(samplesRead_B,80,11,0,100);

      digitalWrite(s2, HIGH);
      digitalWrite(s3, HIGH);
      samplesRead_G = newPulseIn(out, LOW);
      samplesRead_G = map(samplesRead_G,80,20,0,100);

      String sample = String(samplesRead_R) + "," + String(samplesRead_B) + "," + String(samplesRead_G);
      Serial.println(sample);
      txChar.writeValue(sample);
      delay(500);
    }
  } else {
    disconnectedLight();
  }
}


/*
 *  BLUETOOTH
 */
void startBLE() {
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
}

void onBLEConnected(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  connectedLight();
}

void onBLEDisconnected(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  disconnectedLight();
}

/*
 * LEDS
 */
void connectedLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
}


void disconnectedLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
}
