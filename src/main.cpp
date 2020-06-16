#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

// more information at:
// https://www.hackster.io/andrewf1/simplest-wifi-car-using-esp8266-motorshield-37501e

char my_hostname[33];
int PWMA = 5; //Right side
int PWMB = 4; //Left side
int DA = 0;   //Right reverse
int DB = 2;   //Left reverse
int speed = 450;

AsyncWebServer server(80);

void handleNotFound(AsyncWebServerRequest *request)
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}

void generateForm(AsyncResponseStream *response, int x, int y) {
  response->print("<form action='/move' method='post' style='float: left;'>");
  response->printf("<input type='hidden' name='x' value='%d'>", x);
  response->printf("<input type='hidden' name='y' value='%d'>", y);
  response->print("<button type='submit' style='width: 50px; height: 50px;'> </button>");
  response->print("</form>");
}

void generateLine(AsyncResponseStream *response, int x) {
  response->print("<div style='overflow: hidden;'>");
  generateForm(response, x, -1);
  generateForm(response, x, 0);
  generateForm(response, x, 1);
  response->print("</div>");
}

void moveCar(int x, int y)
{
  if (x == -1 && y == -1)
  { //Backward and Left
    digitalWrite(PWMA, LOW);
    digitalWrite(DA, LOW);

    digitalWrite(PWMB, HIGH);
    digitalWrite(DB, HIGH);
  }
  else if (x == -1 && y == 0)
  { //Left Turn
    digitalWrite(PWMA, speed);
    digitalWrite(DA, HIGH);

    digitalWrite(PWMB, speed);
    digitalWrite(DB, LOW);
  }
  else if (x == -1 && y == 1)
  { //Forward and Left
    digitalWrite(PWMA, LOW);
    digitalWrite(DA, LOW);

    digitalWrite(PWMB, HIGH);
    digitalWrite(DB, LOW);
  }
  else if (x == 0 && y == -1)
  { //Backward
    digitalWrite(PWMA, HIGH);
    digitalWrite(DA, HIGH);

    digitalWrite(PWMB, HIGH);
    digitalWrite(DB, HIGH);
  }
  else if (x == 0 && y == 0)
  { //Stay
    digitalWrite(PWMA, LOW);
    digitalWrite(DA, LOW);

    digitalWrite(PWMB, LOW);
    digitalWrite(DB, LOW);
  }
  else if (x == 0 && y == 1)
  { //Forward
    digitalWrite(PWMA, HIGH);
    digitalWrite(DA, LOW);

    digitalWrite(PWMB, HIGH);
    digitalWrite(DB, LOW);
  }
  else if (x == 1 && y == -1)
  { //Backward and Right
    digitalWrite(PWMA, HIGH);
    digitalWrite(DA, HIGH);

    digitalWrite(PWMB, LOW);
    digitalWrite(DB, LOW);
  }
  else if (x == 1 && y == 0)
  { //Right turn
    digitalWrite(PWMA, speed);
    digitalWrite(DA, LOW);

    digitalWrite(PWMB, speed);
    digitalWrite(DB, HIGH);
  }
  else if (x == 1 && y == 1)
  { //Forward and Right
    digitalWrite(PWMA, HIGH);
    digitalWrite(DA, LOW);

    digitalWrite(PWMB, LOW);
    digitalWrite(DB, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(DA, OUTPUT);
  pinMode(DB, OUTPUT);

  snprintf_P(my_hostname, sizeof(my_hostname) - 1, PSTR("%s-%d"), "leaCar", ESP.getChipId() & 0x1FFF);

  Serial.println();
  Serial.print("Konfiguriere soft-AP ");
  Serial.print(my_hostname);
  Serial.println(" ... ");

  WiFi.mode(WIFI_AP);
  boolean result = WiFi.softAP(my_hostname, "geheim123");

  Serial.print("Verbindung wurde ");
  if (result == false)
  {
    Serial.println("NICHT ");
  }
  Serial.print("erfolgreich aufgebaut!");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Directions</title></head><body>");
    response->print("<p>Press this joystick:</p>");
    generateLine(response, -1);
    generateLine(response, 0);
    generateLine(response, 1);
    response->print("</body></html>");
    request->send(response);
  });

  server.on("/move", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(request->hasParam("x", true) && request->hasParam("y", true)) {
      AsyncWebParameter* xParam = request->getParam("x", true);
      AsyncWebParameter* yParam = request->getParam("y", true);
      moveCar(xParam->value().toInt(), yParam->value().toInt());
    }
    request->redirect("/");
    // request->send(200, "application/json", F("{\"success\":true}"));
  });

  server.onNotFound(handleNotFound);
  server.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
}
