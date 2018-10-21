#include <PietteTech_DHT.h>

#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A0           	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   3000  // Sample every 3 seconds

#define RELAY1 D3
#define RELAY2 D4
#define RELAY3 D5
#define RELAY4 D6

SYSTEM_MODE(SEMI_AUTOMATIC);

double humidity,temp,dewpoint = 0;
String fanStatus = "OFF";
bool manualRun = false;
int humidityTreshold = 65;

PietteTech_DHT DHT(DHTPIN, DHTTYPE);

void setup()
{
  //DHT22 requires pullup
  pinMode(DHTPIN, INPUT_PULLUP);

  //Initilize the relay control pins as output
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
  //Serial.begin(9600);
}

void loop()
{
  measure();
  if(humidity >= humidityTreshold && (fanStatus == "OFF"))
    turnOnFan();
  if(humidity < humidityTreshold && (fanStatus == "ON") && !manualRun)
    turnOffFan();
  /*Serial.print("Humidity (%): ");
  Serial.println(humidity, 2);

  Serial.print("Temperature (oC): ");
  Serial.println(temp, 2);

  Serial.print("Dew Point Slow (oC): ");
  Serial.println(dewpoint);*/
  connect();
  delay(DHT_SAMPLE_INTERVAL);
}

int fancontrol(String command) {
  if ((fanStatus == "ON") && command == "OFF") {
    manualRun = false;
    turnOffFan();
  }
  if ((fanStatus == "OFF") && command == "ON") {
    manualRun = true;
    turnOnFan();
  }
  return 1;
}

void publishEvent(String eventName, String data) {
  connect();
  Particle.publish(eventName,data);
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
      humidity = DHT.getHumidity();
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
