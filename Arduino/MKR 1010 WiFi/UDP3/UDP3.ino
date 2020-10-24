#include <SPI.h>
#include <ArduinoECCX08.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "Pristupna.h"

char testAdresa[20] = "";
int testPort = 0;

bool dozvoliRandom = true;
char primanje[513], verzija[64], slanje[64], slucajna[12];

unsigned int localPort = PORT;
int diodaPin = 6;  //MKR1010
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiUDP Udp;
int brojac;

void odgovori(const char* odgovor);
void stringCommand(char *unos);
void sendTest();
void printWifiStatus();
void diodeDelay();
void diodeBlink();

void setup()
{
  int status;
  Serial.begin(9600);

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Nema WiFi-a.");
    while(true)
      delay(1000);
  }
  delay(1000);
  strncpy(verzija,WiFi.firmwareVersion(),32);
  Serial.print("WiFI firmware: ");
  Serial.println(verzija);
  if(strcmp(verzija,WIFI_FIRMWARE_LATEST_VERSION)<0)
    Serial.print("(zastarjela verzija)");

  if (!ECCX08.begin())
  {
    Serial.println("ECC chip nije prisutan.");
    dozvoliRandom = false;
  }
  else
  {
    strncpy(verzija,ECCX08.serialNumber().c_str(),32);
    Serial.print("ECC chip serijski broj: ");
    Serial.println(verzija);

    if(!ECCX08.locked())
    {
      Serial.println("Chip nije zakljucan.");
      dozvoliRandom = false;
    }
  }
  do
  {
    digitalWrite(diodaPin, HIGH);
    status = WiFi.begin(ssid, pass);
    Serial.print("Pokusavam se spojiti na SSID: ");
    Serial.println(ssid);
    diodeDelay(10);
  }while(status != WL_CONNECTED);
  Udp.begin(localPort);
  printWifiStatus();
 brojac = 0; 
}

void loop()
{
  int velicinaPaketa, velicinaPoruke;
  velicinaPaketa = Udp.parsePacket();
  
  if(velicinaPaketa > 0)
  {
    diodeBlink();
    Serial.print("Primljen je paket velicine ");
    Serial.print(velicinaPaketa);
    Serial.print(" od ");
    Serial.print(Udp.remoteIP());
    Serial.print( ":");
    Serial.println(Udp.remotePort());
    velicinaPoruke = Udp.read(primanje,512);
    if(velicinaPoruke > 0)
      primanje[velicinaPoruke]='\0';
    Serial.print("Sadrzaj:");
    Serial.println(primanje);
    odgovori("Primljeno. O.K.");
    stringCommand(primanje);
  }
  delay(500);
  if((brojac++) == 10)
  {
    sendTest();
    brojac = 0;
  }
}

void stringCommand(char *unos)
{
  char* token;
  token=strtok(unos," ");
  Serial.print("Komanda: ");
  Serial.println(token);
  if(strcmp(token,"addr")==0)
  {
    strncpy(testAdresa,strtok(0,":"),15);
    testPort = atoi(strtok(0," "));
    Serial.print("IP adresa za slanje: ");
    Serial.print(testAdresa);
    Serial.print(":");
    Serial.println(testPort);
  }
  else if(strcmp(token,"stop")==0)
  {
    testAdresa[0]='\0';
  }
}

void odgovori(const char* odgovor)
{
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(odgovor);
  Udp.endPacket();
}

void sendTest()
{
  static int brojac = 0;
  if(strcmp(testAdresa,"")==0 || testPort==0)
    return;
  Serial.print("Test slanja prema ");
  Serial.print(testAdresa);
  Serial.print(":");
  Serial.print(testPort);
  Serial.print(" ---> ");
  Udp.beginPacket(testAdresa, testPort);
  if(dozvoliRandom)
  {
    itoa(ECCX08.random(65535), slucajna, 10);
    strcpy(slanje,"RND: ");
  }
  else
  {
    itoa(brojac++, slucajna, 10);
    strcpy(slanje,"TEST: ");
  }
  strcat(slanje,slucajna);
  Udp.write(slanje);
  Udp.endPacket();
  Serial.println(slanje);
}

void printWifiStatus() 
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Addresa: ");
  Serial.print(ip);
  Serial.print(":");
  Serial.println(localPort);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Jakost signala (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
void diodeDelay(int sekundi)
{
  int i;
  pinMode(diodaPin, OUTPUT);
  for(i=0; i<sekundi; i++)
  {
    digitalWrite(diodaPin,HIGH);
    delay(500);
    digitalWrite(diodaPin,LOW);
    delay(500);
  }
  pinMode(diodaPin, INPUT);
}

void diodeBlink()
{
  int i;
  pinMode(diodaPin, OUTPUT);
  for(i=0; i<3; i++)
  {
    digitalWrite(diodaPin, HIGH);
    delay(40);
    digitalWrite(diodaPin, LOW);
    delay(40);
  }
  pinMode(diodaPin, INPUT);
}
