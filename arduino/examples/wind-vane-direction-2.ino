// Timers
unsigned long lastTime = millis();
unsigned long lastBoot = millis();
unsigned long lastRotation = millis();

// Pins
const byte windSpeedPin = 32;
const byte windDirectionPin = 33;

// Variables
int halfRotations = 0;
const int sleepSeconds = 10;

// Sleep safe variables
RTC_DATA_ATTR int bootNumber = 0;

void setup() {
  pinMode(windSpeedPin, INPUT); // Was INPUT
  digitalWrite(windSpeedPin, HIGH);  // Was HIGH; internall pull-up active
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(windSpeedPin), addRotation, HIGH);
  bootNumber++;

  Serial.print("Boot number: ");
  Serial.println(bootNumber);
}

void loop() {
  // Read resistance from pin
  const int directionResistance = analogRead(windDirectionPin);

  // Calculate rpm
  const int rotations = halfRotations / 2;
  const char * cardinalDirection = getCardinalDirection(directionResistance);
  const int interval = millis() - lastTime;
  const float multiply = 60000.00 / interval;
  const float rpm = multiply * rotations;

  Serial.print("interval: ");
  Serial.print(interval);
  Serial.print("\tmultiply: ");
  Serial.print(multiply);
  Serial.print("\trotations: ");
  Serial.print(rotations);
  Serial.print("\trpm: ");
  Serial.print(rpm);
  Serial.print("\tdirection: ");
  Serial.println(cardinalDirection);

  lastTime = millis();
  halfRotations = 0;

  if (millis() - lastBoot > 20000) {
    Serial.println("ESP32 going to Deep Sleep");
    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000);
    esp_deep_sleep_start();
  }

  delay(5000);
}

void addRotation() {
  if (millis() - lastRotation < 10) return;
  lastRotation = millis();
  halfRotations++;
}

// Measurements manually taken
int NE = 2887;
int NW = 3051;
int SE = 3183;
int N  = 3314;
int SW = 3423;
int E  = 3526;
int S  = 3810;
int W  = 3906;

// When number is in the middle of two numbers
int middle(int start, int end) {
  return min(start, end) + abs(start - end) / 2;
}

// Converts resistance to cardinal direction
// Wind heading means the direction where the wind is going to
const char* getCardinalDirection(int value) {
  if (value < middle(NE - 200, NE))
    return "";
  else if (value < middle(NE, NW))
    return "NE";
  else if (value < middle(NW, SE))
    return "NW";
  else if (value < middle(SE, N))
    return "SE";
  else if (value < middle(N, SW))
    return "N";
  else if (value < middle(SW, E))
    return "SW";
  else if (value < middle(E, S))
    return "E";
  else if (value < middle(S, W))
    return "S";
  else if (value < middle(W, W + 200))
    return "W";
  else
    return "";
}
