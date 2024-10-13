#include<math.h>

#define CLK_PIN   12
#define LATCH_PIN 14
#define LED_PIN   2
#define DATA_PIN  13

#define LEFT_BUTTON_PIN   5
#define DOWN_BUTTON_PIN   19
#define RIGHT_BUTTON_PIN  18
#define UP_BUTTON_PIN     4
#define START_BUTTON_PIN  21
//#define START_BUTTON_PIN  23
#define A_BUTTON_PIN      26
#define B_BUTTON_PIN      25
#define X_BUTTON_PIN      27
#define Y_BUTTON_PIN      33

#define XPIN 32
#define YPIN 35
#define ZPIN 34

#define BUTTONS_BUFFER_SIZE 13

#define A_BUTTON_INDEX 0
#define B_BUTTON_INDEX 1
#define X_BUTTON_INDEX 2
#define Y_BUTTON_INDEX 3
#define UP_BUTTON_INDEX 4
#define DOWN_BUTTON_INDEX 5 
#define RIGHT_BUTTON_INDEX 6
#define LEFT_BUTTON_INDEX 7
#define START_BUTTON_INDEX 8
#define UP_ACCELEROMETER_INDEX 9
#define DOWN_ACCELEROMETER_INDEX 10
#define RIGHT_ACCELEROMETER_INDEX 11
#define LEFT_ACCELEROMETER_INDEX 12

#define VARY_DUTY_CYCLE true // If this is set to "false", the duty cycle will remain constant at 100%. If it's set to "true", the duty cycle will vary proportionally with the controller's tilt, ranging from 25% to 100%

const int ACC_DUTY_CYCLE_25[4] = {0,1,1,1};
const int ACC_DUTY_CYCLE_50[4] = {0,0,1,1};
const int ACC_DUTY_CYCLE_75[4] = {0,0,0,1};
const int ACC_DUTY_CYCLE_100[4] = {0,0,0,0};

/*
  Changing the upper and lower no-tilt limit values allows to configure the controller's sensitivity. The greater the interval, the lesser the controller's sensitivity.
  Therefore, if it's wanted to keep the accelerometer active in the firmware, the maximum upper value must be 21, and the lower value must be 15.
  However, setting the upper limit to 21 and the lower limit to 15 prevents the duty cycle from varying, even if the "VARY_DUTY_CYCLE" constant is set to true.
  Therefore, it is recommended that the maximum upper value be 20 and the minimum lower value be 16
*/
const int NO_TILT_UPPER_LIMIT = 19; // It can vary within a range of 18 to 20 (with 18 as the default)
const int NO_TILT_LOWER_LIMIT = 17; // It can vary within a range of 16 to 18 (with 18 as the default)

const int UP_TILT_VALUES[4] = {
  17, // Corresponds to the smaller tilt
  16,
  15,
  14 // Corresponds to the largest tilt
};
const int DOWN_TILT_VALUES[4] = {
  19, // Corresponds to the smaller tilt
  20,
  21,
  22 // Corresponds to the largest tilt
};
const int* RIGHT_TILT_VALUES = DOWN_TILT_VALUES;
const int* LEFT_TILT_VALUES = UP_TILT_VALUES;

int x_acc_duty_cycle_25_index = 0;
int x_acc_duty_cycle_50_index = 0;
int x_acc_duty_cycle_75_index = 0;
int x_acc_duty_cycle_100_index = 0;

int y_acc_duty_cycle_25_index = 0;
int y_acc_duty_cycle_50_index = 0;
int y_acc_duty_cycle_75_index = 0;
int y_acc_duty_cycle_100_index = 0;

bool clk_state = false; 
bool clk_state_high = false;

bool latch_state = false;
bool latch_state_high = false;

long time_since_boot = 0;

bool _data[BUTTONS_BUFFER_SIZE] = {false};
uint8_t data_sent_counter = 0;


void send_data() {
  if(data_sent_counter < BUTTONS_BUFFER_SIZE){
    digitalWrite(DATA_PIN, _data[data_sent_counter]);
    Serial.printf("Data sent data[%d] = %d\n", data_sent_counter, _data[data_sent_counter]);
    data_sent_counter++;
  }
}


