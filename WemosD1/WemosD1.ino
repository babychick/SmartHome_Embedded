#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include <SocketIOClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
 
SocketIOClient client;
ESP8266WiFiMulti wifimulti;
HTTPClient httpClient;

const byte rx = D1;
const byte tx = D2;
 
SoftwareSerial mySerial(rx, tx, false, 256);
SerialCommand sCmd(mySerial);

char host[] = "172.16.199.170";   
int port = 3000;                  

extern String RID; // tên sự kiện
extern String Rfull; // json

void setupNetwork()
{
    WiFi.persistent(false);

    // access to a wifi
    WiFi.mode(WIFI_STA);
    wifimulti.addAP("FPT Telecom", "dongthap");
    wifimulti.addAP("AnhTraiMua", "meochonhe");
    wifimulti.addAP("HoangPhat_Pro", "20052010");
    wifimulti.addAP("ANDY", "01666988779");

    // connecting
    uint8_t i = 0;
    while (wifimulti.run() != WL_CONNECTED && i++ < 20) delay(500);
    if (i == 21) {
        Serial.println("Can not connect wifi!");
        while (1) delay(500);
    }

    //connected
    Serial.print("\nWifi connected to ");    
    Serial.println(WiFi.SSID());
    
    Serial.print("ESP8266 IP: ");    
    Serial.println(WiFi.localIP());
}


void listenSocketIO()
{
    if (RID == "s2d-change")
    {        
        mySerial.print("ESP2A");
        mySerial.print('\r');
        mySerial.print(Rfull);
        mySerial.print('\r');
    }
    else Serial.println("Waiting for imcoming data...");
}

void parseJsonArray(String s)
{
    DynamicJsonBuffer bufferred(512);
    JsonObject& object = bufferred.parseObject(s);
    JsonArray& arr = object["devices"];
    if (arr.success())
      for (int i = 0; i < arr.size(); i++)
      {
          JsonObject& device = arr[i];
          mySerial.print("ESP2A");
          mySerial.print('\r');
          device.printTo(mySerial);
          mySerial.print('\r');
          delay(100);
      }
    else Serial.println("parsing failed!!!");
}

void loadDeviceByType(String deviceType)  
{ 
    String serverAdd = host;
    Serial.println("Get POST from http://" + serverAdd + ":3000/devices/"); 
    httpClient.begin("http://" + serverAdd + ":3000/devices/");
    
    httpClient.addHeader("Content-Type", "application/json");

    String query = "{\"deviceType\":\"" + deviceType + "\"}";
    Serial.println("With query " + query);   
     
    int httpCode = httpClient.POST(query);
    
    if(httpCode == 200)
    {
        String payload = httpClient.getString();
        Serial.println(payload);
        parseJsonArray(payload); 
    }
    httpClient.end();
}

void readfromArduino()
{
  char* s = sCmd.next();
  Serial.println(s);
  client.send("d2s-change", s);
}

void setup()
{
    Serial.begin(19200);
    mySerial.begin(19200);
    setupNetwork();
    client.connect(host, port);
    Serial.println("connected to server!");
    loadDeviceByType("DOOR");
    loadDeviceByType("LIGHT");
    sCmd.addCommand("A2ESP", readfromArduino);
}

void loop()
{
    if (client.monitor())
    {
        listenSocketIO(); 
    }

    // reconnect
    if (!client.connected())
    {
        client.reconnect(host, port);
    }

     sCmd.readSerial();
}
