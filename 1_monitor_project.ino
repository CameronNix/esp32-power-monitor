#include<WiFi.h>
//Auther: Cameron Nix
//Description: Take the voltage readings and current readings from the sensors and apply RMS math to it
//Then calculate the total watts from those to values
//Print out the watts value via wifi over web browser

//define WiFi and Password
const char* ssid = "Lindsay";
const char* password = "Connor12!";


//define wifi server
WiFiServer server(80);

//begin esp and begin wifi and print out IP address
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
  }
  server.begin();
  Serial.println(WiFi.localIP());
}

//store voltage readings and apply RMS math
//bring voltage back down to original
//using the ZMPT101B voltage sensor 
float voltageRead() {

  float samples[1000];

  for(int i = 0; i < 1000; i++) {
    samples[i] = analogRead(35);
  }

  float total = 0;

  for(int i = 0; i < 1000; i++) {
    samples[i] = samples[i] - 2048;
    float square = samples[i] * samples[i];
    samples[i] = square;
    total = total + samples[i];
  }

  float mean =  total / 1000;
  float voltage = sqrt(mean);

  return voltage;
 }

//store current readings and apply RMS math
//bring current back down to original
//using the SCT-013 current transformer
float currentRead() {

  float samples[1000];

  for(int i = 0; i < 1000; i++) {
    samples[i] = analogRead(34);
  }

  float total = 0;

  for(int i = 0; i < 1000; i++) {
    samples[i] = samples[i] - 2048;
    float square = samples[i] * samples[i];
    samples[i] = square;
    total = total + samples[i];
  }

  float mean =  total / 1000;
  float current = sqrt(mean);

  return current;
 }


//calculate watts with the voltage and current readings
//also used a 10 ohms burden reistor instead of 33 ohms
//so I implemented a calibration multiplier to make up for it
 float calculateWatts(float voltage, float current) {
  float watts = voltage * current * 0.000475;
  return watts;
 }

//check if client is connected and send the HTML page with watts value
 void loop() {
  float result = calculateWatts(currentRead(), voltageRead());
  WiFiClient client = server.available();
  if(client) {
    String request = client.readStringUntil('\r');
    if(request.indexOf("/watts") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("");
      client.println(String(result));
    } else {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("");
      client.println("<h1 id='watts'>Watts: --</h1>");
      client.println("<script>");
      client.println("setInterval(function(){");
      client.println("fetch('/watts').then(r=>r.text()).then(t=>{");
      client.println("document.getElementById('watts').innerText='Watts: '+parseFloat(t).toFixed(2);");
      client.println("});},1000);");
      client.println("</script>");
    }
  }
}
