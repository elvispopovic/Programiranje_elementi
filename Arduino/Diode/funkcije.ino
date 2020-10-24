#include "stog.h"

bool inicijalizirano = false;
Stog *stog = NULL;
byte *spremnik = NULL;
byte *pomocni = NULL;

void randomPomocni()
{
  byte i, t, r1, r2;
  for(i=0; i<brojDioda; i++)
    pomocni[i]=i;
  for(i=0; i<255; i++)
  {
    r1 = rand()%brojDioda;
    r2 = rand()%brojDioda;
    if(r2==r1) r2=(r1+1)%brojDioda;
    t = pomocni[r1];
    pomocni[r1]=pomocni[r2];
    pomocni[r2]=t;
  }
}

void inicijaliziraj()
{
  byte i;
  if(stog == NULL)
    stog = new Stog(brojDioda);
  if(spremnik == NULL)
    spremnik = new byte[brojDioda];
  if(pomocni == NULL)
    pomocni = new byte[brojDioda];
  for(i=0; i<brojDioda; i++)
    spremnik[i]=pomocni[i]=0;
  posalji(100);
  srand(millis());
  rand();
  rand();
  randomPomocni();
  inicijalizirano = true;
}

void obrisiSpremnik()
{
  for(byte i=0; i<brojDioda; i++)
    spremnik[i]=0;
}

/* digitalni update - diode mogu biti samo upaljene i ugasene */
void posalji(int d, bool obrnuto)
{
  byte i;
  digitalWrite(pinLatch, LOW);
  for(i=0; i<brojDioda; i++)
  {
    digitalWrite(pinClock, LOW);
    digitalWrite(pinSerial, 
      spremnik[obrnuto?brojDioda-1-i:i]>0?HIGH:LOW);
    digitalWrite(pinClock, HIGH);
  }
  digitalWrite(pinClock, LOW);
  digitalWrite(pinLatch, HIGH);
  digitalWrite(pinLatch, LOW);
  delay(d);
}

/* PWM update - diode mogu biti upaljene odredjenim intenzitetom */
void posalji2(short nd, bool obrnuto)
{
  short i, j, k;
  byte stanje;
  for(k=0; k<nd; k++)
  {
    for(j=0; j<32; j++)
    {
      digitalWrite(pinLatch, LOW);
      for(i=0; i<brojDioda; i++)
      {
        stanje = spremnik[obrnuto?brojDioda-1-i:i];
        if(j<stanje)
        {
          digitalWrite(pinClock, LOW);
          digitalWrite(pinSerial, HIGH);
          digitalWrite(pinClock, HIGH);
          if(i==0)
            digitalWrite(pinTest, HIGH);
        }
        else
        {
          digitalWrite(pinClock, LOW);
          digitalWrite(pinSerial, LOW);
          digitalWrite(pinClock, HIGH);
          if(i==0)
            digitalWrite(pinTest, LOW);
        }
      }
      digitalWrite(pinClock, LOW);
      digitalWrite(pinLatch, HIGH);
    }
    
  }
  digitalWrite(pinLatch, LOW);
}

void pomakni(byte p)
{
  short i, j;
  if(p >= brojDioda)
    return;
  for(i=brojDioda-1, j=brojDioda-1-p; i>=0; i--, j--)
    spremnik[i] = (j>=0)?spremnik[j]:0;
}


void segment(int d, byte j, byte n, bool obrnuto)
{
  byte i, k, c;
  if(!inicijalizirano)
    return;
  if(n<1)
    n=1;
  if(n>brojDioda)
    n=brojDioda;
  if(j>brojDioda+n)
    j = brojDioda+n;
  for(i=n>=j?0:j-n, c=0; i<=(j<(brojDioda-1)?j:brojDioda-1); i++, c++)
    spremnik[i]=(c==0&&j>=n)?0:1;
  if(i<brojDioda-1)
    spremnik[i+1]=0;
  posalji(d, obrnuto);
}

void altSegment(byte j, byte n, bool obrnuto)
{
  byte i, k, c;
  if(!inicijalizirano)
    return;
  if(n<1)
    n=1;
  if(n>brojDioda)
    n=brojDioda;
  if(j>brojDioda+n)
    j = brojDioda+n;
  for(i=(n>=j?j%2:j-n/2*2), c=0; i<=(j<(brojDioda-1)?j:brojDioda-1); i+=2, c+=2)
    spremnik[obrnuto?brojDioda-1-i:i]=(c==0&&j>=n)?0:1;
  if(i<brojDioda-2)
    spremnik[obrnuto?brojDioda-3-i:i+2]=0;
}

void prolaz(int d, byte n, bool obrnuto)
{
  for(byte i=0; i<=brojDioda+n; i++)
    segment(d,i,n,obrnuto);
}

void glitch(int d1, int d2, byte polozaj, byte amplituda, byte ponavljanja, byte n, bool obrnuto)
{
  byte i, j, l;
  if(polozaj>=brojDioda)
    polozaj=brojDioda-1;
  if(amplituda>polozaj)
    amplituda=polozaj;
  l=polozaj-amplituda;
  for(i=0; i<polozaj; i++)
    segment(d1,i,n,obrnuto);
  for(j=0; j<ponavljanja; j++)
  {
    for(;i>l;i--)
      segment(d2,i,n,obrnuto);
    for(;i<polozaj;i++)
      segment(d2,i,n,obrnuto);
  }
  for(;i<brojDioda+n; i++)
    segment(d1,i,n,obrnuto);
}

