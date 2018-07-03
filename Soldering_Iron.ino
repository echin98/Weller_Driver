#define TEMP_PIN 6
#define OUT_PIN 3
#define COLD_JUNCT_PIN 3
#define MAX_TEMP 180
#define DATALOG_PERIOD 500
#define NEG_SUPPLY_PIN 5

float temp;
float setPoint = 0;
float lastError = 0;
float errorInt = 0;
float P = 10;
float I = 0;
float D = 0;
float error = 0;
unsigned short controlPeriod = 50;
unsigned long lastTime = 0;
unsigned long lastDataLog = 0;
unsigned short settleTime = 5;
boolean verb = false;
boolean disabled = false;
boolean dataLog = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(OUT_PIN, OUTPUT);
  pinMode(NEG_SUPPLY_PIN,OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(COLD_JUNCT_PIN, INPUT);
  Serial.begin(9600);

  analogWrite(NEG_SUPPLY_PIN, 125);
}

void loop() {
  analogWrite(NEG_SUPPLY_PIN, 100);
  // put your main code here, to run repeatedly:
  unsigned long t = millis();
  if(lastTime<t-controlPeriod){
    getTemp();

    if(temp>MAX_TEMP)
      disabled = true;
    unsigned short dutyCycle = getDutyCycle();
    if(!disabled){
        analogWrite(OUT_PIN,dutyCycle);
    }
    else{
      analogWrite(OUT_PIN,0);
    }
    if(verb){
          putTemp();
          Serial.print("Set Point: ");
          Serial.print(setPoint);
          Serial.print(", Error: ");
          Serial.print(error);
          Serial.print(", Duty Cycle: ");
          Serial.println(dutyCycle);
          Serial.println(disabled?"Disabled, press 'x' to re-enable.\n":"");
     }
     else if(dataLog){
      if(t>=lastDataLog+DATALOG_PERIOD){
        Serial.println(temp);
        lastDataLog = t;
      }
     }
    lastTime = t;
  }
  parseInput();
  
}

void parseInput(){
  while(Serial.available()) { // there is keyboard input so process it
    int serialByte = Serial.read();
    if (isWhitespace(serialByte) || serialByte == 10) {
      continue;
    }
    else if (serialByte == 'v') {
      Serial.println(verb?"Disabling verbose mode":"Enabling verbose mode");
      verb = !verb;
    }
    else if (serialByte == 'q'){
      setPoint = 0;
    }
    else if(serialByte == 'd'){
      dataLog = !dataLog;
      Serial.println(dataLog?"Data logging enabled":"Data logging disabled");
    }
    else if (serialByte == 'x'){
      disabled = !disabled;
    }
    else if (serialByte == 's'){
      setPoint = Serial.parseFloat();
      Serial.print("Set new setpoint to ");
      Serial.println(setPoint);
    }
    else {
      Serial.print("Unexpected serial input ");
      Serial.println(serialByte);
    }
  }
}

unsigned short getDutyCycleSimple(){
  error = setPoint-temp;
  if(error<0)
    return(0);
  else
    return(100);
}

unsigned short getDutyCycle(){
  error = setPoint-temp;
  errorInt = (controlPeriod*error)+errorInt;
  float dutyCycle = error*P+I*errorInt +D*(error-lastError)/controlPeriod;
  lastError = error;
  if(dutyCycle<0)
    dutyCycle = 0;
  if(dutyCycle>255)
    dutyCycle = 255;
  return (unsigned short)dutyCycle;
  serialProcessor();
}

void putTemp(){
  Serial.print("Current Temp: ");
  Serial.println(temp);
}

void getTemp(){
  analogWrite(OUT_PIN, 0);
  delay(settleTime);
  int raw = analogRead(TEMP_PIN);
  double voltage = raw*5.0/1023.0;
  //temp = voltage*235+105;
  temp = 60.3*pow(voltage,5)-505.9*pow(voltage,4)+1550.8*pow(voltage,3)-2064.5*pow(voltage,2)+1276.9*voltage+68;
}

boolean serialProcessor() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    int newVal = 0;
    int val = 0;

    switch (incomingByte) {
      case 's':
        val = Serial.parseFloat();
        setPoint = val < MAX_TEMP && val >= 0 ? val : setPoint;
        break;
      case 'x':
        verb = !verb;
        break;
      default:
        Serial.println("Invalid Input!");
        break;
    }
    return true;
  }
  return false;
}
