@ program (frame) koji poziva drugi program (frame) uz 3 argumenta
main:			        @ main naredbenog retka iz kojeg se poziva program
	mov ip, sp	        @ punimo ip sa tekucim dnom stoga
	mov r0, #4	        @ argc u r0 (broj argumenata koji se prosljedjuju)
	ldr r1, .L0	        @ **argv u r1 (niz argumenata, tj. niz nizova znakova)
	bl main_programa	@ pozivamo program
	
	mov r0, #1
	ldr r1, .L0+16		@ niz znakova za kraj
	swi 0x69	        @ ispisujemo kraj
	swi 0x11

.L0:			        @ niz argumenata (niz nizova znakova)
	.word .L1
	.word .L1+4
	.word .L1+8
	.word .L1+12
	.word kraj

.L1:			        @ argumenti kao nizovi znakova
	.word program		@ ime programa
	.word arg1	        @ argument 1
	.word arg2	        @ argument 2
	.word arg3	        @ argument 3
.size main, .-main

main_programa:			@ main programa koji se poziva s listom argumenata
@ prolog
	sub sp, sp, #8		@ podizemo stog za 2 mjesta
	str ip, [sp]		@ na vrh stoga stavljamo ip pozivatelja
	str lr, [sp, #4]	@ ispod stavljamo lr pozivatelja
	add ip, sp, #4		@ punimo ip adresom dna stoga za main_programa
	
@ alokacija dvije lokalne varijable na stogu (argc i **argv)
	sub sp, sp, #8		@ podizemo stog za dvije varijable
	str r0, [ip, #-8]	@ argc na drugo mjesto (isto sto i sp+4)
	str r1, [ip, #-12]	@ **argv na prvo mjesto (isto sto i sp)
	
@ tu pocinje konkretni dio
	ldr r5, [ip, #-8]	@ argc u r5
	ldr r4, [ip, #-12]	@ argv u r4
	mov r3, #0	        @ brojac na nulu
petlja:
	mov r0, #1	        @ ispis na standardni izlaz
	ldr r1, [r4]		@ adresa argumenta u r1
	swi 0x69	        @ prekid koji ispisuje sadrzaj na stdout
	add r3, r3, #1		@ povecavamo brojac	
	add r4, r4, #4		@ povecavamo pokazivac na argumente
	cmp r3, r5	        @ usporedjujemo brojac s brojem argumenata
	blt petlja
	
@ dodajemo novu varijablu na stog (koju cemo ispisivati i mijenjati)
	sub sp, sp, #4		@ podizemo stog za jedno mjesto (4 bajta)
	mov r0, #9	        @ pocetna vrijednost koju cemo staviti u varijablu
	str r0, [ip, #-16]	@ referenca nam je ip, dno stoga koje se ne mijenja
		                @ varijabla se nalazi na ip-16 na stogu
	mov r3, #0	        @ donja granica, brojimo do 0 unazad
petlja2:
	mov r0, #1	        @ ispis na standardni izlaz
	ldr r1, [ip, #-16]	@ ucitavamo varijablu sa stoga u r1
	sub r2, r1, #1		@ smanjujemo za 1 u r2
	str r2, [ip, #-16]	@ i spremimo novu vrijednost u varijablu na stogu
	swi 0x6B	        @ ispisujemo staru vrijednost iz r1
	cmp r2, r3	        @ jesmo li dosli do kraja?
	bge petlja2

@ dealociramo varijablu sa stoga
	add sp, sp, #4		@ spustamo stog za jedno mjesto (gdje je varijabla)

@ dealokacija dvije varijable (argc i **argv na stogu)
	add sp, sp, #8		@ spustamo stog za dva mjesta
	
@ epilog
	mov r0, #0	        @ povratna vrijednost (kao exit 0)
	ldr ip, [sp]		@ vracamo ip pozivatelja sa stoga
	ldr lr, [sp, #4]	@ vracamo lr pozivatelja sa stoga
	add sp, sp, #8		@ spustamo stog za dva mjesta
	bx lr		        @ vracamo se (pc punimo sa lr)
	
@ podaci u read only bloku
.section .rodata
.align 2
program:
	.asciz "program\n"
	.equ dProgram, .-program
	.align 2
arg1:
	.asciz "arg1\n"
	.equ dArg1, .-arg1	@ izracunata duljina za slucaj da je negdje potrebna (moze se izostaviti)
	.align 2
arg2:
	.asciz "arg2\n"
	.equ dArg2, .-arg2
	.align 2
arg3:
	.asciz "arg3\n"
	.equ dArg3, .-arg3
	.align 2
kraj:
	.asciz "\nKraj.\n"
	.equ dKraj, .-kraj


