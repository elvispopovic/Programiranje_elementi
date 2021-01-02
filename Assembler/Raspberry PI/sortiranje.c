#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

void quick_sort(short *polje, int velicina);
void heap_sort(short *polje, int velicina);
int usporedba (const void * a, const void * b);
void ispis_polja();

int main(int argc, char **argv)
{
	int i;
	int velicina = 200;
	short *ptPolje;
	short *polje,*polje1,*polje2, *polje3;
	struct timespec start, end;
	int64_t razlika;

	if(argc < 2)
		printf("Nije unesen argument za velicinu polja. Bit ce postavljen na 200.\n");	
	else
	{
		if((velicina = atoi(argv[1]))==0)
		{
			printf("Neispravan argument. Bit ce postavljen na 200.\n");
			velicina = 200;
		}
		else if(velicina<10 || velicina >100000)
		{
			printf("Argument izvan raspona [10, 100000]. Bit ce postavljen na 200.\n");
			velicina = 200;
		}
	}

	polje = malloc(velicina*sizeof(short));
	srand(time(NULL));
	rand(); rand();
	for(i=0, ptPolje=polje; i<velicina; i++, ptPolje++)
		*ptPolje = rand()%1000;
		
	polje1 = malloc(velicina*sizeof(short));
	polje2 = malloc(velicina*sizeof(short));
	polje3 = malloc(velicina*sizeof(short));
	memcpy(polje1, polje, velicina*sizeof(short));
	memcpy(polje2, polje, velicina*sizeof(short));
	memcpy(polje3, polje, velicina*sizeof(short));

	printf("Nesortirano polje1 velicine %d:\n", velicina);
	ispis_polja(polje1, velicina);
	clock_gettime(CLOCK_MONOTONIC, &start);
	quick_sort(polje1, velicina);
	clock_gettime(CLOCK_MONOTONIC, &end);
	razlika = end.tv_nsec - start.tv_nsec;
	printf("Sortirano polje1 (quicksort, %" PRId64 " ns):\n", razlika);
	ispis_polja(polje1, velicina);
	
	printf("Nesortirano polje2 velicine %d:\n", velicina);
	ispis_polja(polje2, velicina);
	clock_gettime(CLOCK_MONOTONIC, &start);
	heap_sort(polje2, velicina);
	clock_gettime(CLOCK_MONOTONIC, &end);
	razlika = end.tv_nsec - start.tv_nsec;
	printf("Sortirano polje2 (heapsort, %" PRId64 " ns):\n", razlika);
	ispis_polja(polje2, velicina);

	printf("Nesortirano polje3 velicine %d:\n", velicina);
	ispis_polja(polje3, velicina);
	clock_gettime(CLOCK_MONOTONIC, &start);
	qsort(polje3, velicina, sizeof(short), usporedba);
	clock_gettime(CLOCK_MONOTONIC, &end);
	razlika = end.tv_nsec - start.tv_nsec;
	printf("Sortirano polje3 (quicksort c library, %" PRId64 " ns):\n", razlika);
	ispis_polja(polje3, velicina);

	free(polje3);
	free(polje2);
	free(polje1);
	free(polje);
	return 0;
}

int usporedba (const void * a, const void * b) 
{
   return ( *(short*)a - *(short*)b );
}

void ispis_polja(short *polje, int velicina)
{
	int i;
	short *ptPolje;
	for(i=0, ptPolje = polje; i<velicina-1; i++, ptPolje++)
		printf("%d ", *ptPolje);
	printf("%d\n", *ptPolje);
}