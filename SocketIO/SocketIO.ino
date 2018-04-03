#include <ESP8266HTTPClient.h>
#include <SocketIOClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
 
SocketIOClient client;
ESP8266WiFiMulti wifimulti;
HTTPClient httpGet;

int yellow = D8;
int green = D7;

char host[] = "192.168.2.109";  
int port = 3000;                  

extern String RID; // tên sự kiện
extern String Rfull; // json
String GET_packet = "";
 
unsigned long prevTime = 0;
long interval = 5000;

void setupNetwork() {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    wifimulti.addAP("BON FPT", "kemtuchon19k");
    wifimulti.addAP("FPT Telecom", "dongthap");
    wifimulti.addAP("AnhTraiMua", "meochonhe");

    uint8_t i = 0;
    while (wifimulti.run() != WL_CONNECTED && i++ < 20) delay(500);
    if (i == 21) {
        Serial.println("Can not connect wifi!");
        while (1) delay(500);
    }
    Serial.print("Wifi connected to ");    Serial.println(WiFi.SSID());
    
    Serial.print("ESP8266 IP: ");    Serial.println(WiFi.localIP());
}

void changeState(int pin, boolean st)
{
    if (st) {
      digitalWrite(pin, HIGH);
      Serial.println("Led on!");
    } 
    else {
      digitalWrite(pin, LOW);
      Serial.println("Led off !");
    }
}

void Led(const char* device, boolean st)
{
  if (device == "LED 1")
        changeState(yellow, st);
  if (device == "LED 2")
        changeState(green, st);
}

void parseJsonObject()
{
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& obj = jsonBuffer.parseObject(Rfull);
  if (!obj.success())
  {
    Serial.println("Parsing failed!!!");
  }
  else 
  {
    const char* deviceName = obj["deviceName"];
    const char* description = obj["description"];
    boolean state = obj["state"];
    boolean conn = obj["connect"];
    Serial.println(deviceName);
    Serial.println(description);
    Serial.println(state);
    Serial.println(conn);
    if (RID == "s2d-ledchange")
    {
      Led(deviceName, state);
    }
  }
}

void parseJsonArray(String s)
{
  StaticJsonBuffer<512> bufferred;
  JsonArray& arr = bufferred.parseArray(s);
  if (arr.success())
  {
    for (int i = 0; i < arr.size(); i++)
    {
      JsonObject& object = arr[i];
      const char* deviceName = object["deviceName"];
      const char* typeDevice = object["typeDevice"];
      boolean state = object["state"];
      boolean conn = object["connect"];
      Serial.println(deviceName);
      Serial.println(typeDevice);
      Serial.println(state);
      Serial.println(conn);
    }
  }
  else Serial.println("parsing failed...");
}

void checkMSG()
{
  if (RID != "")
  {
      parseJsonObject();
  }
  else Serial.println("Parsing...");
}

void setup()
{
    pinMode(yellow, OUTPUT);
    pinMode(green, OUTPUT);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
    Serial.begin(115200);
    setupNetwork();
    client.connect(host, port);
    Serial.println("connected to server!");
        
    httpGet.begin("http://192.168.2.109:3000/devices");
    if (httpGet.GET())
    {
      GET_packet = httpGet.getString();   //Get the request response payload
      Serial.println(GET_packet); 
      parseJsonArray(GET_packet); 
    }
    httpGet.end();
//    client.send("d2s-ledchange","{\"deviceName\":\"LED 1\", \"typeDevice\":\"LED\", \"description\":\"No description\", \"state\":false, \"connect\":true}");
//    Serial.println("sent");
}

void loop()
{
//  if (client.monitor())
//  {
////    Serial.println("RID: " + RID);
////    Serial.println("Rfull: " + Rfull);
////    parseJsonObject();
//client.getREST("devices");
//  }
    
}
