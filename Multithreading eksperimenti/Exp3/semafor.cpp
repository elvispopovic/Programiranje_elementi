#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <ctime>


using namespace std;

mutex m, mi, mb;
condition_variable u_citanje, u_pisanje, u_barijera;
bool izvrsavanje;

void dretva_pise(char *polje, int velicina, int *kursor, int broj_dretve)
{
	int i;
	srand(time(0)+broj_dretve);
	rand(); rand();
	char upis[80];
	char upisana_vrijednost;
	unique_lock<mutex> barijera(mb);
	u_barijera.wait(barijera,[]{return izvrsavanje;});
	mi.lock();
	cout << "--------------------" << endl;
	cout << broj_dretve << "(pisanje): pustena kroz barijeru." << endl;
	mi.unlock();
	barijera.unlock();
	do
	{
		mi.lock();
		cout << "--------------------" << endl;
		cout << broj_dretve << "(pisanje): Radim..." << endl;
		mi.unlock();
		this_thread::sleep_for(chrono::milliseconds(10+rand()%10));
		//uvjetno zakljucavanje pocinje ovdje - preuzima se mutex ali se oslobadja ako nije ispunjen uvjet
		unique_lock<mutex> brava(m);
		u_pisanje.wait(brava,[kursor,velicina]{return (*kursor)<velicina-1;});
		mi.lock();
		cout << "--------------------" << endl;
		cout << broj_dretve << "(pisanje): pristupam resursu..." << endl;
		upisana_vrijednost=5*broj_dretve+rand()%5;
		sprintf(upis,"%d(pisanje): upisujem(%d): %d",broj_dretve,(*kursor)+1,upisana_vrijednost);
		for(i=0; i<strlen(upis); i++)
		{
			cout << upis[i];
			this_thread::sleep_for(chrono::milliseconds(10));
		}
		cout << endl;
		cout << broj_dretve << "(pisanje): upisano." << endl;
		mi.unlock();
		(*kursor)++;
		polje[*kursor]=upisana_vrijednost;
		u_citanje.notify_one();
	}while(izvrsavanje);
}

void dretva_cita(char *polje, int velicina, int *kursor, int broj_dretve)
{
	int i;
	char upis[80];
	srand(time(0)+broj_dretve);
	rand(); rand();
	unique_lock<mutex> barijera(mb);
	u_barijera.wait(barijera,[]{return izvrsavanje;});
	mi.lock();
	cout << "--------------------" << endl;
	cout << broj_dretve << "(citanje): pustena kroz barijeru." << endl;
	mi.unlock();
	barijera.unlock();
	do
	{
		mi.lock();
		cout << "--------------------" << endl;
		cout << broj_dretve << "(citanje): Radim..." << endl;
		mi.unlock();
		this_thread::sleep_for(chrono::milliseconds(10+rand()%10));
		//uvjetno zakljucavanje pocinje ovdje - preuzima se mutex ali se oslobadja ako nije ispunjen uvjet
		unique_lock<mutex> brava(m);
		u_citanje.wait(brava,[kursor,velicina]{return (*kursor)>=0;});
		mi.lock();
		cout << "--------------------" << endl;
		cout << broj_dretve << "(citanje): pristupam resursu..." << endl;
		sprintf(upis,"%d(citanje): citam (%d): %d",broj_dretve,(*kursor),(int)polje[*kursor]);
		for(i=0; i<strlen(upis); i++)
		{
			cout << upis[i];
			this_thread::sleep_for(chrono::milliseconds(10));
		}
		cout << endl;
		cout << broj_dretve << "(citanje): Procitano." << endl;
		mi.unlock();
		(*kursor)--;
		u_pisanje.notify_one();
	}while(izvrsavanje);
}

int main(int argc, char** argv) {
	int i;
	char polje[10];
	int kursor =-1;	
	thread *pisanje[3];
	thread *citanje[3];
	izvrsavanje=false;	
	for(i=0; i<3; i++)
	{
		pisanje[i]= new thread(dretva_pise,polje,10, &kursor,i);
		citanje[i]= new thread(dretva_cita,polje,10, &kursor,i);
	}
	cout << "Dretve su zakljucane na barijeri." << endl;
	this_thread::sleep_for(chrono::seconds(1));
	cout << "Barijera se otvara." << endl;
	izvrsavanje=true;
	u_barijera.notify_all();
	
	cout << "Za kraj unesi bilo sto." << endl;
	char unos[20];
	cin >> unos;
	izvrsavanje=false;
	u_pisanje.notify_all();
	u_citanje.notify_all();
	for(i=0; i<3; i++)
	{
		pisanje[i]->join();
		delete pisanje[i];
		citanje[i]->join();
		delete citanje[i];
	}
	return 0;
}
