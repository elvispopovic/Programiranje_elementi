

Stog::Stog(byte velicina)
{
  this->velicina = velicina;
  sadrzaj = new byte[velicina+1];
  vrh = velicina;
}

bool Stog::dodaj(byte vrijednost)
{
  if(vrh == 0)
    return false;
  sadrzaj[vrh] = vrijednost;
  vrh--;
  return true;
}

bool Stog::skini(byte& vrijednost)
{
  if(vrh==velicina)
    return false;
  vrh++;
  vrijednost = sadrzaj[vrh];
  return true;
}

void Stog::isprazni()
{
  byte vrijednost;
  while(skini(vrijednost));
}
