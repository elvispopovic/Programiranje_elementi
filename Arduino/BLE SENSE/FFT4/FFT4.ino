/*
*  Posebno prilagodjeno Arduino Nano 33 BLE Sense
*  Uzima signal sa mikrofona, i obavlja diskretne Fourierove transformacije (realno u kompleksno)
*  Prikazuje realni ulaz (signal sa mikrofona), te realni i imaginarni dio kompleksnog izlaza
*  u Serial plotteru
*  Uz to mjeri i vrijeme obavljanja transformacija (oko 1 ms za BLE Sense s ARM Cortex M4F jezgrama)
*  DFT nisu posebno optimizirane za realno->kompleksno racunanje zbog jednostavnosti programa
*  Jezgru programa cini DFT kompleksno->kompleksno uz dodane DCT i DFT u oba smjera (IDCT i IDST)
*/

#include <PDM.h>
#define BITOVA 8 //za Uno 4 je maksimalno, za MKR1010 može do 8, za nano 33 BLE može i do 10

/* za testiranje, 4 bita, uzorak 16 */
const PROGMEM float dctTest[] = {1.00000, 6.47410, 0.00000, 0.70977, 0.00000, 0.24805, 0.00000, 0.12005, 0.00000, 0.06635, 0.00000, 0.03788, 0.00000, 0.01981, 0.00000, 0.00619};
const PROGMEM float dstTest[] = {0.63764, 5.12583, 0.21531, 2.61313, 0.13258, 1.79995, 0.09852, 1.41421, 0.08085, 1.20269, 0.07087, 1.08239, 0.06531, 1.01959, 0.06280, 1.00000};
const PROGMEM float dftTestRe[] = {0.50000, -2.01367, -0.70711, -0.24830, -0.00000, 0.16591, 0.29289, 0.40054, 0.50000, 0.59946, 0.70711, 0.83409, 1.00000, 1.24830, 1.70711, 3.01367};
const PROGMEM float dftTestIm[] = {-0.50000, -3.01367, -1.70711, -1.24830, -1.00000, -0.83409, -0.70711, -0.59946, -0.50000, -0.40054, -0.29289, -0.16591, -0.00000, 0.24830, 0.70711, 2.01367};

int analogPin1 = A3, analogPin2 = A4;

byte bitova;
word uzoraka, uzoraka2;
float norma, smjer;

char ispis[64];
short *spremnikUzorkovanja;
volatile int procitano;

float *ulazTestRe, *ulazTestIm, *izlazTestRe, *izlazTestIm;
float *FFTulazRe, *FFTulazIm, *FFTizlazRe, *FFTizlazIm;
word *redoslijed;
float *rampKosinusi, *rampSinusi;
float *Kosinusi, *Sinusi;
  
void setup() 
{
  Serial.begin(19200);
  while (!Serial);

  bitova = BITOVA;
  uzoraka = 1<<bitova;
  uzoraka2 = uzoraka<<1;

  spremnikUzorkovanja = new short[uzoraka];
  /* konstantni testni parametri, zelimo ih nepromjenjive za razne funkcije */
  ulazTestRe = new float[uzoraka];
  ulazTestIm = new float[uzoraka];
  izlazTestRe = new float[uzoraka];
  izlazTestIm = new float[uzoraka];

  /* promjenjivi ulazni parametri za DCT, DST, FFT - funkcije ih mogu mijenjati */
  FFTulazRe        = new float[uzoraka2];
  FFTulazIm        = new float[uzoraka2];
  FFTizlazRe       = new float[uzoraka2];
  FFTizlazIm       = new float[uzoraka2];

  /* konstantne vrijednosti, ne mijenjaju se */
  redoslijed = new word[uzoraka2];
  Kosinusi   = new float[uzoraka2];
  Sinusi     = new float[uzoraka2];
  rampKosinusi  = new float[uzoraka];
  rampSinusi    = new float[uzoraka];

  /* testiranje, privremeno sve postavljamo na 4 bita 
  *  Ako je alocirana memorija uz premalo bita, testiranje se nece provesti
  */
  if(bitova>=4)
  {
    bitova = 4;
    uzoraka = 1<<bitova;
    uzoraka2 = uzoraka<<1;
    inicijalizacija(1);
    test();
  }
  else
  {
    Serial.print("Odredjeno je premalo bitova za testiranje: "); Serial.println(bitova);
  }

  /* vracamo natrag na potreban broj bita i finalno inicijaliziramo */
  bitova = BITOVA;
  uzoraka = 1<<bitova;
  uzoraka2 = uzoraka<<1;
  inicijalizacija(1);
  PDM.onReceive(onPDMdata);
  PDM.setGain(30);
  if (!PDM.begin(1, 16000)) 
  {
    Serial.println("Failed to start PDM!");
    while (1);
  }
}

