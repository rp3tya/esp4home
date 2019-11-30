const uint8_t RX_PIN = D6;
volatile uint64 pack = 0;

void setup() {
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(RX_PIN), handler, CHANGE);
}

void loop() {
  noInterrupts();
  if (pack > 0) {
    Serial.println();
    pack = 0;
  }
  interrupts();
}

static inline unsigned int diff(int A, int B) {
  return abs(A - B);
}
ICACHE_RAM_ATTR void handler() {
  static uint64 code = 0;
  static uint8 cnt = 0;
  static unsigned long lastTime = 0;
  //
  const unsigned long time = micros();
  const unsigned long duration = time - lastTime;
  //
  pack = code;
  //  
  lastTime = time;  
}
