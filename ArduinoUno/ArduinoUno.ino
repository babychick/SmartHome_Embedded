#include <SoftwareSerial.h>
#include <SerialCommand.h>
#include <ArduinoJson.h>
#include <Servo.h>


const byte rx = 3, tx = 2;
 
SoftwareSerial mySerial(rx, tx); 
SerialCommand sCmd(mySerial);

int digitalPin[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
int servo = digitalPin[7];
Servo controlDoor;

// sử dụng cho cảm biến hồng ngoại A0 -> A3
int analogPin[] = {A0, A1, A2, A3, A4, A5};

// 0: mở, 1: đóng
boolean firstState[] = {false , false, false, false, false};
boolean lastState[] = {false , false, false, false, false};

String data = "";

void sendtoESP(String data)
{
    mySerial.print("A2ESP");
    mySerial.print('\r');
    mySerial.print(data);
    mySerial.print('\r');
}

void changeState(String device, String pos, int pin, boolean st)
{
    if (st) 
    {
        digitalWrite(pin, HIGH);
        Serial.println(device + " - " + pos + " - ON");
    } 
    else 
    {
        digitalWrite(pin, LOW);
        Serial.println(device + " - " + pos + " - OFF");
    }
    // send to ESP
    sendtoESP(data);
}

// pos là position
// st là state

void Light(String deviceName, String pos, boolean st)
{ 
    if(pos == "LIVINGROOM")
    {
        if (deviceName == "Light 1")
            changeState(deviceName, pos, digitalPin[0], st);
        else if (deviceName == "Light 2")
            changeState(deviceName, pos, digitalPin[1], st);
    }
    else if(pos == "BEDROOM")  
    {
        if (deviceName == "Light 1")
            changeState(deviceName, pos, digitalPin[2], st);
        else if (deviceName == "Light 2")
            changeState(deviceName, pos, digitalPin[3], st);
    }
    else if(pos == "DININGROOM") 
    {
        if (deviceName == "Door")
            changeState(deviceName, pos, digitalPin[4], st);
        else if (deviceName == "Light 2")
            changeState(deviceName, pos, digitalPin[5], st);
    }
    else if(pos == "BATHROOM") 
    {
        if (deviceName == "Light")
        {
            changeState(deviceName, pos, digitalPin[6], st);
        }
    }
}

void openDoor()   {  }

void closeDoor()  {  }

void Door(String deviceName, String pos, boolean st)
{
    if (pos == "LIVINGROOM")
    {
        if (deviceName == "Main Door")
        {
            if (st == true)
                openDoor();
            else closeDoor();
            sendtoESP(data);
        }
    }
}

void Filter(JsonObject& obj)
{
    String deviceName = obj["deviceName"];
    String deviceType = obj["deviceType"];
    String pos = obj["position"];
    boolean state = obj["state"];
    
    if (deviceType == "LIGHT")
        Light(deviceName, pos, state);
    if (deviceType == "DOOR")
        Door(deviceName, pos, state);
}

void parseJsonObject(String s)
{
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& obj = jsonBuffer.parseObject(s);
    
    if (obj.success())
        Filter(obj);    
    else Serial.println("Parsing failed!!!");
}

void pinSetup()
{
    // set pin mode to Digital
    for (int i = 0; i < 10; i++)
    {
        pinMode(digitalPin[i], OUTPUT);
    }
  
//    servo.attach(controlDoor);
}

void readfromESP ()
{
    data = sCmd.next(); //Chỉ cần một dòng này để đọc tham số nhận đươc
    Serial.println(data);
    parseJsonObject(data);
}

void createJsonObject(String deviceName, String deviceType, String pos, boolean state)
{
      StaticJsonBuffer<512> buff;
      JsonObject& data = buff.createObject();
      data["deviceName"] = deviceName;
      data["deviceType"] = deviceType;
      data["position"] = pos;
      data["state"] = state;
      
      // send
      mySerial.print("A2ESP");
      mySerial.print('\r');
      data.printTo(mySerial);
      mySerial.print('\r');              
}

int r, r1, r2, r3, r4;

void checkDoor()
{
    // main door
    r = analogRead(analogPin[0]);
    // window in livingroom
    r1 = analogRead(analogPin[1]);
    // window in diningroom
    r2 = analogRead(analogPin[2]);
    // window in bedroom
    r3 = analogRead(analogPin[3]);
    // pir
    r4 = analogRead(analogPin[4]);
    
    if (r > 200)
      firstState[0] = false;
    else firstState[0] = true;
    
    if (r1 > 200)
      firstState[1] = false;
    else firstState[1] = true; 
      
    if (r2 > 200)
      firstState[2] = false;
    else firstState[2] = true;    
      
    if (r3 > 200)
      firstState[3] = false;
    else firstState[3] = true;

    if (r4 < 500)
      firstState[4] = false;
    else firstState[4] = true;
    
    for (int i = 0; i < 5; i++)
    {
        if (lastState[i] != firstState[i])
        {    
            // create json object and send to ESP
            switch(i)
            {
                case 0:
                    createJsonObject("Main Door", "DOOR", "LIVINGROOM", firstState[i]);
                    break;
                case 1:
                    createJsonObject("Window 1", "DOOR", "LIVINGROOM", firstState[i]);
                    break;
                case 2:
                    createJsonObject("Window 1", "DOOR", "DININGROOM", firstState[i]);
                    break;
                case 3:
                    createJsonObject("WINDOW 1", "DOOR", "BEDROOM", firstState[i]);
                    break;
                case 4:
                    createJsonObject("PIR", "SENSOR", "LIVINGROOM", firstState[i]);
                    break;
            }
        }
        lastState[i] = firstState[i];
    }
} 

void setup() {
    Serial.begin(19200);
    mySerial.begin(19200);
    sCmd.addCommand("ESP2A", readfromESP);
    Serial.println("Ready...");
}

void loop() 
{
    sCmd.readSerial();  
    checkDoor();  
}
