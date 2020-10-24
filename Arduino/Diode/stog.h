#ifndef STOG_H
#define STOG_H

class Stog
{
  private:
    byte vrh;
    byte velicina;
    byte *sadrzaj;
  public:
    Stog(byte velicina);
    bool dodaj(byte vrijednost);
    bool skini(byte& vrijednost);
    void isprazni();
};

#endif
