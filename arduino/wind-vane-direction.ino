int lastRead;
int lastTime = millis();

void setup() {
  lastRead = 1;
  Serial.begin(115200);
}

void loop() {
  int read = analogRead(33);

  // Only update with substantial change
  if (abs(read - lastRead) > 70 || millis() - lastTime > 5000) {
    Serial.print(read);
    Serial.print("\t");

    const char * cardinalDirection = getCardinalDirection(read);
    Serial.println(cardinalDirection);
    
    lastRead = read;
    lastTime = millis();
  }

  delay(500);
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
