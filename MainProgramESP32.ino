/*
   contact: ajiaryad@gmail.com
*/
//Wifi Via Bluetooth
#include <WiFi.h>
#include <Preferences.h>
#include "BluetoothSerial.h"

String ssids_array[50];
String network_string;
String connected_string;
int notif = 1;

const char* pref_ssid = "";
const char* pref_pass = "";
String client_wifi_ssid;
String client_wifi_password;

const char* bluetooth_name = "Smart kWh Meter";

long start_wifi_millis;
long wifi_timeout = 10000;
bool bluetooth_disconnect = false;

enum wifi_setup_stages { NONE, SCAN_START, SCAN_COMPLETE, SSID_ENTERED, WAIT_PASS, PASS_ENTERED, WAIT_CONNECT, LOGIN_FAILED };
enum wifi_setup_stages wifi_stage = NONE;
BluetoothSerial SerialBT;
Preferences preferences;

//IOT
#include "ThingSpeak.h"

WiFiClient  client;

unsigned long myChannelNumber = 1238827;
const char * myWriteAPIKey = "GXLD67LXCPMIMQYZ";

//SENSOR
#include <PZEM004Tv30.h>
#define RXD1 14
#define TXD1 27
#define BAUD 115200
#define PROTOCOL SERIAL_8N1

int refreshLCD, refreshWEB, refreshSWifi;
int koma;

String strTeg, strArus, strPwr, strEnrgy, strFreq, strPf;
char kirim[160];

int bt1 = 32; //PORT BUTTON 1
int bt2 = 33; //PORT BUTTON 2
int bt3 = 26; //PORT BUTTON 3
int bt4 = 25; //PORT BUTTON 4

PZEM004Tv30 pzem(&Serial2);
int relay = 23;

float voltage, current, power, energy, frequency, pf;

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  Serial1.begin(BAUD, PROTOCOL, RXD1, TXD1);

  SerialBT.begin(bluetooth_name);

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  pinMode(bt1, INPUT_PULLUP);
  pinMode(bt2, INPUT_PULLUP);
  pinMode(bt3, INPUT_PULLUP);
  pinMode(bt4, INPUT_PULLUP);

  Serial1.println("*Loading,Loading,Loading,Loading,Loading,Loading#");

  preferences.begin("wifi_access", false);

  if (!init_wifi()) { // Connect to Wi-Fi fails
    SerialBT.register_callback(callback);
  } else {
    SerialBT.register_callback(callback_show_ip);
  }

  delay(2000);
}

void loop() {
  WifiBT();
  button();
  sensor();
  if (energy >= 10) koma = 2;
  else koma = 3;

  if (millis() > refreshLCD) {
    strTeg = String(voltage);
    strArus = String(current, 3);
    strPwr = String(power);
    strEnrgy = String(energy, koma);
    strFreq = String(frequency);
    strPf = String(pf);
    sprintf(kirim, "*%s V,%s A,%s W,%s kWh,%s Hz,%s#", strTeg, strArus, strPwr, strEnrgy, strFreq, strPf);
    Serial1.println(kirim);
    refreshLCD = millis() + 1000;
  }

  if (millis() > refreshWEB) {
    kirimWeb();
    refreshWEB = millis() + 20000;
  }
}

void sensor() {
  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();
}

bool init_wifi()
{
  String temp_pref_ssid = preferences.getString("pref_ssid");
  String temp_pref_pass = preferences.getString("pref_pass");
  pref_ssid = temp_pref_ssid.c_str();
  pref_pass = temp_pref_pass.c_str();

  Serial.println(pref_ssid);
  Serial.println(pref_pass);

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

  start_wifi_millis = millis();
  WiFi.begin(pref_ssid, pref_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi_millis > wifi_timeout) {
      WiFi.disconnect(true, true);
      return false;
    }
  }
  return true;
}