void loop()
{
  word i;  
  float norma = 1.0/uzoraka;
  if(procitano > 0)
  {
    if(procitano == (1<<bitova))
    {
      for(i=0; i<uzoraka; i++)
      {
        ulazTestRe[i]=spremnikUzorkovanja[i]*norma;
        ulazTestIm[i]=0.0;
      }
      DFT(ulazTestRe, ulazTestIm, izlazTestRe, izlazTestIm);
      
      for(i=0; i<uzoraka/2; i++)
      {
        izlazTestRe[i]*=norma;
        izlazTestIm[i]*=norma;
        Serial.print("Uzorak:");
        Serial.print(ulazTestRe[i*2]-5.0); Serial.print(" ");
        Serial.print("DFT_Re:");
        Serial.print(izlazTestRe[i]*3.0); Serial.print(" ");
        Serial.print("DFT_Im:");
        Serial.println(izlazTestIm[i]*3.0+5.0);
      }
    }

    procitano = 0;
  }
  

}

void onPDMdata()
{
  int spremno = PDM.available();
  PDM.read(spremnikUzorkovanja, spremno);
  procitano = spremno/2;
}


bool inicijalizacija(byte smjer)
{
  word i;
  float di, pidi, invUz1, invUz2;
  invUz1 = 1.0/uzoraka;
  invUz2 = 1.0/(2.0*uzoraka);

  norma = invUz2;
  promijeniSmjer(smjer);
  
  for(i=0, di=0; i<uzoraka; i++, di+=1.0)
  {
    pidi = M_PI*di;
    rampKosinusi[i]  = cos(pidi*invUz2);
    rampSinusi[i]    = sin(pidi*invUz2);

  }
  for(i=0, di=0.0; i<(uzoraka<<1); i++, di+=1.0)
  {
    pidi = M_PI*di;
    redoslijed[i] = zrcaljenjebitova(i, bitova+1);
    Kosinusi[i]   = cos(pidi*invUz1);
    Sinusi[i]     = sin(pidi*invUz1);
  }
  
  return true;
}

void DFT(float rex[], float imx[], float ref[], float imf[])
{
  FFT(rex,imx,ref,imf,2);
}

