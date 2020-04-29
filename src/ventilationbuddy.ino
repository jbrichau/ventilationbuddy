#include <PietteTech_DHT.h>

#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A0           	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL 30000   // Sample every 30 seconds
#define SAMPLE_SIZE 100
#define TRESHOLD_INTERVAL   5
#define INCREASE_TRESHOLD 10

#define RELAY1 D3
#define RELAY2 D4
#define RELAY3 D5
#define RELAY4 D6

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

// WARNING: will not work after device os 1.1.1:
// https://github.com/particle-iot/device-os/issues/1835

double humidity,temp,dewpoint = 0;
double humidity_measurements[SAMPLE_SIZE];
double humidity_cutoff;
unsigned long time_cutoff;
String fanStatus = "OFF";
bool manualOverride = false;

PietteTech_DHT DHT(DHTPIN, DHTTYPE);

void setup()
{
  //DHT22 requires pullup
  pinMode(DHTPIN, INPUT_PULLUP);

  //Initialize the relay control pins as output
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  // Initialize all relays to an OFF state
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);

  Particle.function("fan", fancontrol);
  Particle.variable("temp", temp);
  Particle.variable("humidity", humidity);
  Particle.variable("dewpoint", dewpoint);
  Particle.variable("fanStatus", fanStatus);
  Particle.variable("override", manualOverride);
  DHT.begin();
  measure();
  for(int i=0; i<SAMPLE_SIZE; i++)
    humidity_measurements[i] = humidity_measurements[SAMPLE_SIZE];
  time_cutoff = Time.now();
}

void loop()
{
  measure();
  if(!manualOverride) {
    if((fanStatus == "OFF") && (humidityIncrease() > INCREASE_TRESHOLD)) {
      humidity_cutoff = humidity_measurements[SAMPLE_SIZE - TRESHOLD_INTERVAL];
      time_cutoff = Time.now() + 7200;
      turnOnFan();
    }
    if((fanStatus == "ON") && (humidity_measurements[SAMPLE_SIZE] <= humidity_cutoff || time_cutoff <= Time.now()))
      turnOffFan();
  }
  delay(DHT_SAMPLE_INTERVAL);
}

int humidityIncrease() {
  return (int)(humidity_measurements[SAMPLE_SIZE] - humidity_measurements[SAMPLE_SIZE - TRESHOLD_INTERVAL]);
}

int fancontrol(String command) {
  if ((fanStatus == "ON") && command == "OFF") {
    turnOffFan();
    manualOverride = true;
  }
  if ((fanStatus == "OFF") && command == "ON") {
    turnOnFan();
    manualOverride = true;
  }
  if (command == "AUTO") {
    manualOverride = false;
  }
  return 1;
}

void publishEvent(String eventName, String data) {
  //connect();
  Particle.publish(eventName,data,PRIVATE);
}

void connect() {
  if (Particle.connected() == false) {
    Particle.connect();
  }
}

void turnOnFan() {
  digitalWrite(RELAY1,HIGH);
  fanStatus = "ON";
  publishEvent("fan","on");
}

void turnOffFan() {
  digitalWrite(RELAY1,LOW);
  fanStatus = "OFF";
  publishEvent("fan","off");
}

void measure() {
  int result = DHT.acquireAndWait(2000);
  switch (result) {
    case DHTLIB_OK:
      for(int i=SAMPLE_SIZE; i>0 ; i--)
        humidity_measurements[i-1] = humidity_measurements[i];
      humidity = DHT.getHumidity();
      humidity_measurements[SAMPLE_SIZE] = humidity;
      temp = DHT.getCelsius();
      dewpoint = DHT.getDewPointSlow();
      break;
    case DHTLIB_ERROR_CHECKSUM:
      publishEvent("error","DHT read: Checksum error");
      //Serial.println("Error\n\r\tChecksum error");
      break;
    case DHTLIB_ERROR_ISR_TIMEOUT:
      publishEvent("error","DHT read: ISR time out error");
      //Serial.println("Error\n\r\tISR time out error");
      break;
    case DHTLIB_ERROR_RESPONSE_TIMEOUT:
      publishEvent("error","DHT read: Response time out error");
      //Serial.println("Error\n\r\tResponse time out error");
      break;
    case DHTLIB_ERROR_DATA_TIMEOUT:
      publishEvent("error","DHT read: Data time out error");
      //Serial.println("Error\n\r\tData time out error");
      break;
    case DHTLIB_ERROR_ACQUIRING:
      publishEvent("error","DHT read: Acquiring");
      //Serial.println("Error\n\r\tAcquiring");
      break;
    case DHTLIB_ERROR_DELTA:
      publishEvent("error","DHT read: Delta time to small");
      //Serial.println("Error\n\r\tDelta time too small");
      break;
    case DHTLIB_ERROR_NOTSTARTED:
      publishEvent("error","DHT read: Not started");
      //Serial.println("Error\n\r\tNot started");
      break;
    default:
      publishEvent("error","DHT read: Unknown error");
      //Serial.println("Unknown error");
  }
}
