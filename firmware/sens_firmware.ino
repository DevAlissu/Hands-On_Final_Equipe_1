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

const int ACC_DUTY_CYCLE_25[4] = {0,1,1,1};
const int ACC_DUTY_CYCLE_50[4] = {0,0,1,1};
const int ACC_DUTY_CYCLE_75[4] = {0,0,0,1};
const int ACC_DUTY_CYCLE_100[4] = {0,0,0,0};

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
  _data[0] = digitalRead(A_BUTTON_PIN);
  _data[1] = digitalRead(B_BUTTON_PIN);
  _data[2] = digitalRead(X_BUTTON_PIN);
  _data[3] = digitalRead(Y_BUTTON_PIN);
  
  _data[4] = digitalRead(UP_BUTTON_PIN);
  _data[5] = digitalRead(DOWN_BUTTON_PIN);
  _data[6] = digitalRead(RIGHT_BUTTON_PIN);
  _data[7] = digitalRead(LEFT_BUTTON_PIN);

  _data[8] = digitalRead(START_BUTTON_PIN);

  // The bits from _data[9] to _data[12] won't receive data from the controller. Instead, their values will be defined by the code's internal logic
  
}


int set_acc_duty_cycle_index_value(int index) {
  index = index + 1;
  if (index > 3) return 0;
  return index;
}


void define_acceleration_duty_cycle(
  int acceleration_value,
  const int values_corresponding_to_the_controllers_tilt[4],
  int button_index, // 9 for UP, 10 for DOWN, 11 for RIGHT and 12 FOR LEFT
  int &acc_duty_cycle_25_index,
  int &acc_duty_cycle_50_index,
  int &acc_duty_cycle_75_index,
  int &acc_duty_cycle_100_index
) {
  if (acceleration_value == values_corresponding_to_the_controllers_tilt[0]) {
    _data[button_index] = ACC_DUTY_CYCLE_25[acc_duty_cycle_25_index];
    acc_duty_cycle_25_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_25_index
    );
  } else if (acceleration_value == values_corresponding_to_the_controllers_tilt[1]) {
    _data[button_index] = ACC_DUTY_CYCLE_75[acc_duty_cycle_50_index];
    acc_duty_cycle_50_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_50_index
    );
  } else if (acceleration_value == values_corresponding_to_the_controllers_tilt[2]) {
    _data[button_index] = ACC_DUTY_CYCLE_50[acc_duty_cycle_75_index];
    acc_duty_cycle_75_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_75_index
    );
  } else if (acceleration_value == values_corresponding_to_the_controllers_tilt[3]) {
    _data[button_index] = ACC_DUTY_CYCLE_100[acc_duty_cycle_100_index];
    acc_duty_cycle_100_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_100_index
    );
  } else {
    _data[button_index] = 1;
  }
}


void define_up_acceleration(int acceleration_value) {
  int up_accelerometer_index = 9;
  define_acceleration_duty_cycle(
    acceleration_value,
    UP_TILT_VALUES,
    up_accelerometer_index,
    y_acc_duty_cycle_25_index,
    y_acc_duty_cycle_50_index,
    y_acc_duty_cycle_75_index,
    y_acc_duty_cycle_100_index
  );
}


void define_down_acceleration(int acceleration_value) {
  int down_accelerometer_index = 10;
  define_acceleration_duty_cycle(
    acceleration_value,
    DOWN_TILT_VALUES,
    down_accelerometer_index,
    y_acc_duty_cycle_25_index,
    y_acc_duty_cycle_50_index,
    y_acc_duty_cycle_75_index,
    y_acc_duty_cycle_100_index
  );
}

void define_right_acceleration(int acceleration_value) {
  int right_accelerometer_index = 11;
  define_acceleration_duty_cycle(
    acceleration_value,
    RIGHT_TILT_VALUES,
    right_accelerometer_index,
    x_acc_duty_cycle_25_index,
    x_acc_duty_cycle_50_index,
    x_acc_duty_cycle_75_index,
    x_acc_duty_cycle_100_index
  );
}


void define_left_acceleration(int acceleration_value) {
  int left_accelerometer_index = 12;
  define_acceleration_duty_cycle(
    acceleration_value,
    LEFT_TILT_VALUES,
    left_accelerometer_index,
    x_acc_duty_cycle_25_index,
    x_acc_duty_cycle_50_index,
    x_acc_duty_cycle_75_index,
    x_acc_duty_cycle_100_index
  );
}


void define_x_acceleration(int x_tilt_value) {
  define_right_acceleration(x_tilt_value);
  define_left_acceleration(x_tilt_value);
}


void define_y_acceleration(int y_tilt_value) {
  define_up_acceleration(y_tilt_value);
  define_down_acceleration(y_tilt_value);
}


int round_acceleration(int acceleration) {
  int rounded_acceleration = floor(acceleration / 100);
  return rounded_acceleration;
}


void define_acceleration() {
  int x_tilt_value = analogRead(XPIN);
  int y_tilt_value = analogRead(YPIN);

  x_tilt_value = round_acceleration(
    x_tilt_value
  );
  y_tilt_value = round_acceleration(
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