void print_data_buffer() {
  for(int i = 0; i < BUTTONS_BUFFER_SIZE; i++){
    Serial.printf("%d ", _data[i]);
  }
  Serial.println();
}


void clk(){
  if (clk_state == HIGH && !clk_state_high) {
    // turn LED on:
//    digitalWrite(LED_PIN, HIGH);
//    Serial.println(esp_timer_get_t/ime() - time_since_boot);
//    time_since_boot = esp_timer_get_t/ime();
    clk_state_high = true;
    send_data();
  } else if (clk_state == LOW) {
    // turn LED off:
    // digitalWrite(LED_PIN, LOW);/
    clk_state_high = false;
  }
}


void latch(){
  if (latch_state == HIGH && !latch_state_high) {
    // turn LED on:
    digitalWrite(LED_PIN, HIGH);
//    Serial.println(esp_timer_get_time() - time_since_boot);/
    time_since_boot = esp_timer_get_time();
    latch_state_high = true;
    data_sent_counter = 0;
    read_buttons();
    define_acceleration();
    
    print_data_buffer();
  } else if (latch_state == LOW) {
    // turn LED off:
    digitalWrite(LED_PIN, LOW);
    latch_state_high = false;
  }

}


void read_buttons(){
  /*
   * data[0] = A
   * data[1] = B
   * data[2] = X
   * data[3] = Y
   * data[4] = UP
   * data[5] = DOWN
   * data[6] = RIGHT
   * data[7] = LEFT
   * data[8] = START
   */
  _data[A_BUTTON_INDEX] = digitalRead(A_BUTTON_PIN);
  _data[B_BUTTON_INDEX] = digitalRead(B_BUTTON_PIN);
  _data[X_BUTTON_INDEX] = digitalRead(X_BUTTON_PIN);
  _data[Y_BUTTON_INDEX] = digitalRead(Y_BUTTON_PIN);
  
  _data[UP_BUTTON_INDEX] = digitalRead(UP_BUTTON_PIN);
  _data[DOWN_BUTTON_INDEX] = digitalRead(DOWN_BUTTON_PIN);
  _data[RIGHT_BUTTON_INDEX] = digitalRead(RIGHT_BUTTON_PIN);
  _data[LEFT_BUTTON_INDEX] = digitalRead(LEFT_BUTTON_PIN);

  _data[START_BUTTON_INDEX] = digitalRead(START_BUTTON_PIN);

  // The bits from _data[9] to _data[12] won't receive data from the controller. Instead, their values will be defined by the code's internal logic
  
}


int set_acc_duty_cycle_index_value(int index) {
  index = index + 1;
  if (index > 3) return 0;
  return index;
}


void define_acceleration_duty_cycle(
  int tilt_value,
  const int values_corresponding_to_the_controllers_tilt[4],
  int button_index,
  int &acc_duty_cycle_25_index,
  int &acc_duty_cycle_50_index,
  int &acc_duty_cycle_75_index,
  int &acc_duty_cycle_100_index
) {
  if (
    (tilt_value > NO_TILT_UPPER_LIMIT ||
    tilt_value < NO_TILT_LOWER_LIMIT)  &&
    !VARY_DUTY_CYCLE
  ) {
    _data[button_index] = ACC_DUTY_CYCLE_100[acc_duty_cycle_100_index];
    acc_duty_cycle_100_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_100_index
    );
  } else if (tilt_value == values_corresponding_to_the_controllers_tilt[0]) {
    _data[button_index] = ACC_DUTY_CYCLE_25[acc_duty_cycle_25_index];
    acc_duty_cycle_25_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_25_index
    );
  } else if (tilt_value == values_corresponding_to_the_controllers_tilt[1]) {
    _data[button_index] = ACC_DUTY_CYCLE_75[acc_duty_cycle_50_index];
    acc_duty_cycle_50_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_50_index
    );
  } else if (tilt_value == values_corresponding_to_the_controllers_tilt[2]) {
    _data[button_index] = ACC_DUTY_CYCLE_50[acc_duty_cycle_75_index];
    acc_duty_cycle_75_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_75_index
    );
  } else if (tilt_value == values_corresponding_to_the_controllers_tilt[3]) {
    _data[button_index] = ACC_DUTY_CYCLE_100[acc_duty_cycle_100_index];
    acc_duty_cycle_100_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_100_index
    );
 }
}


