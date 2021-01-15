// connections
const int PIN_PIEZO = A0,
PIN_RELAY1 = 7,
PIN_RELAY2 = 8,
PIN_BUZZER_TEST = 13,
PIN_RADAR1_ECHO = A1,
PIN_RADAR1_TRIG = A2,
PIN_RADAR2_ECHO = A3,
PIN_RADAR2_TRIG = A4;

// state of the system
enum state
{
  // wait for a person to type on a typewriter
  CAN_TRIGGER,
  // turn on distractions
  CONSUME_TRIGGER, 
  // detect using ultrasonic sensors if person wants to turn off distraction
  TRIGGERED_AWAITING_RETURN,
  // distractions are turned off, system
  TRIGGERED_COOLING_DOWN
} state = CAN_TRIGGER;

// how many keys user typed on a typewriter (microphone exceeded threshold)
int user_keystrokes = 0;
unsigned long int trigger_cooldown = 0;

// state of distractions
boolean 
  is_relay1_on = false
  , is_relay2_on = false
  , is_tv_on = false;
  
bool radar1_triggered = false, radar2_triggered = false;
int await_radar_trigger = 0; // 0-none, 1-radar1, 2-radar2

// piezo
int piezo_reading = 0;
const int PIEZO_THRESHOLD = 2;

// ultrasonic sensors
const unsigned int TIMER_MEASURE_RADAR = 30
  , LAST_DISTANCE_COUNT = 10;
float 
  radar1_time = 0, 
  radar1_distance = 0;
float radar1_last_distance = 0;
float radar1_distances[LAST_DISTANCE_COUNT] = { 0 };
// radar2
float 
  radar2_time = 0, 
  radar2_distance = 0;
float radar2_last_distance = 0;
float radar2_distances[LAST_DISTANCE_COUNT] = { 0 };

struct timers
{
  unsigned long int RADAR1_MEASURE;
  unsigned long int RADAR2_MEASURE;
} timers = { 0 };

///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PIEZO, INPUT);
  
  pinMode(PIN_RADAR1_ECHO, INPUT);
  pinMode(PIN_RADAR1_TRIG, OUTPUT);
  pinMode(PIN_RADAR2_ECHO, INPUT);
  pinMode(PIN_RADAR2_TRIG, OUTPUT);
  
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  pinMode(PIN_BUZZER_TEST, OUTPUT);
}