void DST(float x[], float f[])
{
  word i,j;
  if(smjer==1)
    for(i=0, j=uzoraka-1; i<uzoraka; i++,j--)
    {   //pripremamo ulazne podatke za dst
        FFTulazRe[i]=FFTulazIm[i]=x[i];
        FFTulazRe[i+uzoraka]=FFTulazIm[i+uzoraka]=-x[j];
    }
  else
  {
      //pripremamo ulazne podatke za idst
      FFTulazRe[0]=FFTulazIm[0]=0.0;
      FFTulazRe[uzoraka]=FFTulazIm[uzoraka]=x[uzoraka-1];
      for(i=1, j=(uzoraka<<1)-1; i<uzoraka; i++, j--)
      {
          //mnozenje sa inverznom faznom rampom da se dobiju podaci za idct
          FFTulazRe[i]=FFTulazIm[j]= x[i-1]*(rampKosinusi[i]+rampSinusi[i]);
          FFTulazIm[i]=FFTulazRe[j]=-x[i-1]*(rampKosinusi[i]-rampSinusi[i]);
      }
  }
  FFT(FFTulazRe,FFTulazIm,FFTizlazRe,FFTizlazIm,1);
  if(smjer==1)
  {
      f[uzoraka-1]=FFTizlazRe[uzoraka]*rampKosinusi[0]+FFTizlazIm[uzoraka]*rampSinusi[0];
      for(i=0; i<uzoraka-1; i++) //mnozenje sa faznom rampom da se dobije izlaz
          f[i]=FFTizlazRe[i+1]*rampKosinusi[(i+1)]+FFTizlazIm[i+1]*rampSinusi[(i+1)];
  }
  else
      for(i=0; i<uzoraka; i++) //uzimamo samo prve potrebne elemente
          f[i]=FFTizlazRe[i];
}

void DCT(float x[], float f[])
{
  word i,j;
  if(smjer==1)
  for(i=0, j=uzoraka-1; i<uzoraka; i++,j--)
  {   //pripremamo ulazne podatke za dct
      FFTulazRe[i]=FFTulazIm[i]=x[i];
      FFTulazRe[i+uzoraka]=FFTulazIm[i+uzoraka]=x[j];
  }
  else
  {
      //pripremamo ulazne podatke za idct
      FFTulazRe[0]=FFTulazIm[0]=x[0];
      FFTulazRe[uzoraka]=FFTulazIm[uzoraka]=0.0;
      for(i=1, j=uzoraka2-1; i<uzoraka; i++, j--)
      {
          //mnozenje sa inverznom faznom rampom da se dobiju podaci za idct
          FFTulazRe[i]=FFTulazIm[j]=x[i]*(rampKosinusi[i]-rampSinusi[i]);
          FFTulazIm[i]=FFTulazRe[j]=x[i]*(rampKosinusi[i]+rampSinusi[i]);
      }
  }
  //izvodjenje diskretnih fourierovih transformacija (ovisno o smjeru dft ili idft
  FFT(FFTulazRe,FFTulazIm,FFTizlazRe,FFTizlazIm,1);

  if(smjer==1)
    for(i=0; i<uzoraka; i++) //mnozenje sa faznom rampom da se dobije izlaz
        f[i]=FFTizlazRe[i]*rampKosinusi[i]+FFTizlazIm[i]*rampSinusi[i];
  else
    for(i=0; i<uzoraka; i++) //uzimamo samo prve potrebne elemente
        f[i]=FFTizlazRe[i];
}

void promijeniSmjer(short s)
{
    smjer = s>=0?1.0:-1.0;
}

void normiranje(float rex[], float imx[], byte skok)
{
  word i;
  for(i=0; i<uzoraka; i++)
  {
      rex[i]*=(norma*skok);
      imx[i]*=(norma*skok);
  }
}

