#include <ArduinoBLE.h>

#define s0  2
#define s1  3
#define s2  4
#define s3  5
#define out 6

// Device Name
const char* nameOfPeripheral = "Color";
const char* uuidOfService = "c1a7dce8-8875-41d1-9b66-0ae2116393fe";
const char* uuidOfRxChar = "c1a72a3d-8875-41d1-9b66-0ae2116393fe";
const char* uuidOfTxChar = "c1a72a58-8875-41d1-9b66-0ae2116393fe";

// BLE Service
BLEService RGBService(uuidOfService);

// Setup the incoming data characteristic (Rx)
const int WRITE_BUFFER_SIZE = 256;
bool WRITE_BUFFER_FIXED_LENGTH = false;

// Rx/Tx Characteristics
BLECharacteristic rxChar(uuidOfRxChar, BLEWriteWithoutResponse | BLEWrite, WRITE_BUFFER_SIZE, WRITE_BUFFER_FIXED_LENGTH);
BLEByteCharacteristic txChar(uuidOfTxChar, BLERead | BLENotify | BLEBroadcast);

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[3];
volatile int samplesRead_R;
volatile int samplesRead_G;
volatile int samplesRead_B;

void startBLE(){
  if (!BLE.begin()){
    Serial.println("Starting BLE failed!");
    while(1);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  while(!Serial);

  //TCS3200
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out,  INPUT);

  digitalWrite(s0,  HIGH);
  digitalWrite(s1,  HIGH);

  //BLE
  startBLE();

  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(RGBService);
  RGBService.addCharacteristic(rxChar);
  RGBService.addCharacteristic(txChar);
  BLE.addService(RGBService);

  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

  rxChar.setEventHandler(BLEWritten, onRxCharValueUpdate);

  BLE.advertise();

  // Print out full UUID and MAC address.
  Serial.println("Peripheral advertising info: ");
  Serial.print("Name: ");
  Serial.println(nameOfPeripheral);
  Serial.print("MAC: ");
  Serial.println(BLE.address());
  Serial.print("Service UUID: ");
  Serial.println(RGBService.uuid());
  Serial.print("rxCharacteristic UUID: ");
  Serial.println(uuidOfRxChar);
  Serial.print("txCharacteristics UUID: ");
  Serial.println(uuidOfTxChar);

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEDevice central = BLE.central();
  
  if (central)
  {
    // Only send data if we are connected to a central device.
    while (central.connected()) {
      connectedLight();

      // Send the RGB values to the central device.
      digitalWrite(s2, LOW);
      digitalWrite(s3, LOW);
      samplesRead_R = pulseIn(out, LOW);
      sampleBuffer[0] = samplesRead_R;
      delay(20);

      digitalWrite(s2, LOW);
      digitalWrite(s3, HIGH);
      samplesRead_B = pulseIn(out, LOW);
      sampleBuffer[1] = samplesRead_B;
      delay(20);

      digitalWrite(s2, HIGH);
      digitalWrite(s3, HIGH);
      samplesRead_G = pulseIn(out, LOW);
      sampleBuffer[2] = samplesRead_G;
      delay(20);

//      sampleBuffer = {samplesRead_R, samplesRead_B, samplesRead_G};

      for (int i = 0; i < 3; i++){
        txChar.writeValue(sampleBuffer[i]);
      }
    }
  } else {
    disconnectedLight();
  }
}

void onRxCharValueUpdate(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, read: ");
  byte test[256];
  int dataLength = rxChar.readValue(test, 256);

  for(int i = 0; i < dataLength; i++) {
    Serial.print((char)test[i]);
  }
  Serial.println();
  Serial.print("Value length = ");
  Serial.println(rxChar.valueLength());
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

void connectedLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
}


void disconnectedLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
}
