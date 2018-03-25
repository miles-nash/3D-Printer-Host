/*
This sketch will receive data from octoprint, write it to thingspeak channels, and display the data on a neopixel ring.
 */
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <String.h>
#include <OctoPrintAPI.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiClient.h>

unsigned long api_mtbs = 60000; //mean time between api requests
unsigned long api_lasttime = 0;   //last time api request has been done
unsigned long api2_mtbs = 60000; //mean time between api requests
unsigned long api2_lasttime = 0;   //last time api request has been done
#define ringLength 16
#define PIN 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);
uint32_t off = strip.Color(0,0,0);
WiFiClient client; 
//------------------------------------------
//              User Data
//------------------------------------------

const char* ssid     = "----------";       //your SSID(name) of WIFI
const char* password = "----------";       // password of Wifi

String OctoPrintAPIKey = "----------";     //API key from OctoPrint
String PrinterTSAPIKey = "----------";     //write api key for thingspeak printer channel
String JobTSAPIKey = "----------";         //write api key for thingspeak job channel
String POSTWriteAPIKey = "----------";     //write api key for thingspeak POST channel
String POSTReadApiKey = "----------";      //read api key for thingspeak POST channel
String POSTid = "------";                  //channel id for thingspeak POST channel

IPAddress ip(--, -, -, ---);               // IP adress of Raspberry Pi. 11.1.1.111 = (11, 1, 1, 111)
const int octoprint_httpPort = 80;         //If you are connecting through a router this will work, but you need a random port forwarded to the OctoPrint server from your router. Enter that port here if you are external


//------------------------------------------
//           Color Setttings
//------------------------------------------
int background = 1;                                          //Would you like yout Printer Host to show a color even when not printing? 1 = yes 0 = no
int brightness = 150;                                        //How bright would you like your Neopixel Display? 1-255
uint32_t standby = strip.Color(0, 0, 255);                   //What color would you like to show as the background (disregard if you do not want a background)
uint32_t error = strip.Color(255, 0, 0);                     //What color would you like to indicate that the printer is disconnected or experiencing an error
uint32_t paused = strip.Color(255, 255, 0);                  //What color would you like to indicate that the printer is paused
uint32_t printing = strip.Color(0, 255, 0);                  //What color would you like to indicate that the printer is printing
//All Colors Are In RGB Format
//------------------------------------------
//------------------------------------------
bool debug = true;
String TSPostString;
int bedTemp;
int toolTemp;
String operational;    
String status; 
String fileName;
String percentComplete;
int percent;
int timePrinted;
int timePrintedRemaining;
String timePrintedHours;
String timePrintedMinutes;
int timeLeft;
int timeLeftRemaining;
String timeLeftHours;
String timeLeftMinutes;
int estimatedTime;
int estimatedTimeRemainder;
String estimatedTimeHours;
String estimatedTimeMinutes;
const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
const char* host = "api.thingspeak.com";
//WiFiClient client; 
OctoprintApi api(client, ip, octoprint_httpPort, OctoPrintAPIKey);