void zavjesa(int d, bool periferno, bool obrnuto)
{
  byte i, j, h;
  if(!inicijalizirano)
    return;
  h=brojDioda/2;
  if(!obrnuto)
    for(j=0; j<=h; j++)
    {
      for(i=0; i<h; i++)
        if(periferno)
          spremnik[h-1-i]=spremnik[h+i]=i<j?1:0;
        else
          spremnik[brojDioda-1-i]=spremnik[i]=i<j?1:0;
      posalji(d, false);
    }
  else
    for(j=h+1; j>0; j--)
    {
      for(i=0; i<h; i++)
        if(periferno)
          spremnik[h-1-i]=spremnik[h+i]=i<j-1?1:0;
        else
          spremnik[brojDioda-1-i]=spremnik[i]=i<j-1?1:0;
      posalji(d, false);
    }  
}

void policija(int d, byte n)
{
  byte i, j, k;
  if(!inicijalizirano)
    return;
  byte h=brojDioda/2;
  for(k=0; k<2; k++)
    for(j=0; j<n; j++)
    {
      for(i=0; i<brojDioda; i++)
        spremnik[i]=i<h?k:1-k;
      posalji(d, false);
      for(i=0; i<brojDioda; i++)
        spremnik[i]=0;
      posalji(d, false);
    }
}

void slucajno(int d, bool brisanje)
{
  byte i;
  if(!inicijalizirano)
    return;
  for(i=0; i<brojDioda; i++)
  {
    spremnik[pomocni[i]]=brisanje?0:1;
    posalji(d, false);
  }
}

void slucajno2(int d)
{
  byte i;
  if(!inicijalizirano)
    return;
  for(i=0; i<brojDioda; i++)
  {
    spremnik[pomocni[i]]=1;
    posalji(d, false);
    spremnik[pomocni[i]]=0;
    posalji(d, false);
  }
}

void ispreplitanje(int d, byte n)
{
  if(!inicijalizirano)
    return;
  for(byte i=0; i<=brojDioda+n; i+=2)
  {
    altSegment(i,n,false);
    altSegment(i,n,true);
    posalji(d, false);
  }
}

void trcece(int d, bool obrnuto)
{
  byte i, j;
  if(!inicijalizirano)
    return;
  for(j=0; j<3; j++)
  {
    for(i=0; i<brojDioda; i++)
      spremnik[i]=(i+j)%3==0?1:0;
    posalji(d, obrnuto);
  }
}

void sirinaPulsa(int d, byte b, bool obrnuto)
{
  byte i,j;
  if(!inicijalizirano)
    return;
  for(j=0; j<4; j++)
  {
    for(i=0; i<brojDioda; i+=4)
      spremnik[i+j]=b;
    posalji(d, obrnuto);
  }
}

void fibonacci(int d, byte razmak, bool obrnuto)
{
  unsigned int a=1, b=0, t;
  byte i, j;
  if(!inicijalizirano)
    return;
  pomakni(16);
  for(j=0; j<8; j++)
  {
    for(i=0; i<a; i++)
    {
      pomakni(1);
      spremnik[0]=1;
      posalji(d, obrnuto);
    }
    for(i=0; i<razmak; i++)
    {
      pomakni(1);
      posalji(d, obrnuto);
    }
    t=a+b;
    b=a;
    a=t;
  }
  for(i=0; i<brojDioda-razmak; i++)
  {
    pomakni(1);
    posalji(d, obrnuto);
  }
}

void oslabljeno(byte intenzitet, short trajanje)
{
  byte i;
  short j;
  if(!inicijalizirano)
    return;
  for(i=0; i<brojDioda; i++)
    spremnik[i]=intenzitet;
  posalji2(trajanje, false);
}

void fade(bool obrnuto)
{
  short i;
  if(!inicijalizirano)
    return;
  if(!obrnuto)
    for(i=0; i<32; i++)
      oslabljeno(i,10);
  else
    for(i=31; i>=0; i--)
      oslabljeno(i,10);
}

void trag(short trajanje, bool obrnuto)
{
  byte i, intenzitet, opadanje;
  if(!inicijalizirano)
    return;
  intenzitet=1<<5;
  for(i=0; i<6; i++, intenzitet>>=1)
  {
    pomakni(1);
    spremnik[0]=intenzitet;
    posalji2(trajanje, obrnuto);
  }
  for(i=0; i<brojDioda; i++)
  {
    pomakni(1);
    spremnik[0]=0;
    posalji2(trajanje, obrnuto);
  }
}

void trag2(short trajanje, bool obrnuto)
{
  byte i, intenzitet, opadanje;
  if(!inicijalizirano)
    return;
  intenzitet=32;
  opadanje = intenzitet/brojDioda;
  for(i=0; i<32; i++, intenzitet*=0.95)
  {
    pomakni(1);
    spremnik[0]=intenzitet;
    posalji2(trajanje, obrnuto);
  }
  for(i=0; i<brojDioda; i++)
  {
    pomakni(1);
    spremnik[0]=0;
    posalji2(trajanje, obrnuto);
  }
}
