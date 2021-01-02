.cpu    cortex-a72
.fpu    neon-fp-armv8
.syntax unified     

.text
.align 2
.global printf
.global itoa
.type itoa, %function
.global main
.type main, %function

main:
	@prolog
	sub sp, sp, #8	@ podizemo stog za 2 mjesta (za dvije varijable)
	str fp, [sp]	@ na vrh stoga stavljamo fp pozivatelja
	str lr, [sp, 4] @ ispod stavljamo lr pozivatelja
	add fp, sp, #4	@ punimo fp adresom dna stoga (tu je inace lr pozivatelja)

	@ alokacija dvije lokalne varijable na stogu (argc i **argv)
	@ argc je u r0, a **argv je u r1
	sub sp, sp, #8		@ podizemo stog za jos dvije varijable
	str r0, [fp, #-8]	@ argc na drugo mjesto (fp-8 isto sto i sp+4)
	str r1, [fp, #-12]	@ **argv na prvo mjesto (fp-12 isto sto i sp)
	
	@ tu pocinje konkretni dio
	ldr r5, [fp, #-8]	@ argc u r5
	ldr r4, [fp, #-12]	@ argv u r4
	mov r6, #0	        @ brojac na nulu

	mov r7, #4		@ oznaka pisanja
main_L1:
	ldr r1, [r4]		@ adresa argumenta u r1
	bl duljina		@ rezultat ce biti u r2
	mov r0,#1	        @ ispisujemo na standardni izlaz
	svc 0  	        	@ prekid koji ispisuje sadrzaj
	ldr r1, .L0
	mov r2, #1		@ znamo duljinu, samo jedan znak
	mov r0, #1		@ ispisujemo na standardni izlaz
	svc 0	  	        @ prekid koji ispisuje sadrzaj
	add r6, r6, #1		@ povecavamo brojac	
	add r4, r4, #4		@ povecavamo pokazivac na argumente
	cmp r6, r5	        @ usporedjujemo brojac s brojem argumenata
	blt main_L1

	sub sp, sp, #4		@ alociramo mjesto na stogu
	mov r0, #9
	str r0, [fp, #-16]
	mov r3, #0	        @ donja granica, brojimo do 0 unazad

main_L2:	
	ldr r0, [fp, #-16]	@ ucitavamo varijablu sa stoga u r0
	sub r2, r0, #1		@ smanjujemo za 1 u r2
	str r2, [fp, #-16]	@ i spremimo novu vrijednost u varijablu na stogu
	bl itoa			@ uzima r0 i vraca r1
	bl duljina		@ uzima r1 i vraca r2
	mov r0, #1		@ stdout
	svc 0
	ldr r2,[fp, #-16]	@ ponovno ucitamo varijablu sa stoga 
	cmp r2, r3		@ usporedimo sa r3
	bge main_L2		@ ako je vece od r3 ponavljamo
	ldr r1, .L0+4		@ kraj
	mov r2, dKraj		@ duljina
	svc 0			@ r0 nam jos vrijedi pa zovemo svc
	

@ dealociramo varijablu sa stoga
	add sp, sp, #4		@ spustamo stog za jedno mjesto (gdje je varijabla)

@ dealokacija dvije varijable (argc i **argv na stogu)
	add sp, sp, #8		@ spustamo stog za dva mjesta
	
@ epilog
	mov r0, #0	        @ povratna vrijednost (kao exit 0)
	ldr fp, [sp]		@ vracamo fp pozivatelja sa stoga
	ldr lr, [sp, #4]	@ vracamo lr pozivatelja sa stoga
	add sp, sp, #8		@ spustamo stog za dva mjesta
	bx lr		        @ vracamo se (pc punimo sa lr)

.L0:
	.word novi_red
	.word kraj
	.size dMain, .-main
@ ------------------------------------
@ izracunavanje duljine znakovnog niza
@ ------------------------------------
duljina:
	sub sp, sp, #8
	str r0, [sp]
	str r1, [sp, #4]
	mov r2, #0
duljina_L1:
	ldrb r0, [r1], #1
	add r2, r2, #1	
	cmp r0, #0
	bne duljina_L1
	ldr r1, [sp, #4]
	ldr r0, [sp]
	add sp, sp, #8	
	bx lr
@ ------------------------------------
@ itoa
@ pretvorba broja u znakovni niz
@ ------------------------------------
itoa:
	sub sp, sp, #20
	str r0, [sp]
	str r2, [sp, #4]
	str r3, [sp, #8]
	str r4, [sp, #12]
	str lr, [sp, #16]	@ moramo spremiti i lr jer itoa dalje zove potprograme

	mov r2, r0		@ predznak
	cmp r0, #0		@ usporedba
	bge itoa_L1		@ ako je ne negativan ide se dalje 
	neg r0, r0		@ inace se negira
itoa_L1:
	ldr r3, .L1
itoa_L2:
	bl dijeli10
	add r1, r1, #48 	@ dodaj ASCII za 0
	strb r1, [r3], #1
	cmp r0, #0
	bgt itoa_L2
	cmp r2, #0
	bge itoa_L3
	mov r0, #45
	strb r0, [r3], #1
itoa_L3:
	mov r0, #0
	strb r0, [r3], #-1
	ldr r0, .L1
	sub r4, r3, r0
	lsr r4, #1
	add r4, r0, r4 
	ldr r0, .L1
itoa_L4:
	ldrb r1, [r0]
	ldrb r2, [r3]
	strb r1, [r3], #-1
	strb r2, [r0], #1
	cmp r0, r4
	ble itoa_L4

	ldr r1, .L1		@rezultat u r1

	ldr r0, [sp]
	ldr r2, [sp, #4]
	ldr r3, [sp, #8]
	ldr r4, [sp, #12]
	ldr lr, [sp, #16]
	add sp, sp, #20
	bx lr

@ -----------------------------------
@ dijeljenje s 10
@ pomocni potprogram kojeg treba itoa
@ -----------------------------------

dijeli10:			@ dijeli r0 sa 10, rezultat u r0, ostatak u r1
	sub sp, sp, #12
	str lr, [sp]
	str r2, [sp, #4]
	str r0, [sp, #8]

	ldr r1, =0xcccccccd	@ magicni broj 3435973837
	umull r1, r2, r1, r0	@ mnozenje dva 32 bitna broja u 64 bitni (dva registra)
	lsr r0, r2, #3		@ treba pomaknuti za 35 bitova, 35-32=3 za ova dva bajta
	mov r1, #10		@ sad racunamo ostatak
	mul r1, r0, r1		@ pomnozimo rezultat sa 10
	ldr r2, [sp, #8]	@ ucitamo pocetni broj sa stoga
	sub r1, r2, r1		@ oduzmemo pocetni broj od 10xrezultat

	ldr r2, [sp, #4]
	ldr lr, [sp]
	add sp, sp, #12
	bx lr

.L1:
	.word n0
	.size dItoa, .-itoa

@ podaci u bbs bloku
.section .bss
.align 2
n0:
	.skip 16

@ podaci u read only bloku
.section .rodata
.align 2
novi_red:
	.asciz "\n"
	.equ dNovi, .-novi_red
	.align 2
kraj:
	.asciz "\nKraj.\n"
	.equ dKraj, .-kraj
	.text
