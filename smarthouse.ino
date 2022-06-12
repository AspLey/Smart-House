#include <HTTPClient.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include "WiFi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>



const char* ssid = "FFFFFF";
const char* password = "Mars1402";
String url = "http://192.168.132.206:80/Loglamas/Degerler";

#define TRIG_PIN  23  // ESP32 pin GIOP23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN  22  // ESP32 pin GIOP22 connected to Ultrasonic Sensor's ECHO pin
#define SERVOGaraj_PIN 26  // ESP32 pin GIOP26 connected to Servo Motor's pin
#define SERVOKapi_PIN 13  // ESP32 pin GIOP26 connected to Servo Motor's pin
#define DISTANCE_THRESHOLD  10 // centimeters


#define MOTION_SENSOR_PIN  32  // ESP32 pin GIOP32 connected to the OUTPUT pin of motion sensor
#define BUZZER_PIN         33  // ESP32 pin GIOP17 connected to Buzzer's pin
int motionStateCurrent  = LOW; // current  state of motion sensor's pin
int motionStatePrevious = LOW;



#define DHT_SENSOR_PIN  21 // ESP32 pin GIOP21 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11
DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);




Servo servo; // create servo object to control a servo
Servo servo2; // create servo object to control a servo
float duration_us, distance_cm;



#define LIGHT_SENSOR_PIN  36  // ESP32 pin GIOP36 (ADC0) connected to light sensor
#define LED_PIN           25  // ESP32 pin GIOP22 connected to LED
#define ANALOG_THRESHOLD  1000
#define LED_PIN2           14  // ESP32 pin GIOP22 connected to LED



int Gas_analog = 4 ;    // ESP32 için kullanılır 
int Gas_digital = 2 ;

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}
String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE Html>

<head>
    <meta charset="utf-8">
    <title> Web Sayfasi </title>
    <style>
        html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
        body {
            margin: 0;
            text-align: center;
        }
        
        button {
            width: 200px;
            height: 100px;
            border-radius: 3px;
        }
    </style>
</head>

<body>
   // <h2>Esp32 İle Asenkron Web Server</h2>
    <br>
    <button style="background-color: #77d772;" id="ac" onclick="myfunction('ac')"><b>Aç</b></button>
    <button style="background-color: red;" id="kapat" onclick="myfunction('kapat')"><b>Kapat</b></button>
    <h4>Led Durum:
        <h4 id="leddurum">Led Kapalı</h4>
    </h4>

     <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
    <script>
        function myfunction(value) {
            var xhr = new XMLHttpRequest();
            if (value == 'ac') {
                document.getElementById("leddurum").innerHTML = "Led Açık";
                xhr.open("GET", "/ac", true);
                xhr.send();
            } else {
                document.getElementById("leddurum").innerHTML = "Led Kapalı";
                xhr.open("GET", "/kapat", true);
                xhr.send();
            }
        }
    </script>
</body>

</Html>)rawliteral";

String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Sayfa Bulunamdadi");
}
AsyncWebServer server(80);
void setup()
{
  
  Serial.begin (9600);
  srvwifi();
  initGenel();
  inityakinlik();
  initlight();
  initsicaksensor();
  initgassensor();
}

void loop()
{
 yakinlikolc();
 alarmsensor();
 ldr();
 gas();
}
void inityakinlik()
{
  // configure the trigger pin to output mode
  pinMode(TRIG_PIN, OUTPUT);
  // configure the echo pin to input mode
  pinMode(ECHO_PIN, INPUT);
  servo.attach(SERVOGaraj_PIN);
}

void yakinlikolc(){
   // generate 10-microsecond pulse to TRIG pin
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // measure duration of pulse from ECHO pin
  duration_us = pulseIn(ECHO_PIN, HIGH);
  // calculate the distance
  distance_cm = 0.017 * duration_us;

  if (distance_cm < DISTANCE_THRESHOLD){
    servo.write(90); // rotate servo motor to 90 degree
    postislem("Garaj Kapısı","Garaj kapısı açık.");
   
    }
  else{
    servo.write(0);  // rotate servo motor to 0 degree
     
  }
  // print the value to Serial Monitor
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  delay(500);
}

void initGenel(){
   pinMode(BUZZER_PIN, OUTPUT);
      pinMode(LED_PIN, OUTPUT);
         pinMode(LED_PIN2, OUTPUT);
          
}
void initlight(){
 pinMode(MOTION_SENSOR_PIN, INPUT); // set ESP32 pin to input mode
  
}

