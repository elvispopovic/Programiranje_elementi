#include <iostream>
#include <windows.h>
#include "main.h"

using namespace std;

//tip pokazivaca na funkciju koja dobavlja objekt
typedef void* (*VoidFunc)();
typedef void* (*VoidFunc1)(IntFuncChar);

int PozoviMe(const char * tekst)
{
	cout << "Pozvana je funkcija \"Pozovi me\" sa tekstom \"" << tekst << "\"." << endl;
	return 1;
}

int main(int argc, char** argv) {

	//ucitavamo dll
	HINSTANCE hInst = LoadLibrary("DLL_projekt.dll");
	if(!hInst)
	{
		cout << "Ne mogu otvoriti dll." << endl;
		return 1;
	}
	IntFuncChar pm = PozoviMe;
	cout << "Testiramo pozivanje funkcije \"Pozovi me\" preko pokazivaca na nju." << endl;
	pm("Ovo je tekst");

	//dobavljamo funkciju za proslijedjivanje pokazivaca na funkciju pozovi me
	VoidFunc1 PosaljiPok = reinterpret_cast<VoidFunc1>(GetProcAddress(hInst,"PosaljiFunkciju"));
	if(!PosaljiPok)
	{
		cout << "Ne mogu dobaviti funkciju za slanje pokazivaca na funkciju \"Pozovi me\" u dll." << endl;
		return 1;
	}
	cout << "Funkcija za prijenos pokazivaca na funkciju \"Pozovi me \" je dobavljena iz dll-a." << endl;
	PosaljiPok(pm);

	//dobavljamo funkciju za dobavljanje objekta
	VoidFunc DobaviObjekt = reinterpret_cast<VoidFunc>(GetProcAddress(hInst,"DobaviObjekt"));
	if(!DobaviObjekt)
	{
		cout << "Ne mogu dobaviti funkciju za eksport objekta iz dll-a." << endl;
		return 1;
	}
	cout << "Funkcija za dobavljanje objekta je dobavljena iz dll-a." << endl;
	//dobavljamo objekt i castamo ga na interface
	IKlasa *objekt = static_cast<IKlasa *>(DobaviObjekt());
	if(objekt==NULL)
	{
		cout << "Ne mogu dobaviti funkciju za eksport objekta iz dll-a." << endl;
		return 1;
	}
	//koristimo objekt
	cout << "Kreiran je i dobavljen objekt." << endl;
	cout << "------------------------------" << endl;
	objekt->f1();
	cout << "Pozivam f2(5) metodu objekta." << endl;
	int r = objekt->f2(5);
	cout << "Rezultat je " << r << "." << endl;
	cout << "Pozivam f3(25) metodu objekta." << endl;
	float rf = objekt->f3(25);
	cout << "Rezultat je " << rf << "." << endl;
	cout << "------------------------------" << endl;
	//unistavamo objekt
	delete objekt;

	return 0;
}
