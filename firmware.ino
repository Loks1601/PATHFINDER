// ── Pin assignments ──────────────────────────────────────────
const int BUT1 = D0;
const int BUT2 = D1;
const int BUT3 = D2;
const int BUT4 = D6;

const int LED1 = D3;   // PWM capable — used for heartbeat pulse
const int LED2 = D4;
const int LED3 = D5;
const int LED4 = D7;




const int   MAX_TAPS       = 8;      // rolling window for BPM calc
const float BPM_DECAY_RATE = 0.995f; // per loop tick (~1 ms)
const float BPM_MIN        = 20.0f;
const float BPM_MAX        = 200.0f;

long  tapTimes[MAX_TAPS];
int   tapHead       = 0;
int   tapCount      = 0;
float currentBPM    = 0.0f;
bool  but1Last      = HIGH;

// heartbeat pulse shape: quick rise, double-bump, long rest
// We split one beat into 4 phases (ms durations, BPM-scaled)
// Phase 0: rise     Phase 1: fall+bump  Phase 2: small bump  Phase 3: rest
float pulsePhase     = 0.0f;   // 0.0–1.0 within one beat
long  lastBeatTick   = 0;

// ── FEATURE 2 : Secret Knock (B1 then B2 → LED 2) ───────────
const long  KNOCK_WINDOW_MS = 1500;  // B2 must follow B1 within this
const long  UNLOCK_HOLD_MS  = 3000; // LED 2 stays lit this long

bool  b1WasPressed   = false;
long  b1PressTime    = 0;
bool  led2Unlocked   = false;
long  led2UnlockTime = 0;
bool  but2Last       = HIGH;

// ── FEATURE 3 : Patience Glow (hold B3 ≥ 2 s → toggle LED 3) 
const long PATIENCE_MS = 2000;

bool  led3State      = false;
long  but3HoldStart  = 0;
bool  but3Holding    = false;
bool  but3Triggered  = false; // prevent re-trigger while held
bool  but3Last       = HIGH;

// ── FEATURE 4 : Strobe Alert (hold B4 → LED4 strobes) ───────
const long STROBE_INTERVAL_MS = 80;  // on/off toggle period while held

bool  but4Last       = HIGH;
bool  led4Strobing   = false;
bool  led4State      = false;
long  led4LastToggle = 0;

// ── Helpers ──────────────────────────────────────────────────

// Simple non-blocking heartbeat brightness at given BPM, time t ms
// Returns 0–255 following a double-bump cardiac-like envelope
int heartbeatBrightness(float bpm, long nowMs) {
  if (bpm < BPM_MIN) return 0;

  float periodMs    = 60000.0f / bpm;
  float phase       = fmod((float)nowMs, periodMs) / periodMs; // 0–1

  // Double-bump envelope: two quick Gaussian peaks then silence
  // Peak 1 at phase=0.10, peak 2 at phase=0.22
  auto gauss = [](float x, float mu, float sigma) -> float {
    float d = (x - mu) / sigma;
    return expf(-0.5f * d * d);
  };

  float env = gauss(phase, 0.10f, 0.04f) * 1.0f
            + gauss(phase, 0.22f, 0.035f) * 0.6f;

  // Clamp and scale
  if (env > 1.0f) env = 1.0f;
  return (int)(env * 255.0f);
}

// Estimate BPM from the rolling tap window
float estimateBPM() {
  if (tapCount < 2) return 0.0f;
  int samples = (tapCount < MAX_TAPS) ? tapCount : MAX_TAPS;
  // oldest sample in ring buffer
  int oldest  = (tapHead - samples + MAX_TAPS) % MAX_TAPS;
  int newest  = (tapHead - 1 + MAX_TAPS) % MAX_TAPS;
  long spanMs = tapTimes[newest] - tapTimes[oldest];
  if (spanMs <= 0) return 0.0f;
  float intervals = (float)(samples - 1);
  float avgMs     = (float)spanMs / intervals;
  float bpm       = 60000.0f / avgMs;
  if (bpm < BPM_MIN) bpm = 0.0f;
  if (bpm > BPM_MAX) bpm = BPM_MAX;
  return bpm;
}

// ─────────────────────────────────────────────────────────────
void setup() {
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(BUT3, INPUT_PULLUP);
  pinMode(BUT4, INPUT_PULLUP);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  analogWrite(LED1, 0);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);

  Serial.begin(115200);
  Serial.println("Firmware ready — Four Personalities");
}

