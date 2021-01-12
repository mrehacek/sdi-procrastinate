
const int PIN_PIEZO = A0,
PIN_RELAY1 = 7,
PIN_RELAY2 = 8,
PIN_BUZZER_TEST = 13,
PIN_RADAR1_ECHO = A1,
PIN_RADAR1_TRIG = A2;

// piezo
int piezo_reading = 0;
const int PIEZO_THRESHOLD = 1.5;

// ultrasonic sensor
const unsigned int TIMER_MEASURE_RADAR = 30
  , LAST_DISTANCE_COUNT = 8;
float 
  radar_time = 0, 
  radar_distance = 0;
int current_last_distance = 0;
float last_distances[LAST_DISTANCE_COUNT] = { 0 };

struct timers
{
  unsigned long int RADAR_MEASURE;
} timers = { 0 };

///////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(PIN_PIEZO, INPUT); // INPUT_PULLUP?
  
  pinMode(PIN_RADAR1_ECHO, INPUT);
  pinMode(PIN_RADAR1_TRIG, OUTPUT);
  
  pinMode(PIN_RELAY1, OUTPUT);
  pinMode(PIN_RELAY2, OUTPUT);
  pinMode(PIN_BUZZER_TEST, OUTPUT);
}

void loop() {
  unsigned long t_now = millis();

  //digitalWrite(PIN_RELAY1, LOW);
  //digitalWrite(PIN_RELAY2, LOW);
  //digitalWrite(PIN_BUZZER_TEST, LOW);

  // measure radar
  if (t_now - timers.RADAR_MEASURE >= TIMER_MEASURE_RADAR) {
        digitalWrite(PIN_RADAR1_TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(PIN_RADAR1_TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_RADAR1_TRIG, LOW);
        //delayMicroseconds(2);
        radar_time = pulseIn(PIN_RADAR1_ECHO, HIGH);
        // radar_distance = radar_time * 340 / 20000;

        float new_radar_dist = (radar_time/2) / 29.1;
        
        // ignore overblown values, max distance measured is 4 meters
        if (new_radar_dist < 400) {
          radar_distance = new_radar_dist;
        }
  }

  // read piezo sensor
  piezo_reading = analogRead(PIN_PIEZO);
  if (piezo_reading > PIEZO_THRESHOLD) {
    //Serial.print("piezo " + piezo_reading);
  }

  // Serial.print("radar_distance ");
  // Serial.print(radar_distance);

  if (is_person_present(radar_distance, 300.0f)) {
    digitalWrite(PIN_RELAY1, LOW);
    digitalWrite(PIN_RELAY2, LOW);
    Serial.print("PERSON_PRESENT ");
  } else {
    digitalWrite(PIN_RELAY1, HIGH);
    digitalWrite(PIN_RELAY2, HIGH);
    Serial.print("NO_PERSON ");
  }
  
  //sensorValue1 = analogRead(analogInPin);
  //sensorValue2 = map(sensorValue1, 0, 1023, 0, 255);

  //Serial.print(sensorValue1);
  //Serial.print(" ");
  Serial.print("\n");
  
  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(30);
}

bool is_person_present(float dist, float threshold_dist) {
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
