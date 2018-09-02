#include <PietteTech_DHT.h>

#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A0           	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   2000  // Sample every two seconds

#define RELAY1 D3
#define RELAY2 D4
#define RELAY3 D5
#define RELAY4 D6

double humidity,temp,dewpoint=0;
bool fanIsRunning=false;

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

  Particle.function("relay", relayControl);
  Particle.variable("temp", temp);
  Particle.variable("humidity", humidity);
  Particle.variable("dewpoint", dewpoint);
  //Serial.begin(9600);
}

void loop()
{
  measure();
  if(humidity >= 65 && !fanIsRunning)
    turnOnFan();
  if(humidity < 65 && fanIsRunning)
    turnOffFan();
  /*Serial.print("Humidity (%): ");
  Serial.println(humidity, 2);

  Serial.print("Temperature (oC): ");
  Serial.println(temp, 2);

  Serial.print("Dew Point Slow (oC): ");
  Serial.println(dewpoint);*/
  delay(2500);
}

// command format r1,HIGH
int relayControl(String command)
{
  int relayState = 0;
  // parse the relay number
  int relayNumber = command.charAt(1) - '0';
  // do a sanity check
  if (relayNumber < 1 || relayNumber > 4) return -1;

  // find out the state of the relay
  if (command.substring(3,7) == "HIGH") relayState = 1;
  else if (command.substring(3,6) == "LOW") relayState = 0;
  else return -1;

  // write to the appropriate relay
  digitalWrite(relayNumber+2, relayState);
  return 1;
}

void turnOnFan() {
  digitalWrite(RELAY1,HIGH);
  fanIsRunning = true;
}

void turnOffFan() {
  digitalWrite(RELAY1,LOW);
  fanIsRunning = false;
}

void measure() {
  int result = DHT.acquireAndWait(2000); // wait up to 2 sec (default indefinitely)

  if(result==DHTLIB_OK) {
    humidity = DHT.getHumidity();
    temp = DHT.getCelsius();
    dewpoint = DHT.getDewPointSlow();
  } else {
    switch (result) {
      case DHTLIB_ERROR_CHECKSUM:
        Serial.println("Error\n\r\tChecksum error");
        break;
      case DHTLIB_ERROR_ISR_TIMEOUT:
        Serial.println("Error\n\r\tISR time out error");
        break;
      case DHTLIB_ERROR_RESPONSE_TIMEOUT:
        Serial.println("Error\n\r\tResponse time out error");
        break;
      case DHTLIB_ERROR_DATA_TIMEOUT:
        Serial.println("Error\n\r\tData time out error");
        break;
      case DHTLIB_ERROR_ACQUIRING:
        Serial.println("Error\n\r\tAcquiring");
        break;
      case DHTLIB_ERROR_DELTA:
        Serial.println("Error\n\r\tDelta time to small");
        break;
      case DHTLIB_ERROR_NOTSTARTED:
        Serial.println("Error\n\r\tNot started");
        break;
      default:
        Serial.println("Unknown error");
    }
  }
}