void FFT(float *rex, float *imx, float *ref, float *imf, byte skok)
{
  word uz;
  word broj_grupa;
  word sirina;
  word polusirina;
  word grupa, sirgrupa;
  word element, polusirelement;
  word trig_indeks, trig_indeks2;
  word i1, i2;
  float cosref, cosimf, sinref, sinimf, temp;
  float reElement, imElement, reKopija, imKopija;
  uz = uzoraka2/skok;
  broj_grupa=uz>>1;
  sirina=2;
  polusirina=1;
  /* racunamo naopako, od zadnjeg prema prvom unutar svake grupe, okrecu se svi osim prvog elementa
   *  prvo pomicemo sve a onda cirkularno okrenemo za jedan
   *  Prva faza: najmanje grupe od po dva elementa
   */
  temp=uz-1;
  for(grupa=0,sirgrupa=0, polusirelement=polusirina; grupa < broj_grupa;
        grupa++,sirgrupa+=sirina,polusirelement+=sirina)
    {
        i1=redoslijed[sirgrupa*skok];
        i2=redoslijed[polusirelement*skok];

        reElement=rex[i1]+rex[i2];
        imElement=imx[i1]+imx[i2];
        reKopija=rex[i1]-rex[i2];
        imKopija=imx[i1]-imx[i2];

        //okrecemo naopako
        i1=temp-redoslijed[i1*skok];
        i2=temp-redoslijed[i2*skok];
        ref[i1]=reElement;
        imf[i1]=imElement;
        ref[i2]=reKopija;
        imf[i2]=imKopija;
    }
    /* 
     *  Glavna faza - postupno sirenje grupa (dupliranje) i smanjivanje njihovog broja
     */
    sirina=4;
    polusirina=2;
    while(sirina<=uz)
    {
        broj_grupa>>=1;
        for(grupa=0, sirgrupa=0; grupa < broj_grupa; grupa++,sirgrupa+=sirina)
        {   //idemo u obrnutom smjeru unutar grupa
            for(element=sirgrupa+sirina-1, trig_indeks=0, polusirelement=sirgrupa+polusirina-1;
                element>=sirgrupa+polusirina; element--, trig_indeks+=broj_grupa, polusirelement--)
            {
                trig_indeks2 = trig_indeks*skok;
                cosref = ref[polusirelement]*Kosinusi[trig_indeks2];
                sinref = smjer*ref[polusirelement]*Sinusi[trig_indeks2];
                cosimf = imf[polusirelement]*Kosinusi[trig_indeks2];
                sinimf = smjer*imf[polusirelement]*Sinusi[trig_indeks2];
                ref[polusirelement]  = ref[element]-cosref+sinimf;
                imf[polusirelement]  = imf[element]-cosimf-sinref;
                ref[element]+=cosref-sinimf;
                imf[element]+=cosimf+sinref;
            }
        }
        sirina <<=1;
        polusirina <<=1;
    }
    /*
     * Cirkularno pomicanje udesno
     */
    temp=ref[uz-1];
    for(element=uz-1; element>0; element--)
        ref[element]=ref[element-1];
    ref[0]=temp;
    temp=imf[uz-1];
    for(element=uz-1; element>0; element--)
        imf[element]=imf[element-1];
    imf[0]=temp;
}

/* FFT scramble metoda, izbjegava se potreba za rekurzijama */
int zrcaljenjebitova(int ulaz, byte bitova)
{
  word i, postavljanje, maska, rezultat;
  for(i=0, rezultat=0, maska=1, postavljanje = 1<<(bitova-1); i<bitova; i++, maska <<=1, postavljanje>>=1)
    if(maska & ulaz)
      rezultat |= postavljanje;
  return rezultat;
}

/* pomocna funkcija */
char* ispisbitova(int ulaz, byte bitova)
{
  static char rezultat[17];
  byte i;
  for(i=0; i<bitova; i++)
    rezultat[i] = ulaz & (1<<(bitova-1-i))?'1':'0';
  rezultat[i]='\0';
  return rezultat;
}


