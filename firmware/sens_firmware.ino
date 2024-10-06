

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


#include<math.h>
#define XPIN 32
#define YPIN 35
#define ZPIN 34

#define SIZE_BUTTONS 16

#define MAX_ACCELERATION_READ 5000
#define MAX_ACCELERATION_MAGNITUDE 255

bool clk_state = false; 
bool clk_state_high = false;

bool latch_state = false;
bool latch_state_high = false;

long time_since_boot = 0;

bool _data[SIZE_BUTTONS] = {false};
uint8_t data_sent_counter = 0;

int previous_x_accleration = analogRead(XPIN);
int yv = analogRead(YPIN);
int zv = analogRead(ZPIN);

void clk(){
  if (clk_state == HIGH && !clk_state_high) {
    // turn LED on:
//    digitalWrite(LED_PIN, HIGH);
//    Serial.println(esp_timer_get_t/ime() - time_since_boot);
//    time_since_boot = esp_timer_get_t/ime();
    clk_state_high = true;
    if(data_sent_counter < SIZE_BUTTONS){
      digitalWrite(DATA_PIN, _data[data_sent_counter]);
      Serial.printf("Data sent data[%d] = %d\n", data_sent_counter, _data[data_sent_counter]);
      data_sent_counter++;
    }
      
  } 
  else if(clk_state == LOW) {
    // turn LED off:
//    digitalWrite(LED_PIN, LOW);/
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

    for(int i = 0; i < SIZE_BUTTONS; i++){
      Serial.printf("%d ", _data[i]);
    }
    Serial.println();

    
  } 
  else if(latch_state == LOW) {
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

  // The bits from _data[9] to _data[15] won't receive data from the controller. Instead, their values will be defined by the code's internal logic
  
}


void define_acceleration_direction(int acceleration_difference) {
  if (acceleration_difference > 0) { // If the x acceleration value is positive, it indicates movement in the right direction; therefore, it is equivalent to pressing the RIGHT button
    _data[6] = 1; // Then, the _data[6] value is set to 1, indicating that the RIGHT button is pressed
  } else if (acceleration_difference < 0) { // If the x acceleration value is negative, it indicates movement in the left direction. I'ts equivalent to pressing the LEFT button
    _data[7] = 1;// Then, the _data[7] value is set to 1, indicating that the LEFT button is pressed
  }
}


int get_acceleration_magnitude(int acceleration_difference) {
  double x_acceleration_magnitude_double = (double)MAX_ACCELERATION_MAGNITUDE / (double)MAX_ACCELERATION_READ * acceleration_difference;
  int x_acceleration_magnitude_int = floor(x_acceleration_magnitude_double);
  return abs(x_acceleration_magnitude_int);
}


void decimal_to_binary(
  unsigned int decimal_number,
  int* buffer,
  size_t buffer_size
) {
  int index = buffer_size - 1;  // Start from the last position of the buffer
  // Fill the buffer with binary digits
  for (int i = 0; i < buffer_size; i++) {
    buffer[index--] = (decimal_number & 1);  // Extract the least significant bit and store it as an int (0 or 1)
    decimal_number >>= 1;  // Shift the number right to process the next bit
  }
}


void define_acceleration() {
  /*
   * data[9] = ACCELERATION MAGNITITUDE MSB
   * data[10] = ACCELERATION MAGNITITUDE BIT
   * data[11] = ACCELERATION MAGNITITUDE BIT
   * data[12] = ACCELERATION MAGNITITUDE BIT
   * data[13] = ACCELERATION MAGNITITUDE BIT
   * data[14] = ACCELERATION MAGNITITUDE BIT
   * data[15] = ACCELERATION MAGNITITUDE LSB
   */
  int current_x_acceleration = analogRead(XPIN);
  int x_acceleration_difference = current_x_acceleration - previous_x_accleration;
  
  define_acceleration_direction(x_acceleration_difference);

  int x_acceleration_magnitude = get_acceleration_magnitude(
    x_acceleration_difference
  );

  // Fill the "binary_buffer" with the binary representation of the x acceleration magnitude
  const size_t buffer_size = 8;  // Corresponds to the 8 bits used to store the binary representation of the acceleration magnitude in the "_data" buffer
  int binary_buffer[buffer_size];  // Buffer to hold the 0s and 1s that represent the binary representation of the acceleration magnitude
  
  decimal_to_binary( // Once this function is executed, the "binary_buffer" is filled with the binary representation of the "x_acceleration_magnitude"
    x_acceleration_magnitude,
    binary_buffer,
    buffer_size
  );

  // And the binary representation of the x acceleration magnitude can be written into the "_data" buffer (which stores the data sent to the driver) from the "binary_buffer"
  for (int i = 0; i < buffer_size; i++) {
    _data[i + buffer_size] = binary_buffer[i];
  }
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
