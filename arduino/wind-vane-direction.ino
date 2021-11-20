int LastValue;

void setup() {
  LastValue = 1;
  Serial.begin(115200);
}

void loop() {
  int VaneValue = analogRead(33);

  // Only update the display if change greater than 2 degrees.
  if (abs(VaneValue - LastValue) > 70)
  {
    Serial.print(VaneValue);
    Serial.print("\t");
    getHeading(VaneValue);
    LastValue = VaneValue;
  }

  delay(500);
}


int NE = 2887;
int NW = 3051;
int SE = 3183;
int N  = 3314;
int SW = 3423;
int E  = 3526;
int S  = 3810;
int W  = 3906;

int middle(int start, int end) {
  return min(start, end) + abs(start - end) / 2;
}

// Converts compass direction to heading
void getHeading(int VaneValue) {
  if (VaneValue < middle(NE - 200, NE))
    Serial.println("TOO LOW");
  else if (VaneValue < middle(NE, NW))
    Serial.println("NE");
  else if (VaneValue < middle(NW, SE))
    Serial.println("NW");
  else if (VaneValue < middle(SE, N))
    Serial.println("SE");
  else if (VaneValue < middle(N, SW))
    Serial.println("N");
  else if (VaneValue < middle(SW, E))
    Serial.println("SW");
  else if (VaneValue < middle(E, S))
    Serial.println("E");
  else if (VaneValue < middle(S, W))
    Serial.println("S");
  else if (VaneValue < middle(W, W + 200))
    Serial.println("W");
  else
    Serial.println("TOO HIGH");
} 
