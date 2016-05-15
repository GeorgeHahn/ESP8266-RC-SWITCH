#include <ESP.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

const char *ssid =  "WIFI SSID";       // cannot be longer than 32 characters!
const char *pass =  "WIFI PASSWORD";   //

// Currently ignored (these will be auto-detected by mDNS)
IPAddress server( 192, 168, 1, 15 );
int mqttport = 1883;

const char *MQTT_USERNAME = "";
const char *MQTT_PASSWORD = "";
char topic[26];
char clientname[26];
char data[50];
char uid[7];

#define PIN_DATA D1
#define PIN_STATUS_SENDING D4
#define MQTT_TOPIC "rfsend"

WiFiClient wclient;
PubSubClient client(wclient);

// Long:  640us
// Short: 140us
// Sync: 5980us

inline void longdelay()
{
  delayMicroseconds(640);
}

inline void shortdelay()
{
  delayMicroseconds(140);
}

inline void sendsync() {
  digitalWrite(PIN_DATA, HIGH);
  shortdelay();
  digitalWrite(PIN_DATA, LOW);
  delayMicroseconds(5980);
}

void writePacket(byte* payload, unsigned int length) { 
  for(byte i = 0; i < length; i++)
  {
    if(payload[i] == '0')
    {
      digitalWrite(PIN_DATA, HIGH);
      shortdelay();
      digitalWrite(PIN_DATA, LOW);
      longdelay();
      digitalWrite(PIN_DATA, HIGH);
      shortdelay();
      digitalWrite(PIN_DATA, LOW);
      longdelay();
    }
    else if(payload[i] == '1')
    {      
      digitalWrite(PIN_DATA, HIGH);
      longdelay();
      digitalWrite(PIN_DATA, LOW);
      shortdelay();
      digitalWrite(PIN_DATA, HIGH);
      longdelay();
      digitalWrite(PIN_DATA, LOW);
      shortdelay();
    }
    else if(payload[i] == 'F')
    {
      digitalWrite(PIN_DATA, HIGH);
      shortdelay();
      digitalWrite(PIN_DATA, LOW);
      longdelay();
      digitalWrite(PIN_DATA, HIGH);
      longdelay();
      digitalWrite(PIN_DATA, LOW);
      shortdelay();
    }
  }
  
  // Send sync
  sendsync();
}

void callback(char* inTopic, byte* payload, unsigned int length){
  Serial.println("Got RF data");
  Serial.write(payload, length);
  Serial.println();
  
  digitalWrite(PIN_STATUS_SENDING, LOW);

  // Resend six times to make sure the command was received
  for(int i = 0; i < 6; i++) {
    writePacket(payload, length);
    delayMicroseconds(100);
  }
  digitalWrite(PIN_STATUS_SENDING, HIGH);
  
  Serial.println("RF data sent");
}

void CreateUid()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(uid, 7, "%02X%02X%02X", mac[3], mac[4], mac[5]);
  //sprintf(uid, "ESP_%06X", ESP.getChipId());
}

void setup() {
  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  pinMode(PIN_DATA, OUTPUT);
  pinMode(PIN_STATUS_SENDING, OUTPUT);
  digitalWrite(PIN_STATUS_SENDING, HIGH);
  digitalWrite(PIN_DATA, LOW);
  
  CreateUid();

  Serial.print("Hostname: ");
  Serial.println(uid);
  WiFi.hostname(uid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

 if (!MDNS.begin(uid)) {
    Serial.println(F("Error setting up MDNS responder!"));
  }
  Serial.println("mDNS responder started");
  //MDNS.addService("mqtt", "tcp", 1883);

  int n = 0;
  Serial.println("Sending mDNS query");
  while(n == 0) 
  {
    n = MDNS.queryService("mqtt", "tcp"); // Send out query for esp tcp services
    if(n > 0)
    {
      Serial.print(n);
      Serial.println(F(" service(s) found"));
      
      // Default to first service
      server = MDNS.IP(0);
      mqttport = MDNS.port(0);
      for (int i = 0; i < n; ++i) {
        // Print details for each service found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(MDNS.hostname(i));
        Serial.print(" (");
        Serial.print(MDNS.IP(i));
        Serial.print(":");
        Serial.print(MDNS.port(i));
        Serial.println(")");
/*
        // 'Preferred' mDNS hostname
        if(strcmp(MDNS.hostname(i).c_str(), "iotstation") == 0)
        {
          Serial.println("iotstation found");
          server = MDNS.IP(i);
          mqttport = MDNS.port(i);
          break;
        }
*/
      }
    }
    else
    {
      Serial.println("Retry mDNS");
      delay(1000);
    }
    Serial.println();
  }

  Serial.println("Connecting");
  client.setServer(server, mqttport);
  client.setCallback(callback);
  Serial.println("Connected.");
}

void RandomClientName()
{
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(clientname, 25, "%s%d",uid, micros() & 0xff);
}

void loop() {
  // Connect wifi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
  }

  // Connect MQTT
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      RandomClientName();
      if (client.connect(clientname, MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println("MQTT connected");
        client.subscribe(MQTT_TOPIC);
      }
      else
        return;
    }
    else
      client.loop();
  }
  
  delay(100);
}
