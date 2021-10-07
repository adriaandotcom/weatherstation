// Your GPRS credentials, if any
const char apn[] = "data.lycamobile.nl"; //SET TO YOUR APN
const char gprsUser[] = "lmnl";
const char gprsPass[] = "plus";

const int interval = 1000;

// Server details
const char server[] = "66df-87-239-11-35.ngrok.io";

int pulse_counter = 0;
volatile unsigned long raintime, rainlast, raininterval, rain;

void interrupt_handler() {
  raintime = millis();
  raininterval = raintime - rainlast;

  if (raininterval > 1000) {
    rainlast = raintime;
    pulse_counter = pulse_counter + 1;
  }
}

#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Define how you're planning to connect to thea internet
// These defines are only for this example; they are not needed in other code.

// set GSM PIN, if anys
#define GSM_PIN ""

#include <TinyGsmClient.h>

#include <ArduinoHttpClient.h>

#include <Ticker.h>

#include <WiFi.h>

#include <TimeLib.h>

#include "time.h"

#ifdef DUMP_AT_COMMANDS#include <StreamDebugger.h>

StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#ifdef USE_SSL
TinyGsmClientSecure client(modem);
const int port = 443;
#else
TinyGsmClient client(modem);
const int port = 80;
#endif

HttpClient http(client, server, port);

Ticker tick;

#define uS_TO_S_FACTOR 1000000 ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60 /*  Time ESP32 will go to sleep (in seconds) */

#define PIN_TX 27
#define PIN_RX 26
#define UART_BAUD 115200
#define PWR_PIN 4
#define LED_PIN 12
#define POWER_PIN 25
#define IND_PIN 36
#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13

#define INTERRUPT_INPUT 32

void connectCellular() {
  DBG("Initializing modem...");
  if (!modem.restart()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    return;
  }

  String name = modem.getModemName();
  DBG("Modem Name:", name);

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);

  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);

    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
}

void disconnectCellular() {
  client.stop();
  SerialMon.println(F("Server disconnected"));

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));
}

bool sendRequest(String payload) {
  payload = "timestamp,sensor,value\n" + payload;

  http.beginRequest();
  http.post("/path2");
  http.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
  http.sendHeader(HTTP_HEADER_CONTENT_LENGTH, payload.length());
  http.sendHeader("Adriaan-Device-Time", getNow());
  http.sendHeader("Adriaan-Version", "1");
  http.sendHeader("Adriaan-Interval", interval);
  http.endRequest();
  http.print(payload);

  // read the status code and body of the response
  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  // SerialMon.print("Server response code: " + statusCode);

  if (String(response) == "OK") return true;
  return false;
}

void updateTime() {
  SerialAT.println("AT+CCLK?");

  delay(100);

  if (SerialAT.available()) {
    String r = SerialAT.readString();
    int base = 12;

    // +CCLK: "21/10/06,22:12:17+08"

    Serial.println("Getting time from AT: " + r.substring(base - 2, base + 18));

    setTime
      (
        r.substring(base + 7, base + 9).toInt(), //  hour
        r.substring(base + 10, base + 12).toInt(), //  minute
        r.substring(base + 13, base + 15).toInt(), //  second
        r.substring(base + 4, base + 6).toInt(), //  day
        r.substring(base + 1, base + 3).toInt(), //  month
        ("20" + r.substring(base - 2, base)).toInt() //  year
      );
  } else {
    Serial.println("[ERROR] Can't get date and time from AT");
  }
}

String payload = "";
int payloadLines = 0;

String zeroFill(int number) {
  if (number < 10) return "0" + String(number);
  return String(number);
}

// Return string in ISO 8601 format
// 2021-10-06T23:44:14 (without time zone)
String getNow() {
  time_t t2 = now();
  return String(year(t2)) +
    "-" + zeroFill(month(t2)) +
    "-" + zeroFill(day(t2)) +
    "T" + zeroFill(hour(t2)) +
    ":" + zeroFill(minute(t2)) +
    ":" + zeroFill(second(t2));
  s
}

void addToPayload(String sensor, String value) {
  payload += getNow() + "," + sensor + "," + value + "\n";
  payloadLines += 1;
}

void flushPayload() {
  payload = "";
  payloadLines = 0;
}

void sendPayload() {
  const bool ok = sendRequest(payload);
  if (ok) flushPayload();
}

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  Serial.print("Booting version 3\n");

  // Read rain switch from pins
  pinMode(INTERRUPT_INPUT, INPUT_PULLUP);
  delay(10);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_INPUT), interrupt_handler, RISING);

  delay(10);

  // Set LED OFF
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  Serial.println("\nWait...");

  delay(10000);

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(300);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  //  DBG("Initializing modem...");
  //  if (!modem.init()) {
  //    DBG("Failed to restart modem, delaying 10s and retrying");
  //    return;
  //  }
  //
  //  DBG("AT+CGMR");
  //  modem.sendAT("+CGMR");
  //  DBG("END");
}

void loop() {
  connectCellular();

  updateTime();

  // disconnectCellular();

  // Do nothing forevermore
  while (true) {
    //    Serial.print(pulse_counter);
    //    Serial.print("\n\r");
    addToPayload("rain", String(pulse_counter));
    pulse_counter = 0;

    if (payloadLines > 0 && payloadLines % 5 == 0) {
      sendPayload();
    }

    delay(interval);
  }
}