void define_up_acceleration(int tilt_value) {
  define_acceleration_duty_cycle(
    tilt_value,
    UP_TILT_VALUES,
    UP_ACCELEROMETER_INDEX,
    y_acc_duty_cycle_25_index,
    y_acc_duty_cycle_50_index,
    y_acc_duty_cycle_75_index,
    y_acc_duty_cycle_100_index
  );
}


void define_down_acceleration(int tilt_value) {
  define_acceleration_duty_cycle(
    tilt_value,
    DOWN_TILT_VALUES,
    DOWN_ACCELEROMETER_INDEX,
    y_acc_duty_cycle_25_index,
    y_acc_duty_cycle_50_index,
    y_acc_duty_cycle_75_index,
    y_acc_duty_cycle_100_index
  );
}

void define_right_acceleration(int tilt_value) {
  define_acceleration_duty_cycle(
    tilt_value,
    RIGHT_TILT_VALUES,
    RIGHT_ACCELEROMETER_INDEX,
    x_acc_duty_cycle_25_index,
    x_acc_duty_cycle_50_index,
    x_acc_duty_cycle_75_index,
    x_acc_duty_cycle_100_index
  );
}


void define_left_acceleration(int tilt_value) {
  define_acceleration_duty_cycle(
    tilt_value,
    LEFT_TILT_VALUES,
    LEFT_ACCELEROMETER_INDEX,
    x_acc_duty_cycle_25_index,
    x_acc_duty_cycle_50_index,
    x_acc_duty_cycle_75_index,
    x_acc_duty_cycle_100_index
  );
}


void define_x_acceleration(int x_tilt_value) {
  if (x_tilt_value > NO_TILT_UPPER_LIMIT) {
    define_right_acceleration(x_tilt_value);
  } else if (x_tilt_value < NO_TILT_LOWER_LIMIT) {
    define_left_acceleration(x_tilt_value);
  } else {
    _data[RIGHT_ACCELEROMETER_INDEX] = 1;
    _data[LEFT_ACCELEROMETER_INDEX] = 1;
  }
}


void define_y_acceleration(int y_tilt_value) {
  if (y_tilt_value > NO_TILT_UPPER_LIMIT) {
    define_down_acceleration(y_tilt_value);
  } else if (y_tilt_value < NO_TILT_LOWER_LIMIT) {
    define_up_acceleration(y_tilt_value);
  } else {
    _data[DOWN_ACCELEROMETER_INDEX] = 1;
    _data[UP_ACCELEROMETER_INDEX] = 1;
  }
}


int round_tilt_value(int acceleration) {
  int rounded_acceleration = floor(acceleration / 100);
  return rounded_acceleration;
}


void define_acceleration() {
  int x_tilt_value = analogRead(XPIN);
  int y_tilt_value = analogRead(YPIN);

  x_tilt_value = round_tilt_value(
    x_tilt_value
  );
  y_tilt_value = round_tilt_value(
    y_tilt_value
  );

  define_x_acceleration(
    x_tilt_value
  );
  define_y_acceleration(
    y_tilt_value
  );

  delay(100);
}

void init_buttons(){
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(A_BUTTON_PIN, INPUT_PULLUP);
  pinMode(B_BUTTON_PIN, INPUT_PULLUP);
  pinMode(X_BUTTON_PIN, INPUT_PULLUP);
  pinMode(Y_BUTTON_PIN, INPUT_PULLUP);

  pinMode(XPIN, INPUT);
  pinMode(YPIN, INPUT);
  pinMode(ZPIN, INPUT);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLK_PIN, INPUT_PULLDOWN);
  pinMode(LATCH_PIN, INPUT_PULLDOWN);
  init_buttons();
  time_since_boot = esp_timer_get_time();
}

void loop() {
  // put your main code here, to run repeatedly:

  clk_state = digitalRead(CLK_PIN);
  latch_state = digitalRead(LATCH_PIN);

  clk();
  latch();
}
