

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

#define SIZE_BUTTONS 9

bool clk_state = false; 
bool clk_state_high = false;

bool latch_state = false;
bool latch_state_high = false;

long time_since_boot = 0;

bool _data[SIZE_BUTTONS] = {false};
uint8_t data_sent_counter = 0;

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
