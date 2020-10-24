//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner, ITestFunkcije *objekt)
	: TForm(Owner)
{
	testObjekt = objekt;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Gumb_Click(TObject *Sender)
{
	char ime[25];
	TControl *Ctrl = static_cast<TControl*>(Sender);
	wcstombs(ime,Ctrl->Name.c_str(),25);
	if(testObjekt!=NULL)
	   testObjekt->Click(ime);

}
//---------------------------------------------------------------------------