void alarmsensor(){
  motionStatePrevious = motionStateCurrent;            // store old state
  motionStateCurrent  = digitalRead(MOTION_SENSOR_PIN); // read new state

  if (motionStatePrevious == LOW && motionStateCurrent == HIGH) 
  { // pin state change: LOW -> HIGH
    Serial.println("Motion detected!, making sound");
    digitalWrite(BUZZER_PIN, HIGH); // turn on
    postislem("Hareket Sensörü","Hareket başladı.");
  } 
  else if (motionStatePrevious == HIGH && motionStateCurrent == LOW) 
  { // pin state change: HIGH -> LOW
    Serial.println("Motion stopped!, stops making sound");
    digitalWrite(BUZZER_PIN, LOW);  // turn off
    postislem("Hareket Sensörü","Hareket durdu.");
  }
}

void initsicaksensor(){
  dht.begin(); // initialize the DHT sensor
}

void ldr(){
  int analogValue = analogRead(LIGHT_SENSOR_PIN); // read the value on analog pin
  Serial.print(analogValue);
  if (analogValue > ANALOG_THRESHOLD){
    digitalWrite(LED_PIN, HIGH); // turn on LED
    postislem("Işık Sensörü","ışık yandı");
  }
  else{
   
    digitalWrite(LED_PIN, LOW);  // turn off LED
     }
}

void initgassensor(){      
  pinMode(Gas_digital, INPUT);
}

void gas(){
  int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);

  Serial.print("Gas Sensor: ");
  Serial.print(gassensorAnalog);
  Serial.print("\t");
  Serial.print("Gas Class: ");
  Serial.print(gassensorDigital);
  Serial.print("\t");
  Serial.print("\t");
  
  if (gassensorDigital==false) {
    Serial.println("Gas");
    digitalWrite (BUZZER_PIN , HIGH) ; //send tone
    delay(1000);
    digitalWrite (BUZZER_PIN , LOW) ;  //no tone
    postislem("Gaz Sensörü", "Gaz Algılandı");
  }
  else {
    Serial.println("No Gas");
  }
  delay(100);
}

void srvwifi()
{
    servo2.attach(SERVOKapi_PIN);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Wifi agina Baglanmadi!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  
  server.on("/ac", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(LED_PIN2, HIGH);
    Serial.println("led açık");
    postislem("Led","Lamba açıldı.");
    request->send(200, "text/plain", "ac sinyali gonderildi.");
  });

  
  server.on("/kapat", HTTP_GET, [] (AsyncWebServerRequest *request) {
    digitalWrite(LED_PIN2, LOW);
    Serial.println("led kapalı");
     postislem("Led","Lamba Kapatıldı.");
    request->send(200, "text/plain", "kapat sinyali gönderildi.");
  });

  server.on("/sac", HTTP_GET, [] (AsyncWebServerRequest *request) {
    servo2.write(90);
    Serial.println("servoacıldı");
     postislem("Kapi","Kapi açıldı.");
    request->send(200, "text/plain", "ac sinyali gonderildi.");
  });

   server.on("/skapat", HTTP_GET, [] (AsyncWebServerRequest *request) {
    servo2.write(0);
    Serial.println("servokapandı");
     postislem("Kapi","Kapı kapatıldı.");
    request->send(200, "text/plain", "Kapat sinyali gonderildi.");
  });
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  server.onNotFound(notFound);
  server.begin();
}

String postislem(String ad,String mesaj)
{
  HTTPClient http; // NodeMCU nun HTTPClient nesnesinden bir örnek alınıyor
  String postData; // Web sitesindeki php'ye göndereceğimiz veri için string tanımlıyoruz
  postData = "ad=" + ad + "&mesaj="+ mesaj; // ilgili stringe verimizi ekliyoruz. cardid field'i ile
   Serial.println(postData);
  http.begin(url); // HTTPClient e URL veriyoruz
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // HTTP Header olarak Content-Type i form olarak ayarlıyoruz
  int httpCode = http.POST(postData); // POST ile veriyi ilgili URL ye gönderiyoruz ve dönen response kodunu integer bir değişkene alıyoruz
  Serial.println(httpCode); // Seriale yazılıyor
   String payload; // Dönen veriyi almak için string bir değişken tanımlıyoruz
  if (httpCode != 200) // http response kodu 200 değil donecek kodu 0 veriyoruz
  {
    payload = "0";
  }
  else {
    payload = http.getString(); //http response kodu 200 ise response text'i değişkene ekliyoruz
  }
  Serial.println(payload); // Seriale yazılıyor
  http.end(); // Bağlantı sonlandırılıyor
  return payload; // Veri geri gönderiliyor
}
