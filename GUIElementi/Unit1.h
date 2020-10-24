//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include "CodeBlocks_test\\Test\\test.h"
#include <FMX.Memo.hpp>
#include <FMX.ScrollBox.hpp>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TCornerButton *Gumb_t1;
	TCornerButton *Gumb_t2;
	TGroupBox *GroupBox1;
	TMemo *memIspis;
	void __fastcall Gumb_Click(TObject *Sender);
private:	// User declarations
	ITestFunkcije *testObjekt;
public:		// User declarations
	__fastcall TForm1(TComponent* Owner, ITestFunkcije *objekt);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
