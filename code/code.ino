#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Gakibia hostel";
const char* password = "emmakim19";
float resistance;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 3000;

//Initialize WiFi
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}
void getSensorReadings(){
  resistance = analogRead(A0);
  resistance = map(resistance, 0, 1023, 0, 255);
  }
String processor(const String& var){
  //Serial.println(var);
  getSensorReadings();
  if(var == "RESISTANCE"){
    return String(resistance);
  }

  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
 <div class="content"> 
  <div class="topnav">
    <h1>LDR WEB SERVER (SSE)</h1>
  </div>
 <div class="card">
        <p><i class="fas fa-angle-double-down" style="color:#e1e437;"></i> RESISTANCE </p><p><span class="reading"><span id="res">%RESISTANCE%</span> ohms</span></p>
      </div>
 </div>     
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
  source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);

 source.addEventListener('resistance', function(e) {
  console.log("resistance", e.data);
  document.getElementById("res").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";
  

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
initWiFi();

// Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

// Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });  
 server.addHandler(&events);
  server.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
if ((millis() - lastTime) > timerDelay){
  getSensorReadings();
  Serial.printf("Resistance = %.2f ohms \n", resistance);
  Serial.println();
  
  // Send Events to the Web Server with the Sensor Readings
  events.send("ping",NULL,millis());
  events.send(String(resistance).c_str(),"resistance",millis());
  
  lastTime = millis();
  }


}
