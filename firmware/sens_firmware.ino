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

#define NUMBER_OF_REGULAR_BUTTONS 8 // Not counting the accelerometer directions
#define BUTTONS_AND_ACCELEROMETER_DIRECTIONS_BUFFER_SIZE 13
#define BUTTONS_BUFFER_SIZE 37

#define MAX_ACCELERATION_READ 2000
#define MAX_ACCELERATION_MAGNITUDE 255

#define CHANGE_ACCELERATION_MAGNITUDE false

bool clk_state = false; 
bool clk_state_high = false;

bool latch_state = false;
bool latch_state_high = false;

long time_since_boot = 0;

bool _data[BUTTONS_BUFFER_SIZE] = {false};
uint8_t data_sent_counter = 0;

int xv = analogRead(XPIN);
int yv = analogRead(YPIN);
int zv = analogRead(ZPIN);


void send_data() {
  int buttons_buffer_size = BUTTONS_AND_ACCELEROMETER_DIRECTIONS_BUFFER_SIZE;
  if (CHANGE_ACCELERATION_MAGNITUDE) buttons_buffer_size = BUTTONS_BUFFER_SIZE;

  if(data_sent_counter < buttons_buffer_size){
    digitalWrite(DATA_PIN, _data[data_sent_counter]);
    Serial.printf("Data sent data[%d] = %d\n", data_sent_counter, _data[data_sent_counter]);
    data_sent_counter++;
  }
}