void loop() {
  unsigned long t_now = millis();

  count_keystrokes();
  // measure radar only when distraction is on to prevent problems with processor sleep
  if (state == TRIGGERED_AWAITING_RETURN) {
    if (t_now - timers.RADAR1_MEASURE >= TIMER_MEASURE_RADAR) {
          timers.RADAR1_MEASURE = t_now;
          digitalWrite(PIN_RADAR1_TRIG, LOW);
          delayMicroseconds(2);
          digitalWrite(PIN_RADAR1_TRIG, HIGH);
          delayMicroseconds(10);
          digitalWrite(PIN_RADAR1_TRIG, LOW);
          radar1_time = pulseIn(PIN_RADAR1_ECHO, HIGH);
          float new_radar1_dist = (radar1_time/2) / 29.1;
          
          // ignore overblown values, max distance measured is 4 meters
          if (new_radar1_dist < 500) {
            radar1_distance = new_radar1_dist;
          }
    }
    if (t_now - timers.RADAR2_MEASURE >= TIMER_MEASURE_RADAR) {
          timers.RADAR2_MEASURE = t_now;
          digitalWrite(PIN_RADAR2_TRIG, LOW);
          delayMicroseconds(2);
          digitalWrite(PIN_RADAR2_TRIG, HIGH);
          delayMicroseconds(10);
          digitalWrite(PIN_RADAR2_TRIG, LOW);
          radar2_time = pulseIn(PIN_RADAR2_ECHO, HIGH);
          float new_radar2_dist = (radar2_time/2) / 29.1;
          if (new_radar2_dist < 500) {
            radar2_distance = new_radar2_dist;
          }
    }
  }
  
  // distance sensors
  if (is_person_present(radar1_distance, 100.0f, radar1_last_distance, radar1_distances)) {
    radar1_triggered = true;
    digitalWrite(PIN_RELAY1, LOW);
  } else {
    digitalWrite(PIN_RELAY1, HIGH);
  }
  if (is_person_present(radar2_distance, 100.0f, radar2_last_distance, radar2_distances)) {
    radar2_triggered = true;
    digitalWrite(PIN_RELAY2, LOW);
  } else {
    digitalWrite(PIN_RELAY2, HIGH);
  }

  /////////////////////////////////// actions based on state
  const bool print_state = true; // for debug
  
  switch (state) {
    case CAN_TRIGGER:
      if (print_state) {
        Serial.print("CAN_TRIGGER ");
        Serial.println(user_keystrokes);
      }
      if (user_keystrokes > 6 && random(100) > 70) {
        if (piezo_reading > PIEZO_THRESHOLD) {
          state = CONSUME_TRIGGER;        
        }  
      }
      break;

    case CONSUME_TRIGGER:
      if (print_state) {
        Serial.println("CONSUME_TRIGGER");
      }
      switch (random(0,2)) {
        case 0:
          is_relay1_on = true;
          is_relay2_on = true;
          await_radar_trigger = 1;
        case 1: 
          is_tv_on = true;
          await_radar_trigger = 2;
      }
      state = TRIGGERED_AWAITING_RETURN;
      break;

    case TRIGGERED_AWAITING_RETURN:
      if (print_state) { 
        Serial.println("TRIGGERED_AWAITING_RETURN");
      }
      switch (await_radar_trigger) {
        case 1:
          if (radar1_triggered) {
            is_relay1_on = false;
            is_relay2_on = false;
            trigger_cooldown = 3000;
            state = TRIGGERED_COOLING_DOWN;
          }
          break;
        case 2:
          if (radar2_triggered) {
            is_tv_on = false;
            trigger_cooldown = 3000;
            state = TRIGGERED_COOLING_DOWN;
          }
          break;
        default:
          break;
      }
      break;

    case TRIGGERED_COOLING_DOWN:
      if (print_state) {
        Serial.print("TRIGGERED_COOLING_DOWN ");
        Serial.println(trigger_cooldown);
      }
      trigger_cooldown -= 10;
      if (trigger_cooldown > 300000) { // checking for underflow
        await_radar_trigger = 0;
        radar1_triggered = false;
        radar2_triggered = false;
        user_keystrokes = 0;
        state = CAN_TRIGGER;
      }
      break;
  }
  
  // send instruction to Processing
  Serial.println(is_tv_on ? "true" : "false");
}

// filter 
bool is_person_present(float dist, float threshold_dist, float current_last_distance, float *last_distances) {
    if (current_last_distance < LAST_DISTANCE_COUNT) {
        current_last_distance++;
        return dist < threshold_dist;
    }
    last_distances[LAST_DISTANCE_COUNT - 1] = dist;
    
    for (uint8_t i = 0; i < LAST_DISTANCE_COUNT - 1; i++) {
        last_distances[i] = last_distances[i + 1];
    }
    float dist_med = median(LAST_DISTANCE_COUNT, last_distances);
    return dist_med < threshold_dist;
}

// detect person typing, needs to be called as often as possible
void count_keystrokes() {
  piezo_reading = analogRead(PIN_PIEZO);
  if (piezo_reading > PIEZO_THRESHOLD) {
          user_keystrokes += 1;        
  }
}

// used to filter noise of ultrasonic sensor measurements
float median(int n, float x[]) {
    int temp;
    int i, j;
    // the following two loops sort the array x in ascending order
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(x[j] < x[i]) {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
        return((x[n/2] + x[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        return x[n/2];
    }
}