void scan_wifi_networks()
{
  WiFi.mode(WIFI_STA);
  // WiFi.scanNetworks will return the number of networks found
  int n =  WiFi.scanNetworks();
  if (n == 0) {
    SerialBT.println("no networks found");
  } else {
    SerialBT.println();
    SerialBT.print(n);
    SerialBT.println(" networks found");
    delay(1000);
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
      SerialBT.println(network_string);
    }
    wifi_stage = SCAN_COMPLETE;
  }
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{

  if (event == ESP_SPP_SRV_OPEN_EVT) {
    wifi_stage = SCAN_START;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == SCAN_COMPLETE) { // data from phone is SSID
    int client_wifi_ssid_id = SerialBT.readString().toInt();
    client_wifi_ssid = ssids_array[client_wifi_ssid_id];
    wifi_stage = SSID_ENTERED;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == WAIT_PASS) { // data from phone is password
    client_wifi_password = SerialBT.readString();
    client_wifi_password.trim();
    wifi_stage = PASS_ENTERED;
  }

}

void callback_show_ip(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    SerialBT.print("ESP32 IP: ");
    SerialBT.println(WiFi.localIP());  
    //    bluetooth_disconnect = true;
  }
}

void disconnect_bluetooth()
{
  delay(1000);
  Serial.println("BT stopping");
  SerialBT.println("Bluetooth disconnecting...");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("BT stopped");
  delay(1000);
  bluetooth_disconnect = false;
}

void WifiBT()
{
  if (millis() > refreshSWifi) {
  if (WiFi.status() != WL_CONNECTED && notif == 0) {
    SerialBT.println("DISCONNECT");
    notif = 1;
  }
  else if (WiFi.status() == WL_CONNECTED && notif == 1) {
    SerialBT.println("CONNECTED");
    notif = 0;
  }
    refreshSWifi = millis() + 1000;
  }


    if (bluetooth_disconnect)
    {
      disconnect_bluetooth();
    }

    switch (wifi_stage)
    {
      case SCAN_START:
        SerialBT.println("Scanning Wi-Fi networks");
        Serial.println("Scanning Wi-Fi networks");
        scan_wifi_networks();
        SerialBT.println("Please enter the number for your Wi-Fi");
        wifi_stage = SCAN_COMPLETE;
        break;

      case SSID_ENTERED:
        SerialBT.println("Please enter your Wi-Fi password");
        Serial.println("Please enter your Wi-Fi password");
        wifi_stage = WAIT_PASS;
        break;

      case PASS_ENTERED:
        SerialBT.println("Please wait for Wi-Fi connection...");
        Serial.println("Please wait for Wi_Fi connection...");
        wifi_stage = WAIT_CONNECT;
        preferences.putString("pref_ssid", client_wifi_ssid);
        preferences.putString("pref_pass", client_wifi_password);
        if (init_wifi()) { // Connected to WiFi
          connected_string = "ESP32 IP: ";
          connected_string = connected_string + WiFi.localIP().toString();
          SerialBT.println(connected_string);
          Serial.println(connected_string);
          //        bluetooth_disconnect = true;
        } else { // try again
          wifi_stage = LOGIN_FAILED;
        }
        break;

      case LOGIN_FAILED:
        SerialBT.println("Wi-Fi connection failed");
        Serial.println("Wi-Fi connection failed");
        delay(2000);
        wifi_stage = SCAN_START;
        break;
    }
  }
  
  void kirimWeb(){
  // set the fields with the values
  ThingSpeak.setField(1, voltage);
  ThingSpeak.setField(2, current);
  ThingSpeak.setField(3, power);
  ThingSpeak.setField(4, energy);
  ThingSpeak.setField(5, frequency);
  ThingSpeak.setField(6, pf);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

void button() {
  int nbt1 = digitalRead(bt1);
  int nbt2 = digitalRead(bt2);
  int nbt3 = digitalRead(bt3);
  int nbt4 = digitalRead(bt4);

//  Serial.print(nbt1);
//  Serial.print(nbt2);
//  Serial.print(nbt3);
//  Serial.println(nbt4);

if(nbt1 == 0) pzem.resetEnergy();
}