void print_data_buffer() {
  int buttons_buffer_size = BUTTONS_AND_ACCELEROMETER_DIRECTIONS_BUFFER_SIZE;
  if (CHANGE_ACCELERATION_MAGNITUDE) buttons_buffer_size = BUTTONS_BUFFER_SIZE;

  for(int i = 0; i < buttons_buffer_size; i++){
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

  // The bits from _data[9] to _data[15] won't receive data from the controller. Instead, their values will be defined by the code's internal logic
  
}


void define_x_acceleration_direction(int x_acceleration_difference) {
  if (x_acceleration_difference > 0) { // If the "x_acceleration_difference" is positive, it indicates movement in the right direction; therefore, it is equivalent to pressing the RIGHT button
    _data[11] = 0; // Then, the _data[11] value is set to 0, indicating that the RIGHT button is pressed
  } else if (x_acceleration_difference < 0) { // If the "x_acceleration_difference" is negative, it indicates movement in the left direction. I'ts equivalent to pressing the LEFT button
    _data[12] = 0;// Then, the _data[12] value is set to 0, indicating that the LEFT button is pressed
  } else { // If the x_acceleration_difference is zero, it indicates no movement, either to the right or left. In this case, _data[11] and _data[12] need to be reset to 1. Otherwise, once movement to the right or left has occurred, they will remain set to 0 indefinitely
    _data[11] = 1;
    _data[12] = 1;
  }
}


void define_y_acceleration_direction(int y_acceleration_difference) {
  if (y_acceleration_difference > 0) { // If the "y_acceleration_difference" is positive, it indicates movement in the up direction; therefore, it is equivalent to pressing the UP button
    _data[9] = 0; // Then, the _data[9] value is set to 0, indicating that the UP button is pressed
  } else if (y_acceleration_difference < 0) { // If the "y_acceleration_difference" is negative, it indicates movement in the down direction. I'ts equivalent to pressing the DOWN button
    _data[10] = 0;// Then, the _data[10] value is set to 0, indicating that the DOWN button is pressed
  } else { // If the y_acceleration_difference is zero, it indicates no movement, either to the up or down. In this case, _data[9] and _data[10] need to be reset to 1. Otherwise, once movement to the up or down has occurred, they will remain set to 0 indefinitely
    _data[9] = 1;
    _data[10] = 1;
  }
}


int get_acceleration_magnitude(int acceleration_difference) {
  double acceleration_magnitude_double = (double)MAX_ACCELERATION_MAGNITUDE / (double)MAX_ACCELERATION_READ * acceleration_difference;
  int acceleration_magnitude_int = floor(acceleration_magnitude_double);
  return abs(acceleration_magnitude_int);
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


void define_acceleration_magnitudes(
  int x_acceleration_difference,
  int y_acceleration_difference,
  int z_acceleration_difference
) {
  /*
   * data[9] = ACCELEROMETER UP
   * data[10] = ACCELEROMETER DOWN
   * data[11] = ACCELEROMETER RIGHT
   * data[12] = ACCELEROMETER LEFT
   *
   * data[13] = X ACCELERATION MAGNITITUDE MSB
   * data[14] = X ACCELERATION MAGNITITUDE BIT
   * data[15] = X ACCELERATION MAGNITITUDE BIT
   * data[16] = X ACCELERATION MAGNITITUDE BIT
   * data[17] = X ACCELERATION MAGNITITUDE BIT
   * data[18] = X ACCELERATION MAGNITITUDE BIT
   * data[19] = X ACCELERATION MAGNITITUDE BIT
   * data[20] = X ACCELERATION MAGNITITUDE LSB
   *
   * data[21] = Y ACCELERATION MAGNITITUDE MSB
   * data[22] = Y ACCELERATION MAGNITITUDE BIT
   * data[23] = Y ACCELERATION MAGNITITUDE BIT
   * data[24] = Y ACCELERATION MAGNITITUDE BIT
   * data[25] = Y ACCELERATION MAGNITITUDE BIT
   * data[26] = Y ACCELERATION MAGNITITUDE BIT
   * data[27] = Y ACCELERATION MAGNITITUDE BIT
   * data[28] = Y ACCELERATION MAGNITITUDE LSB
   *
   * data[29] = Z ACCELERATION MAGNITITUDE MSB
   * data[30] = Z ACCELERATION MAGNITITUDE BIT
   * data[31] = Z ACCELERATION MAGNITITUDE BIT
   * data[32] = Z ACCELERATION MAGNITITUDE BIT
   * data[33] = Z ACCELERATION MAGNITITUDE BIT
   * data[34] = Z ACCELERATION MAGNITITUDE BIT
   * data[35] = Z ACCELERATION MAGNITITUDE BIT
   * data[36] = Z ACCELERATION MAGNITITUDE LSB
   */
  int x_acceleration_magnitude = get_acceleration_magnitude(
    x_acceleration_difference
  );
  int y_acceleration_magnitude = get_acceleration_magnitude(
    y_acceleration_difference
  );
  int z_acceleration_magnitude = get_acceleration_magnitude(
    z_acceleration_difference
  );

  // Fill the axis binary buffers with the binary representation of each axis acceleration magnitude
  const size_t axis_buffers_size = 8;  // Corresponds to the 8 bits used to store the binary representation of the x (as well as y and z) acceleration magnitude in the "_data" buffer
  int x_binary_buffer[axis_buffers_size]; // Buffer to hold the 0s and 1s that represent the binary representation of the x acceleration magnitude
  int y_binary_buffer[axis_buffers_size]; // Buffer to hold the 0s and 1s that represent the binary representation of the y acceleration magnitude
  int z_binary_buffer[axis_buffers_size]; // Buffer to hold the 0s and 1s that represent the binary representation of the z acceleration magnitude
  
  decimal_to_binary( // Once this function is executed, the "x_binary_buffer" is filled with the binary representation of the "x_acceleration_magnitude"
    x_acceleration_magnitude,
    x_binary_buffer,
    axis_buffers_size
  );
  decimal_to_binary( // Same thing for the y axis
    y_acceleration_magnitude,
    y_binary_buffer,
    axis_buffers_size
  );
  decimal_to_binary( // Same thing for the z axis
    z_acceleration_magnitude,
    z_binary_buffer,
    axis_buffers_size
  );

  // And the binary representation of the x acceleration magnitude can be written into the "_data" buffer (which stores the data sent to the driver) from the "x_binary_buffer"
  int i = 0;
  int _data_index = BUTTONS_AND_ACCELEROMETER_DIRECTIONS_BUFFER_SIZE; // Starts from index 13
  while (i < axis_buffers_size) { // In the "_data" buffer, the x acceleration magnitude bits ranges from the index 9 to 15 (8 steps)
    _data[_data_index] = x_binary_buffer[i];
    _data_index++;
    i++;
  }
  // Same thing for the "y_binary_buffer"
  i = 0;
  while (i < axis_buffers_size * 2) { // In the "_data" buffer, the y acceleration magnitude bits ranges from the index 17 to 24 (8 steps)
    _data[_data_index] = y_binary_buffer[i];
    _data_index++;
    i++;
  }
  // Same thing for the "z_binary_buffer"
  i = 0;
  while (i < axis_buffers_size * 3) { // In the "_data" buffer, the y acceleration magnitude bits ranges from the index 25 to 32 (8 steps)
    _data[_data_index] = z_binary_buffer[i];
    _data_index++;
    i++;
  }
}


void define_acceleration() {
  int current_x_acceleration = analogRead(XPIN);
  int current_y_acceleration = analogRead(YPIN);
  int current_z_acceleration = analogRead(ZPIN);

  int x_acceleration_difference = current_x_acceleration - xv;
  int y_acceleration_difference = current_y_acceleration - yv;
  int z_acceleration_difference = current_z_acceleration - zv;
  
  define_x_acceleration_direction(x_acceleration_difference);
  define_y_acceleration_direction(y_acceleration_difference);

  if (CHANGE_ACCELERATION_MAGNITUDE) {
    define_acceleration_magnitudes(
      x_acceleration_difference,
      y_acceleration_difference,
      z_acceleration_difference
    );
  }

  // Update the previous acceleration values at the end of each cycle
  xv = analogRead(XPIN);
  yv = analogRead(YPIN);
  zv = analogRead(ZPIN);

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
