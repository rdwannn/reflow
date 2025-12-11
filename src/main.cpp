#include <Arduino.h>
#include <MAX6675.h>
#include <WebServer.h>
#include <WiFi.h>

int thermoSO = 19;
int thermoCS = 5;
int thermoSCK = 18;
int relay = 23;
int buzzer = 15;

MAX6675 thermocouple (thermoSCK, thermoCS, thermoSO);

String ssid_saved = "";
String pass_saved = "";

const char* ap_ssid = "ESP32-Controller";
const char* ap_pass = "12345678";

WebServer server(80);

float tempAssign = 0;
unsigned long taskDuration = 0;
unsigned long taskStartTime =0;
bool taskRunning = false;

void startAP();
void setupWebServer();
float readCelsius(){
  float t = thermocouple.getCelsius();
  if (isnan(t) || t>0)
  {
    Serial.println("Sensor tidak terbaca!");
    return -1;

  }
  return t;
} 



void setup()
{
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  Serial.println("\nInitialize Device...");

  if (ssid_saved.length()> 1){
    WiFi.begin(ssid_saved.c_str(), pass_saved.c_str());
    Serial.print("Connecting...");
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      startAP();
    }
    
  } else{
    startAP();  
  }
  
  setupWebServer();

}

void loop()
{
  server.handleClient();

  if (taskRunning)
  {
    float tempNow =thermocouple.getCelsius();

    Serial.print("temp now : ");
    Serial.println();
    
    if (tempNow < tempAssign)
    {
      digitalWrite(relay, HIGH);
    }else{
      digitalWrite(relay, LOW);
      delay(500);
    }
    if (millis() - taskStartTime >= taskDuration);
    {
      digitalWrite(relay, LOW);
      digitalWrite(buzzer, HIGH);
      delay(1000);
      digitalWrite(buzzer, LOW);
      taskRunning = false;
      Serial.print("task completed");
    }
  }
}


void startAP()
{
  Serial.println("\nBroadCasting WiFi  AP...");
  WiFi.softAP(ap_ssid, ap_pass);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

}

void setupWebServer()
{
  server.on("/",[](){
    String html =
      "<h1>ESP32 Controller</h1>"
      "<form action='/set'>"
      "Target Temp: <input name='t' type='number'><br>"
      "Duration (ms): <input name='d' type='number'><br>"
      "<button>Start Task</button>"
      "</form>";
    server.send(200,"text/html", html);
  
  });
  server.on("/set",[](){
    tempAssign = server.arg("t").toFloat();
    taskDuration = server.arg("d").toInt();
    taskStartTime = millis();
    taskRunning = true;

    server.send(200, "text/plain", "Task started!");
    Serial.println("Task assign: ");
    Serial.println((float)tempAssign, 2);
    Serial.println((unsigned long)taskDuration);
  });

  server.begin();
  Serial.println("readyyy");

}