//---------------------------------------------------------------------------

#ifndef GUI_unit1H
#define GUI_unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Forms3D.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
//---------------------------------------------------------------------------
class TGUI_form : public TForm3D
{
__published:	// IDE-managed Components
private:	// User declarations
public:		// User declarations
	__fastcall TGUI_form(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TGUI_form *GUI_form;
//---------------------------------------------------------------------------
#endif
