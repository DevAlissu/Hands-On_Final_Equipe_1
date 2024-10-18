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

#define BUTTONS_BUFFER_SIZE 9

#define A_BUTTON_INDEX 0
#define B_BUTTON_INDEX 1
#define X_BUTTON_INDEX 2
#define Y_BUTTON_INDEX 3
#define UP_BUTTON_INDEX 4
#define DOWN_BUTTON_INDEX 5 
#define RIGHT_BUTTON_INDEX 6
#define LEFT_BUTTON_INDEX 7
#define START_BUTTON_INDEX 8

#define VARY_DUTY_CYCLE true // If this is set to "false", the duty cycle will remain constant at 100%. If it's set to "true", the duty cycle will vary proportionally with the controller's tilt, ranging from 25% to 100%
#define TILT_INTERVAL_STEP 1

#define DUTY_CYCLE_VECTOR_SIZE 4

int* ACC_DUTY_CYCLE_25 = new int[DUTY_CYCLE_VECTOR_SIZE];
int* ACC_DUTY_CYCLE_50 = new int[DUTY_CYCLE_VECTOR_SIZE];
int* ACC_DUTY_CYCLE_75 = new int[DUTY_CYCLE_VECTOR_SIZE];
int* ACC_DUTY_CYCLE_100 = new int[DUTY_CYCLE_VECTOR_SIZE];

/*const int ACC_DUTY_CYCLE_25[4] = {0,1,1,1};
const int ACC_DUTY_CYCLE_50[4] = {0,0,1,1};
const int ACC_DUTY_CYCLE_75[4] = {0,0,0,1};
const int ACC_DUTY_CYCLE_100[4] = {0,0,0,0};*/

const int NO_TILT_UPPER_LIMIT = 19;
const int NO_TILT_LOWER_LIMIT = 17;

int UP_TILT_VALUES[4] = {0};
int DOWN_TILT_VALUES[4] = {0};
int RIGHT_TILT_VALUES[4] = {0};
int LEFT_TILT_VALUES[4] = {0};

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


void set_active_elements(
    int* duty_cycle_vector,
    int number_of_active_elements
) {
    for (int i = 0; i < number_of_active_elements; i++) {
      duty_cycle_vector[i] = 0;
    }
    for (int i = number_of_active_elements; i < DUTY_CYCLE_VECTOR_SIZE; i++) {
      duty_cycle_vector[i] = 1;
    }
}


void fill_duty_cycle_vector(
  int* duty_cycle_vector,
  int duty_cycle_percentage
) {

  switch (duty_cycle_percentage) {
    case 25: {
      int number_of_active_elements = DUTY_CYCLE_VECTOR_SIZE / 4; // 25% of the duty cycle vector's elements
      set_active_elements(
        duty_cycle_vector,
        number_of_active_elements
      );
      break;
    }
    case 50: {
      int number_of_active_elements = DUTY_CYCLE_VECTOR_SIZE / 2; // 50% of the duty cycle vector's elements
      set_active_elements(
        duty_cycle_vector,
        number_of_active_elements
      );
      break;
    }
    case 75: {
      int number_of_active_elements = DUTY_CYCLE_VECTOR_SIZE * 3 / 4; // 75% of the duty cycle vector's elements
      set_active_elements(
        duty_cycle_vector,
        number_of_active_elements
      );
      break;
    }
    case 100: {
      int number_of_active_elements = DUTY_CYCLE_VECTOR_SIZE; // 100% of the duty cycle vector's elements
      set_active_elements(
        duty_cycle_vector,
        number_of_active_elements
      );
      break;
    }
    default: {
        break;
    }
  }
}


void fill_duty_cycle_vectors() {
  fill_duty_cycle_vector(ACC_DUTY_CYCLE_25, 25);
  fill_duty_cycle_vector(ACC_DUTY_CYCLE_50, 50);
  fill_duty_cycle_vector(ACC_DUTY_CYCLE_75, 75);
  fill_duty_cycle_vector(ACC_DUTY_CYCLE_75, 100);
}


void define_tilt_values(
  int tilt_values_vector[4],
  int NO_TILT_UPPER_OR_LOWER_LIMIT,
  int _step,
  bool icreasing
) {
  int tilt_value = NO_TILT_UPPER_OR_LOWER_LIMIT;
  for (int i = 0; i < 4; i++) {
    tilt_values_vector[i] = tilt_value;
    if (icreasing) tilt_value += _step;
    else tilt_value -= _step;
  }
}


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


