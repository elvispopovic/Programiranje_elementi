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

	@ tu pocinje primjer sa vjezbe
	mov r0,#7
	mov r1,#5
	cmp r0, r1
	blt manji
	beq jednak
	bgt veci
manji:
	mov r1, #1
	bal kraj
jednak:
	mov r1, #2
	bal kraj
veci:
	mov r1, #3
	bal kraj
kraj:
	bl ispisi
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
	.word ispis
	.word ispis_n
	.size main,.-main

ispisi:
	push {r0-r2, lr}
	ldr r0, .L0	@adresa znakovnog niza za ispis teksta
			@u r1 vec jest upisana vrijednost
	ldr r2, .L0+4	@argument, adresa znakovnog niza za novi red
	bl printf	@poziv C funkcije za formatirani ispis
	pop {r0-r2, pc}


.section .rodata	@podatkovni blok, read only
.align 2
ispis:
	.asciz "U registar r1 je upisano %d%s"
	.align 2
ispis_n:
	.asciz "\n"
	.equ dI_n,.-ispis_n
