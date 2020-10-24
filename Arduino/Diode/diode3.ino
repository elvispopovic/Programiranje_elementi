#include "stog.h"

int pinLatch = 8;
int pinClock = 12;
int pinSerial = 11;
int pinTest = 13; //on board dioda
int brojDioda = 16;

void posalji(int d, bool obrnuto = false);
void prolaz(int d, byte n, bool obrnuto = false);
void glitch(int d, byte polozaj, byte amplituda, byte ponavljanja, byte n, bool obrnuto=false);
void zavjesa(int d1, int d2, bool periferno, bool obrnuto = false);
void policija(int d, byte n);
void slucajno(int d, bool brisanje = false);
void fade(bool obrnuto = false);
void ispreplitanje(int d, byte n = 4);
void fibonacci(int d, bool obrnuto = false);
void sirinaPulsa(int d, byte b, bool obrnuto = false);

void f1()
{
  byte i, j;
  Serial.println(" f1");
  for(i=0; i<30; i++)
    prolaz(30-i,i/3,i%2);
}

void f2()
{
  byte i, trajanje;
  Serial.println(" f2");
  for(i=0, trajanje=16; i<4; i++, trajanje>>=1)
  {
    trag(trajanje,false);
    trag(trajanje,true);
  }
}

void f3()
{
  byte i;
  Serial.println(" f3");
  for(i=0; i<10; i++)
    policija(30,5);
}
void f4()
{
  Serial.println(" f4");
  zavjesa(50,false,false);
  zavjesa(50,true,true);
  zavjesa(50,true,false);
  zavjesa(50,false,true);
  zavjesa(50,false,false);
  zavjesa(50,false,true);
}
void f5()
{
  byte i;
  Serial.println(" f5");
  for(i=0; i<3; i++)
    glitch(40, 10, 14, 8, 3, 3, i%2);
}
void f6()
{
  byte i;
  Serial.println(" f6");
  slucajno(40, false);
  zavjesa(50,false,true);
  zavjesa(50,false,false);
  slucajno(40, true);
  for(i=0; i<5; i++)
    slucajno2(40);
}

void f7()
{
  byte i;
  Serial.println(" f7");
  fade(false);
  oslabljeno(255,10);
  fade(true);
  posalji(1000);
}

void f8()
{
  byte i;
  Serial.println(" f8");
  for(i=0; i<20; i++)
  {
    ispreplitanje(100-5*i,4);
    delay(1000-50*i);
  }
  for(i=0; i<20; i++)
  {
    ispreplitanje(10);
    delay(100);
  }
}

void f9()
{
  int i, j;
  Serial.println(" f9");
  for(j=0; j<2; j++)
    for(i=0; i<5; i++)
      prolaz(35,brojDioda,j%2==0?true:false);
}

void f10()
{
  byte i, j, trajanje;
  Serial.println(" f2");
  for(j=0; j<2; j++)
  {
    for(i=0, trajanje=8; i<3; i++, trajanje>>=1)
      trag2(trajanje,j==1?false:true);
    for(i=0; i<4; i++)
      trag2(trajanje,j==1?false:true);
  }
}

void f11()
{
  byte i;
  Serial.println(" f11");
  for(i=0; i<20; i++)
    trcece(90,true);
  zavjesa(50,false,false);
  zavjesa(50,true,true);
  for(i=0; i<20; i++)
    trcece(90,false);
  zavjesa(50,false,false);
  zavjesa(50,true,true);
}

void f12()
{
  byte i;
  int d;
  Serial.println(" f12");
  obrisiSpremnik();
  for(i=1, d=800; i<=10; i++)
  {
    d/=2;
    sirinaPulsa(d,1);
    sirinaPulsa(d,0);
    sirinaPulsa(d,1, true);
    sirinaPulsa(d,0, true);
  }
}

void f13()
{
  byte i, j;
  Serial.println(" f13");
  for(j=0; j<2; j++)
  {
    for(i=0; i<8; i++)
    {
      pomakni(2);
      posalji(100, j>0?true:false);
    }
    fibonacci(250, 3, j>0?true:false);
  }
}

void (*func_ptr[])() = {f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13};

void setup() 
{
  byte i;
  Serial.begin(9600);
  delay(100);
  inicijaliziraj();
  pinMode(pinLatch, OUTPUT);
  pinMode(pinClock, OUTPUT);
  pinMode(pinSerial, OUTPUT);
  pinMode(pinTest, OUTPUT);
  digitalWrite(pinLatch, LOW);
  digitalWrite(pinClock, LOW);
  digitalWrite(pinLatch, LOW);
  digitalWrite(pinTest, LOW);
  
  Serial.println(" Program pokrenut.");
  for(i=0; i<2; i++)
    slucajno2(40);

//testiranje
  trag(8,false);
  delay(1000);
  obrisiSpremnik();
  posalji(100);
}

void loop() 
{
  int i;
  for(i=0; i<13; i++)
  {
    (*func_ptr[i])();
    delay(100);
  }
}
