const PROGMEM int velicina = 500;
int *polje;
char *ispis;
unsigned int dubina, maxDubina, brZamjena;
unsigned long pocetak, kraj;

void Zamjena(int *a, int *b)
{
  int t = *a;
  *a = *b;
  *b = t;
  brZamjena++;
}

void MedianTrojke(int Lindex, int Rindex)
{
    int sredina = (Lindex + Rindex)/2;
    if(polje[sredina] < polje[Lindex])
        Zamjena(polje+Lindex, polje+sredina);
    if(polje[Rindex] < polje[Lindex])
        Zamjena(polje+Lindex, polje+Rindex);
    if(polje+sredina < polje+Rindex)
        Zamjena(polje+sredina, polje+Rindex);
}

int Podijeli(int Lindex, int Rindex)
{
  int i, index = Lindex;
    MedianTrojke(Lindex, Rindex);
    for(i=Lindex; i<Rindex; i++)
    {
        if(*(polje+i) < *(polje+Rindex))
        {
            Zamjena(polje+i, polje+index);
            index++;
        }
    }
    Zamjena(polje+Rindex, polje+index);
    return index;
}

void QSort(int Lindex, int Rindex)
{
    int index;
    if(Lindex < Rindex)
    {
      index = Podijeli(Lindex, Rindex);
      dubina++;
      if(dubina > maxDubina)
        maxDubina = dubina;
      QSort(Lindex, index-1);
      QSort(index+1, Rindex);
      dubina--;
    }
}

bool Test()
{
    int i;
    for(i=0; i<velicina-1; i++)
        if(*(polje+i)>*(polje+i+1))
            return false;
    return true;
}

void setup() {
  polje = new int[velicina];
  ispis = new char[80];
  srand(millis());
  rand();
  rand();
  Serial.begin(9600);
}

void loop() {
  int i;
  for(i=0; i<velicina; i++)
    polje[i]=rand()%velicina;
  dubina = 0; 
  maxDubina = 0;
  brZamjena = 0;
  pocetak = millis();
  QSort(0, velicina-1);
  kraj = millis();
  for(i=0; i<velicina; i++)
  {
    sprintf(ispis, "%d ",polje[i]);
    Serial.print(ispis);
    if(i%20 == 0)
      Serial.print("\n");
  }
  Serial.print("\n");
  if(Test()==false)
    Serial.print("Greska u sortiranju.\n");
  else
  {
    sprintf(ispis,"Vrijeme sortiranja: %d ms\n",(kraj-pocetak));
    Serial.print(ispis);
    sprintf(ispis,"Elemenata: %d, max dubina: %d, broj zamjena: %d\n",velicina, maxDubina, brZamjena);
    Serial.print(ispis);
  }
  delay(2000);
  Serial.print("----------------\n");
}