void wifiConnect(){
  WiFi.begin(ssid, password);
  Serial.print("ssid");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup() {
  
  Serial.begin(115200);
  delay(10);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off' 
  strip.setBrightness(brightness);
  
  // start by connecting to WiFi 
  
  wifiConnect();

}



void loop() {

  delay(100);

 //-----------------------------------------------------------------------------
 //                  Fetch data from Octoprint
 //-----------------------------------------------------------------------------


  if (millis() - api_lasttime > api_mtbs || api_lasttime==0) { //only do this after a certain period of time has passed
  
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status     
      if(api.getOctoprintVersion()){
        Serial.println("---------Version---------");
        Serial.print("Octoprint API: ");
        Serial.println(api.octoprintVer.octoprintApi);
        Serial.print("Octoprint Server: ");
        Serial.println(api.octoprintVer.octoprintServer);
        Serial.println("------------------------");
      }
      Serial.println();
      if(api.getPrinterStatistics()){
        Serial.println("---------Stats---------");
        Serial.print("Printer Current State: ");
        Serial.println(api.printerStats.printerState);
        Serial.print("Printer State - closedOrError:  ");
        Serial.println(api.printerStats.printerStateclosedOrError);
        Serial.print("Printer State - error:  ");
        Serial.println(api.printerStats.printerStateerror);
        Serial.print("Printer State - operational:  ");
        Serial.println(api.printerStats.printerStateoperational);
        Serial.print("Printer State - paused:  ");
        Serial.println(api.printerStats.printerStatepaused);
        Serial.print("Printer State - printing:  ");
        Serial.println(api.printerStats.printerStatePrinting);
        Serial.print("Printer State - ready:  ");
        Serial.println(api.printerStats.printerStateready);
        Serial.print("Printer State - sdReady:  ");
        Serial.println(api.printerStats.printerStatesdReady);
        Serial.println();
        Serial.println("------Termperatures-----");
        Serial.print("Printer Temp - Tool (°C):  ");
        Serial.println(api.printerStats.printerTool0TempActual);
        Serial.print("Printer State - Bed (°C):  ");
        Serial.println(api.printerStats.printerBedTempActual);
        Serial.println("------------------------");           
      }
      Serial.println();
      if(api.getPrintJob()){  //Get the print job API endpoint
        Serial.println("---------------------------");
        Serial.print("Printer current state string:\t");
        Serial.println(api.printJob.printerState);

        Serial.println();
        Serial.println("----------Job (File information)--------");
        Serial.print("jobFileDate (Epoch) long:\t");
        Serial.println(api.printJob.jobFileDate);
        Serial.print("jobFileName String:\t\t");
        Serial.println(api.printJob.jobFileName);
        Serial.print("jobFileOrigin String:\t\t");
        Serial.println(api.printJob.jobFileOrigin);
        Serial.print("jobFileSize (bytes) long:\t");
        Serial.println(api.printJob.jobFileSize);
        Serial.print("estimatedPrintTime (sec) long:\t");
        Serial.println(api.printJob.estimatedPrintTime);
        Serial.println("----------------------------------------");  
        Serial.println();
        Serial.println("-------Job (Progress information)-------");
        Serial.print("progressCompletion (%) float:\t\t");
        Serial.println(api.printJob.progressCompletion);
        Serial.print("progressFilepos (bytes) long:\t\t");
        Serial.println(api.printJob.progressFilepos);
        Serial.print("progressPrintTime (sec) long:\t\t");
        Serial.println(api.printJob.progressPrintTime);
        Serial.print("progressPrintTimeLeft (sec) long:\t");
        Serial.println(api.printJob.progressPrintTimeLeft);
        Serial.println("----------------------------------------");
        Serial.println();  
      }
      //Map Values to Variables
      operational = api.printerStats.printerStateoperational;
        
        
      if(api.printerStats.printerStatePrinting == 1){
        status = "1"; //if printing, set status to printing
      }else if(api.printerStats.printerStatepaused == 1){
        status = "2";
      }else if(api.printerStats.printerStateready == 1){
        status = "3";
      }else{
        status = "4";
      }
      //map all received data to variables and convert times to hh:mm format
      toolTemp = api.printerStats.printerTool0TempActual;
      bedTemp = api.printerStats.printerBedTempActual;
      fileName = api.printJob.jobFileName;
      estimatedTime = api.printJob.estimatedPrintTime;
      percentComplete = api.printJob.progressCompletion;
      percent = percentComplete.toFloat();
      timePrinted = api.printJob.progressPrintTime;
      timeLeft = api.printJob.progressPrintTimeLeft;
      timePrintedHours =  timePrinted / 3600;
      timePrintedRemaining = timePrinted % 3600;
      timePrintedMinutes = timePrintedRemaining /60;
      timeLeftHours =  timeLeft / 3600;
      timeLeftRemaining = timeLeft % 3600;
      timeLeftMinutes = timeLeftRemaining /60;
      estimatedTimeHours =  estimatedTime / 3600;
      estimatedTimeRemainder = estimatedTime % 3600;
      estimatedTimeMinutes = estimatedTimeRemainder /60;
    }
    api_lasttime = millis();
  }


 //-----------------------------------------------------------------------------
 //                  Update thingspeak with new data from octoprint
 //-----------------------------------------------------------------------------


  //connect to thingspeak
  if (millis() - api2_lasttime > api2_mtbs || api2_lasttime==0) {
    // Serial.print("connecting to ");
    //Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }


    //check for POST commands
    //code adapted from zoomkat arduino forum post: http://forum.arduino.cc/index.php?topic=44646.0
    client.print(String("GET ")  + "/channels/"+POSTid+"/feeds/last.json?api_key="+POSTReadApiKey+"&results=1 HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");                                                         //updates values in printer thingspeak channel   

                
     while(client.connected() && !client.available()) delay(1); //waits for data
     while (client.connected() || client.available()) { //connected or data available
      char c = client.read(); //gets byte from ethernet buffer
      TSPostString += c; //places captured byte in TSPostString
     }
    TSPostString.remove(0,TSPostString.indexOf("{")); //isolate the json data
    TSPostString.remove(TSPostString.indexOf("}")+1); //isolate the json data
    Serial.println(TSPostString);
    String fixedString = TSPostString;
    TSPostString=""; //clear TSPostString variable
    
    //parsing
    //code adapted from
    const size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonBuffer jsonBuffer(bufferSize);
    char b[104];
    TSPostString.toCharArray(b,104);
    JsonObject& root = jsonBuffer.parseObject(fixedString);
    //json parameters
    const char* field1 = root["field1"]; // "1"
    const char* field2 = root["field2"]; // "0"
    const char* field3 = root["field3"]; // "0"
    const char* field4 = root["field4"]; // "0"
    //debug
    Serial.println("debug");
    Serial.println(field1); 
    Serial.println(field2); 
    Serial.println(field3); 
    Serial.println(field4); 
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //TO DO: Clean Up, Parse, Connect to OctoPrint commands.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
    //update data
    //updates values in printer thingspeak channel
    client.print(String("GET ")  + "/update?api_key="+printerTSAPIKey+"&field1="+operational+"&field2="+status+"&field3="+toolTemp+"&field4="+bedTemp+" HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");                                                         //updates values in printer thingspeak channel   
                
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }


    //updates values in job thingspeak channel 
    client.print(String("GET ")  + "/update?api_key="+jobTSAPIKey+"&field1="+fileName+"&field2="+estimatedTimeHours+"&field3="+estimatedTimeMinutes+"&field4="+percentComplete+"&field5="+timePrintedHours+"&field6="+timePrintedMinutes+"&field7="+timeLeftHours+"&field8="+timeLeftMinutes+" HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");                                                        //updates values in job thingspeak channel

    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    delay(100);
    //Reset POST commands
    /*client.print(String("GET ")  + "/update?api_key="+POSTWriteApiKey+"&field1=0&field2=0&field3=0&field4=0 HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");
  */
    if (debug == true){
      //serial print data: for debugging
      Serial.println("Thingspeak Updated");
      delay(10);
      Serial.println("operational = " + operational);   
      delay(10);
      Serial.println("status = " + status);   
      delay(10);
      Serial.println("tool temp = " + toolTemp);
      delay(10);
      Serial.println("bed temp = " + bedTemp);
      delay(10);
      Serial.println("file name = " + fileName);   
      delay(10);
      Serial.println("estimated print time = " + estimatedTimeMinutes);   
      delay(10);
      Serial.println("percent complete = " + percentComplete);   
      delay(10);
      Serial.println("hours printed = " + timePrintedHours);   
      delay(10);
      Serial.println("minutes printed = " + timePrintedMinutes);   
      delay(10);
      Serial.println("Hours remaining = " + timeLeftHours); 
      delay(10);
      Serial.println("mnutes remaining = " + timeLeftMinutes); 
    }
    api2_lasttime = millis();
  }

  //-----------------------------------------------------------------------------
  //                  Check for Post commands
  //-----------------------------------------------------------------------------


  //-----------------------------------------------------------------------------
  //                  Display data using neopixels
  //-----------------------------------------------------------------------------
  if (operational == "0"){
    for(int i = 0; i<= ringLength; i++){  //display error color if experiencing an error
      strip.setPixelColor(i, error);
    }
  }else{
    if(status == "1"){  
      drawRing(printing, percent);  //display printing color on number of pixels representing print completion percentage
    }else if(status == "2"){
      drawRing(paused, percent);  //display paused color on number of pixels representing print completion percentage
    }else{
      if(background = 1){
        for(int i = 0; i<= ringLength; i++){
          strip.setPixelColor(i, standby); //display background color if a backround is wanted
        }
      }else{
        for(int i = 0; i<= ringLength; i++){
          strip.setPixelColor(i, off);  //turn all pixels off if no background is wanted
        }
      }
    }
  }
  strip.show();//show the pixels in their written state
}

void drawRing(uint32_t color, float percent){
  int pixelPercent = 100/ringLength; // the percentage needed to fill one pixel
  int fullPixels = percent/(pixelPercent); //the number of pixels that should be lit up 
  int i = 0;
  while (i < fullPixels){ //color all pixels up to fullPixels
    strip.setPixelColor(i, color);
    i++;
  }
}