void test()
{
  word i;
  float t;

  /* punimo parametre */
  for(i=0; i<16; i++)
    ulazTestRe[i]=0.5-1.0/uzoraka*i;
  /* testiramo DCT */
  promijeniSmjer(1);
  DCT(ulazTestRe, izlazTestRe);
  for(i=0; i<16; i++)
  {
    t = pgm_read_float(dctTest+i);
    if(abs(izlazTestRe[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na DCT "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestRe: "); Serial.print(izlazTestRe[i],5); Serial.print(", dctTest: "); Serial.println(t,5);
      return;
    }
  }
  /* testiramo IDCT */
  for(i=0; i<16; i++)
    ulazTestRe[i]=pgm_read_float(dctTest+i);
  promijeniSmjer(-1);
  DCT(ulazTestRe, izlazTestRe);
  normiranje(izlazTestRe, izlazTestIm, 1);
  for(i=0; i<16; i++)
  {
    t = 0.5-1.0/uzoraka*i;
    if(abs(izlazTestRe[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na IDCT "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestRe: "); Serial.print(izlazTestRe[i],5); Serial.print(", dctTest: "); Serial.println(t,5);
      return;
    }
  }
  /* testiramo DST */
  for(i=0; i<16; i++)
    ulazTestRe[i]=0.5-1.0/uzoraka*i;
  promijeniSmjer(1);
  DST(ulazTestRe, izlazTestRe);
  for(i=0; i<16; i++)
  {
    t = pgm_read_float(dstTest+i);
    if(abs(izlazTestRe[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na DST "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestRe: "); Serial.print(izlazTestRe[i],5); Serial.print(", dstTest: "); Serial.println(t,5);
      return;
    }
  }
  /* testiramo IDST */
  for(i=0; i<16; i++)
    ulazTestRe[i]=pgm_read_float(dstTest+i);
  promijeniSmjer(-1);
  DST(ulazTestRe, izlazTestRe);
  normiranje(izlazTestRe, izlazTestIm, 1);
  for(i=0; i<16; i++)
  {
    t = 0.5-1.0/uzoraka*i;
    if(abs(izlazTestRe[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na IDST "); Serial.print(i); Serial.println("/15");
      Serial.print("ulazTestRe: "); Serial.print(izlazTestRe[i],5); Serial.print(", idstTest: "); Serial.println(t,5);
      return;
    }
  }
  /* testiramo DFT */
  for(i=0; i<16; i++)
  {
    ulazTestRe[i]=0.5-1.0/uzoraka*i;
    ulazTestIm[i]=-0.5+1.0/uzoraka*i;
  }
  promijeniSmjer(1);
  DFT(ulazTestRe, ulazTestIm, izlazTestRe, izlazTestIm);
  for(i=0; i<16; i++)
  {
    t = pgm_read_float(dftTestRe+i);
    if(abs(izlazTestRe[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na DFT, realni dio "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestRe: "); Serial.print(izlazTestRe[i],5); Serial.print(", dftTestRe: "); Serial.println(t,5);
      return;
    }
    t = pgm_read_float(dftTestIm+i);
    if(abs(izlazTestIm[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na DFT, imaginarni dio "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestIm: "); Serial.print(izlazTestIm[i],5); Serial.print(", dftTestIm: "); Serial.println(t,5);
      return;
    }
  }
  /* testiramo IDFT */
  for(i=0; i<16; i++)
  {
    ulazTestRe[i]=pgm_read_float(dftTestRe+i);
    ulazTestIm[i]=pgm_read_float(dftTestIm+i);
  }
  promijeniSmjer(-1);
  DFT(ulazTestRe, ulazTestIm, izlazTestRe, izlazTestIm);
  normiranje(izlazTestRe, izlazTestIm, 2);
  for(i=0; i<16; i++)
  {
    t = 0.5-1.0/uzoraka*i;
    if(abs(izlazTestRe[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na IDFT, realni dio "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestRe: "); Serial.print(izlazTestRe[i],5); Serial.print(", idftTestRe: "); Serial.println(t,5);
      return;
    }
    t = -0.5+1.0/uzoraka*i;
    if(abs(izlazTestIm[i]-t)>0.0001)
    {
      Serial.print("Test je podbacio na IDFT, imaginarni dio "); Serial.print(i); Serial.println("/15");
      Serial.print("izlazTestIm: "); Serial.print(izlazTestIm[i],5); Serial.print(", idftTestIm: "); Serial.println(t,5);
      return;
    }
  }
  Serial.println("TEST O.K.");
  delay(2000);
}
