.cpu    cortex-a72
.fpu    neon-fp-armv8
.syntax unified     

.text
.align 2
.global printf
.global main
.type main, %function

main:
	push {fp,lr}	@spremamo frame pointer i link register
	add fp, sp, #4	@u frame pointer stavljamo stack pointer sa lr
	mov r0, #0
	mov r1, r0
	mov r2, r1
	mov r3, r0
	mov r4, r0
	mov r5, r0
	mov r6, r0
	mov r7, r0

	@ tu pocinje primjer sa vjezbe
	
	@ inicijalizacija
	mov r0, #1
	mov r1, #6

	ldr r8, .L0+4
	mov r9, dInicijal
	bl ispisi_tekst
	bl ispisi_registre
	ldr r8, .L0+12
	mov r9, dZbrajanja
	bl ispisi_tekst

	add r0, r0, #2
	bl ispisi_registre

	add r2, r1, r0
	bl ispisi_registre

	add r2, r2, r1
	bl ispisi_registre

	ldr r8, .L0+16
	mov r9, dOduzimanja
	bl ispisi_tekst

	sub r1, r1, #3
	bl ispisi_registre

	sub r4, r2, r1
	bl ispisi_registre
	
	sub r5, r0, r4
	bl ispisi_registre

	@ inicijalizacija
	mov r0, #4
	mov r1, #6

	ldr r8, .L0+4
	mov r9, dInicijal
	bl ispisi_tekst
	bl ispisi_registre

	mul r2, r1, r0

	ldr r8, .L0+20
	mov r9, dMnozenje
	bl ispisi_tekst
	bl ispisi_registre

	@ inicijalizacija
	ldr r0, =0x00000006

	ldr r8, .L0+4
	mov r9, dInicijal
	bl ispisi_tekst
	bl ispisi_registre

	asr r0,r0,#1
	
	ldr r8, .L0+24
	mov r9, dPomakA
	bl ispisi_tekst
	bl ispisi_registre

	mov r0, #-6

	ldr r8, .L0+4
	mov r9, dInicijal
	bl ispisi_tekst
	bl ispisi_registre

	ldr r8, .L0+28
	mov r9, dPomakL
	bl ispisi_tekst

	asr r1,r0,#1
	
	bl ispisi_registre

	lsr r2,r0,#1
	
	bl ispisi_registre

	ldr r0, =0x00000003

	ldr r8, .L0+4
	mov r9, dInicijal
	bl ispisi_tekst
	bl ispisi_registre

	lsl r1,r0, #1

	bl ispisi_registre

	ldr r0, =0x00000001
	ldr r1, =0x00000001
	ldr r2, =0x00000000

	ldr r8, .L0+4
	mov r9, dInicijal
	bl ispisi_tekst
	bl ispisi_registre
	ldr r8, .L0+32
	mov r9, dLogicke
	bl ispisi_tekst

	ldr r8, .L0+36
	mov r9, dI
	bl ispisi_tekst
	
	and r3,r0,r1
	bl ispisi_registre
	and r4,r0,r2
	bl ispisi_registre

	ldr r8, .L0+40
	mov r9, dIli
	bl ispisi_tekst

	orr r5,r0,r1
	bl ispisi_registre
	orr r6,r0,r2
	bl ispisi_registre

	ldr r8, .L0+44
	mov r9, dXor
	bl ispisi_tekst

	eor r7,r0,r1
	bl ispisi_registre
	eor r8,r0,r2
	bl ispisi_registre
		
	
	@ tu zavrsava primjer s vjezbe
	
	@sad cemo za primjer ispisati novi red koristeci supervisor call
	mov r7, #4	@ispis
	mov r2, dI_n	@duljina znakovnog niza
	ldr r1, .L0+4	@adresa znakovnog niza za novi red
	mov r0, #1	@standardni izlaz (stdout)
	svc 0		@poziv supervizora (on ce generirati prekid)

	@sad cemo stvoriti povratnu vrijednost iz programa
	mov r7, #1   	@oznaka za exit
	mov r0, #0	@povratnu vrijednost stavljamo u r0
	svc 0		@pozivamo supervizor
	pop {fp,pc}	@vracanje
.align 2
.L0:
	.word ispis1
	.word inicijal
	.word ispis_n
	.word zbrajanja
	.word oduzimanja
	.word mnozenje
	.word pomakA
	.word pomakL
	.word logicke
	.word i
	.word ili
	.word xor
	.size main,.-main

@imamo mnogo argumenata, prva tri idu u registre, ostali na stog
ispisi_registre:
	push {r0-r3,lr}
	mov r3, r2	@u r3 (treci argument) stavljamo r2
	mov r2, r1	@u r2 (drugi argument) stavljamo r1
	mov r1, r0	@u r1 (prvi argument) stavljamo r0
	ldr r0, .L0+8	@u r0 stavljamo adresu novog reda
	push {r0}	@zadnji argument ide prvi na stog
	push {r3-r7}	@guramo sve argumente iznad r2 na stog
	ldr r0, .L0	@adresa znakovnog niza za ispis teksta
	bl printf	@poziv C funkcije za formatirani ispis
	add sp, sp, #24	@spustamo stog za 6 varijabli (r7-r3 i novi red)
	pop {r0-r3, pc}	@vracamo stanja registara sa stoga

@ ispis samo teksta, koristimo supervizor (svc) kao primjer
ispisi_tekst:
	push {r0-r2,r7,lr} @spremamo registre na stog
	mov r7, #4	@ispis
	mov r1, r8	@adresa znakovnog niza za novi red
	mov r2, r9	@duljina znakovnog niza
	mov r0, #1	@standardni izlaz (stdout)
	svc 0		@poziv supervizora (on ce generirati prekid)
	pop {r0-r2,r7,pc} @vracamo registre sa stoga

.section .rodata	@podatkovni blok, read only
.align 2
ispis1:
	.asciz "Registri r0: %d, r1: %d, r2: %d, r3: %d, r4: %d, r5: %d, r6: %d, r7: %d%s"
	.align 2
inicijal:
	.asciz "\nInicijalizacija registara\n"
	.equ dInicijal,.-inicijal
	.align 2
ispis_n:
	.asciz "\n"
	.equ dI_n,.-ispis_n
	.align 2
zbrajanja:
	.asciz "\nZbrajanja\n"
	.equ dZbrajanja,.-zbrajanja
	.align 2
oduzimanja:
	.asciz "\nOduzimanja\n"
	.equ dOduzimanja,.-oduzimanja
	.align 2
mnozenje:
	.asciz "\nMnozenje\n"
	.equ dMnozenje,.-mnozenje
	.align 2
pomakA:
	.asciz "\nAritmeticki pomak bitova\n"
	.equ dPomakA,.-pomakA
	.align 2
pomakL:
	.asciz "\nUsporedba arit. i log. pomaka za negativne brojeve\n"
	.equ dPomakL,.-pomakL
	.align 2
logicke:
	.asciz "\nLogicke operacije (i, ili i ekskluzivno ili)\n"
	.equ dLogicke,.-logicke
	.align 2
i:	
	.asciz "\nLogicka i\n"
	.equ dI,.-i
	.align 2
ili:	
	.asciz "\nLogicka ili\n"
	.equ dIli,.-ili
	.align 2
xor:	
	.asciz "\nLogicka xor (eor)\n"
	.equ dXor,.-xor