void print_int_vector(
  int* vector,
  int vector_size
) {
  for(int i = 0; i < vector_size; i++){
    printf("%d ", vector[i]);
  }
  printf("\n");
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


void define_tilt_values_for_each_direction() {
  define_tilt_values(
    UP_TILT_VALUES,
    NO_TILT_LOWER_LIMIT,
    TILT_INTERVAL_STEP,
    false
  );

  define_tilt_values(
    DOWN_TILT_VALUES,
    NO_TILT_UPPER_LIMIT,
    TILT_INTERVAL_STEP,
    true
  );

  define_tilt_values(
    RIGHT_TILT_VALUES,
    NO_TILT_UPPER_LIMIT,
    TILT_INTERVAL_STEP,
    true
  );

  define_tilt_values(
    LEFT_TILT_VALUES,
    NO_TILT_LOWER_LIMIT,
    TILT_INTERVAL_STEP,
    false
  );
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


/*void latch(){
  time_since_boot = esp_timer_get_time();
  data_sent_counter = 0;
  read_buttons();
  define_acceleration();
  
  print_data_buffer();
}*/


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


bool define_acceleration_according_to_the_tilt_interval(
  int tilt_value,
  const int values_corresponding_to_the_controllers_tilt[4],
  int upper_index,
  int lower_index
) {
  bool right_or_down_accelearation_condition = tilt_value >= values_corresponding_to_the_controllers_tilt[upper_index] && tilt_value < values_corresponding_to_the_controllers_tilt[lower_index];
  bool left_or_up_acceleration_condition = tilt_value <= values_corresponding_to_the_controllers_tilt[upper_index] && tilt_value > values_corresponding_to_the_controllers_tilt[lower_index];
  return right_or_down_accelearation_condition  || left_or_up_acceleration_condition;
}


void define_acceleration_duty_cycle(
  int tilt_value,
  int values_corresponding_to_the_controllers_tilt[4],
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
  } else if (define_acceleration_according_to_the_tilt_interval(
      tilt_value,
      values_corresponding_to_the_controllers_tilt,
      0,
      1
    )) {
    _data[button_index] = ACC_DUTY_CYCLE_25[acc_duty_cycle_25_index];
    acc_duty_cycle_25_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_25_index
    );
  } else if (define_acceleration_according_to_the_tilt_interval(
      tilt_value,
      values_corresponding_to_the_controllers_tilt,
      1,
      2
    )) {
    _data[button_index] = ACC_DUTY_CYCLE_75[acc_duty_cycle_50_index];
    acc_duty_cycle_50_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_50_index
    );
  } else if (
    define_acceleration_according_to_the_tilt_interval(
      tilt_value,
      values_corresponding_to_the_controllers_tilt,
      2,
      3
    )) {
    _data[button_index] = ACC_DUTY_CYCLE_50[acc_duty_cycle_75_index];
    acc_duty_cycle_75_index = set_acc_duty_cycle_index_value(
      acc_duty_cycle_75_index
    );
  } else if (
    tilt_value >= values_corresponding_to_the_controllers_tilt[3] ||
    tilt_value <= values_corresponding_to_the_controllers_tilt[3]
    ) {
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
    UP_BUTTON_INDEX,
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
    DOWN_BUTTON_INDEX,
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
    RIGHT_BUTTON_INDEX,
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
    LEFT_BUTTON_INDEX,
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
    if (digitalRead(RIGHT_BUTTON_INDEX)) {
      _data[RIGHT_BUTTON_INDEX] = 1;
    }
    if (digitalRead(LEFT_BUTTON_PIN)) {
      _data[LEFT_BUTTON_INDEX] = 1;
    }
  }
}


void define_y_acceleration(int y_tilt_value) {
  if (y_tilt_value > NO_TILT_UPPER_LIMIT) {
    define_down_acceleration(y_tilt_value);
  } else if (y_tilt_value < NO_TILT_LOWER_LIMIT) {
    define_up_acceleration(y_tilt_value);
  } else {
    if (digitalRead(DOWN_BUTTON_INDEX)) {
      _data[DOWN_BUTTON_INDEX] = 1;
    }
    if (digitalRead(UP_BUTTON_INDEX)) {
      _data[UP_BUTTON_INDEX] = 1;
    }
  }
}


int round_tilt_value(int acceleration) {
  int rounded_acceleration = floor(acceleration / 100);
  return rounded_acceleration;
}


/*int round_tilt_value(int acceleration) {
  int rounded_acceleration = floor(acceleration / 10);
  Serial.println(rounded_acceleration );
  return rounded_acceleration;
}*/


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

  fill_duty_cycle_vectors();
  define_tilt_values_for_each_direction();
}

void loop() {
  // put your main code here, to run repeatedly:

  clk_state = digitalRead(CLK_PIN);
  latch_state = digitalRead(LATCH_PIN);

  clk();
  latch();
}