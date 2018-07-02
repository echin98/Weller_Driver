#define TEMP_PIN 1
#define OUT_PIN 2
#define COLD_JUNCT_PIN 3
#define MAX_TEMP 800

float temp;
float setPoint;
float lastError = 0;
float errorInt = 0;
float P = 1;
float I = 0;
float D = 0;
unsigned short controlPeriod = 4;
unsigned long lastTime = 0;
unsigned short settleTime = 1;
boolean verb = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(OUT_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(COLD_JUNCT_PIN, INPUT);
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long t = millis();
  if(lastTime>t)
    lastTime = t;
  if(lastTime<t+controlPeriod){
    getTemp();
    analogWrite(OUT_PIN,getDutyCycle());
    if(verb)
      putTemp();
      Serial.print("Set Point: ");
      Serial.println(setPoint);
  }
  
}

unsigned short getDutyCycle(){
  float error = setPoint-temp;
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
  temp = voltage*235+105;
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
