
const int PIN_PIEZO = A0,
PIN_RELAY1 = 7,
PIN_RELAY2 = 8,
PIN_BUZZER_TEST = 13,
PIN_RADAR1_ECHO = A1,
PIN_RADAR1_TRIG = A2,
PIN_RADAR2_ECHO = A3,
PIN_RADAR2_TRIG = A4;

// piezo
int piezo_reading = 0;
const int PIEZO_THRESHOLD = 1.5;

// ultrasonic sensors
const unsigned int TIMER_MEASURE_RADAR = 0
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

// lights
boolean is_relay1_on = false
  , is_relay2_on = false;

///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PIEZO, INPUT); // INPUT_PULLUP?
  
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

  // measure radar
  if (t_now - timers.RADAR1_MEASURE >= TIMER_MEASURE_RADAR) {
        timers.RADAR1_MEASURE = t_now;
        digitalWrite(PIN_RADAR1_TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(PIN_RADAR1_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_RADAR1_TRIG, LOW);
        //delayMicroseconds(2);
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
  Serial.print(radar1_distance);
  Serial.print(" ");
  Serial.print(radar2_distance);

  // piezo sensor
  piezo_reading = analogRead(PIN_PIEZO);
  if (piezo_reading > PIEZO_THRESHOLD) {
     Serial.print(" true");
  } else {
    Serial.print(" false");
  }

  // distance sensor
  if (is_person_present(radar1_distance, 100.0f, radar1_last_distance, radar1_distances)) {
    digitalWrite(PIN_RELAY1, LOW);
    Serial.print(" true");
  } else {
    digitalWrite(PIN_RELAY1, HIGH);
    Serial.print(" false");
  }
  if (is_person_present(radar2_distance, 100.0f, radar2_last_distance, radar2_distances)) {
    digitalWrite(PIN_RELAY2, LOW);
    Serial.print(" true");
  } else {
    digitalWrite(PIN_RELAY2, HIGH);
    Serial.print(" false");
  }
  Serial.print("\n");
}

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