// ─────────────────────────────────────────────────────────────
void loop() {
  long now = millis();

  bool but1 = digitalRead(BUT1);  // LOW = pressed
  bool but2 = digitalRead(BUT2);
  bool but3 = digitalRead(BUT3);
  bool but4 = digitalRead(BUT4);

  // ── Feature 1 : Heartbeat Tap ──────────────────────────────
  // Detect rising edge of press (HIGH→LOW for active-low)
  if (but1 == LOW && but1Last == HIGH) {
    // Register tap
    tapTimes[tapHead] = now;
    tapHead = (tapHead + 1) % MAX_TAPS;
    if (tapCount < MAX_TAPS) tapCount++;

    currentBPM = estimateBPM();
    Serial.print("Tap! BPM = "); Serial.println(currentBPM);

    // Also flag that B1 was pressed (for secret knock)
    b1WasPressed = true;
    b1PressTime  = now;
  }
  but1Last = but1;

  // Decay BPM naturally when user stops tapping
  // (if last tap was >2 beats ago at current BPM, start decaying)
  if (currentBPM > 0.0f && tapCount >= 1) {
    long lastTap  = tapTimes[(tapHead - 1 + MAX_TAPS) % MAX_TAPS];
    float beatMs  = 60000.0f / currentBPM;
    if ((now - lastTap) > (long)(beatMs * 2.5f)) {
      currentBPM *= BPM_DECAY_RATE;
      if (currentBPM < BPM_MIN) currentBPM = 0.0f;
    }
  }

  // Drive LED 1 with heartbeat brightness
  analogWrite(LED1, heartbeatBrightness(currentBPM, now));

  // ── Feature 2 : Secret Knock ────────────────────────────────
  if (but2 == LOW && but2Last == HIGH) {
    // Check if B1 was pressed recently
    if (b1WasPressed && (now - b1PressTime) <= KNOCK_WINDOW_MS) {
      Serial.println("Secret knock! LED2 unlocked.");
      led2Unlocked   = true;
      led2UnlockTime = now;
      b1WasPressed   = false;  // consume the B1 press
    } else {
      Serial.println("B2 alone — nothing happens.");
    }
  }
  but2Last = but2;

  // Expire B1 press if window elapsed
  if (b1WasPressed && (now - b1PressTime) > KNOCK_WINDOW_MS) {
    b1WasPressed = false;
  }

  // Keep LED 2 lit for UNLOCK_HOLD_MS then fade
  if (led2Unlocked) {
    long elapsed = now - led2UnlockTime;
    if (elapsed < UNLOCK_HOLD_MS) {
      // Simple fade-out in last 500 ms
      long fadeStart = UNLOCK_HOLD_MS - 500;
      if (elapsed > fadeStart) {
        float t = (float)(elapsed - fadeStart) / 500.0f;
        analogWrite(LED2, (int)((1.0f - t) * 255));
      } else {
        digitalWrite(LED2, HIGH);
      }
    } else {
      analogWrite(LED2, 0);
      led2Unlocked = false;
    }
  }

  // ── Feature 3 : Patience Glow ───────────────────────────────
  if (but3 == LOW) {                         // button held
    if (!but3Holding) {
      but3Holding    = true;
      but3HoldStart  = now;
      but3Triggered  = false;
    }
    if (!but3Triggered && (now - but3HoldStart) >= PATIENCE_MS) {
      led3State     = !led3State;            // toggle
      but3Triggered = true;
      Serial.print("Patience rewarded! LED3 = ");
      Serial.println(led3State ? "ON" : "OFF");
    }
  } else {                                   // button released
    but3Holding   = false;
    but3Triggered = false;
  }
  but3Last = but3;

  digitalWrite(LED3, led3State ? HIGH : LOW);

  // ── Feature 4 : Strobe Alert (hold B4 → LED4 strobes) ──────
  if (but4 == LOW && but4Last == HIGH) {
    // Just pressed — start strobing
    led4Strobing   = true;
    led4LastToggle = now;
    led4State      = true;
    Serial.println("Strobe alert engaged.");
  }
  if (but4 == HIGH && but4Last == LOW) {
    // Just released — stop strobing, LED off
    led4Strobing = false;
    led4State    = false;
    Serial.println("Strobe alert stopped.");
  }
  but4Last = but4;

  if (led4Strobing && (now - led4LastToggle) >= STROBE_INTERVAL_MS) {
    led4State      = !led4State;
    led4LastToggle = now;
  }

  digitalWrite(LED4, led4State ? HIGH : LOW);

  // Loop runs ~every 1 ms naturally; no delay needed
}
