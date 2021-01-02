

main:
	mov r0, #1        	@ standardni izlaz
	ldr r1, .L0+4    	@ niz znakova nesortirano_polje
	swi 0x69        	@ ispis
	ldr r0, .L0        	@ adresa polja
	mov r1, #dPolje   	@ velicina polja
	bl ispisi        	@ pozivanje potprograma za ispis
	
	ldr r0, .L0        	@ adresa polja (lijeva strana)
	add r1, r0, #dPolje	@ kraj polja, koristi se konstanta za velicinu
	sub r1, r1, #2    	@ zadnji element polja (kraj- 2 bajta, desna strana)
	bl qsort        	@ poziv potprograma za quick sort

	mov r0, #1        	@ standardni izlaz
	ldr r1, .L0+8    	@ niz znakova sortirano_polje
	swi 0x69        	@ ispis	
	ldr r0, .L0        	@ adresa polja
	mov r1, #dPolje   	@ velicina polja
	bl ispisi        	@ pozivanje potprograma za ispis
	swi 0x11        	@ kraj
	
.L0:            		@ varijable u mainu (pokazivaci na podatke) u strukturi
	.word polje
	.word ispis_nesortirano
	.word ispis_sortirano
	.size main, .-main	@ velicina funkcije main na kraju strukture podataka

@ quick sort algoritam
@ u r0 ide lijeva strana, u r1 ide desna strana
qsort:
	cmp r0, r1
	bge qsort_L6      	@ ako je polje od samo jednog elementa vracamo se, ne koristimo stog
	push {r0-r2,lr}    	@ spremamo registre koji cuvaju strane, te lr na stog
	ldrh r2, [r0]		@ pivot je sadrzaj lijeve strane, ide u r2
	mov r3, r0	    	@ lijeva strelica
	mov r4, r1	    	@ desna strelica
qsort_L1:
	cmp r3, r4	       	@ ako su se preklopile
	bgt qsort_L5		@ ide se na kraj
	ldrh r5, [r3]		@ sadrzaj lijeve strelice
	cmp r5, r2	    	@ usporedba sa sadrzajem pivota
	ble qsort_L2    	@ ako je manja ili jednaka preskace se
	ldrh r5, [r4]		@ sadrzaj desne strelice
	cmp r5, r2	    	@ usporedba sa sadrzajem pivota
	bge qsort_L2    	@ ako je veca ili jednaka preskace se
	ldrh r5, [r3]    	@ zamjena sadrzaja strelica, ako su oba gornja uvijeta
	ldrh r6, [r4]    	@ u r5 ide sadrzaj lijeve a u r6 sadrzaj desne
	strh r5, [r4]    	@ u desnu ide sadrzaj lijeve
	strh r6, [r3]    	@ u lijevu ide sadrzaj desne
qsort_L2:
	ldrh r5, [r3]		@ pomicanje lijeve strelice, sadrzaj u r5
	cmp r5, r2        	@ ako je sadrzaj veci od pivota
	bgt qsort_L3    	@ preskace se
	add r3, r3, #2    	@ lijeva strelica se pomice udesno za jedno mjesto (2 bajta)
qsort_L3:
	ldrh r5, [r4]		@ pomicanje desne strelice, sadrzaj u r5
	cmp r5, r2        	@ ako je sadrzaj manji od pivota
	blt qsort_L4    	@ preskace se
	sub r4, r4, #2    	@ desna strelica se pomice ulijevo za jedno mjesto (2 bajta)
qsort_L4: 
	b qsort_L1        	@ ako se nisu preklopile, vracamo se natrag (petlja)
	
qsort_L5:       		@ pivot ide na kraj prvog dijela
	ldrh r5, [r4]    	@ sadrzaj desne strelice koja je preklopljena pa je kraj prvog dijela
	strh r2, [r4]    	@ sadrzaj pivota (sadrzaj lijeva strane) ide u sadrzaj desne strelice
	strh r5, [r0]    	@ sadrzaj desne strelice ide u sadrzaj lijeve strane
	
	@ podijeli pa ovladaj od lijeve strane do desne strelice
	mov r2, r1        	@ spremamo desnu stranu u r2 (ne treba nam vise pivot)		
	sub r1, r4, #2	    @ u desnu stranu ide desna strelica bez pivota, on je na svom mjestu
	bl qsort        	@ pozivamo sami sebe (rekurzivno), r0, r1 i r2 moramo cuvati na stogu
	mov r1, r2        	@ vracamo desnu stranu stranu iz r2

	@ podijeli pa ovladaj od lijeve strelice do desne strane
	mov r2, r0        	@ spremamo lijevu stranu u r2 (ne treba nam vise pivot)
	mov r0, r3       	@ u lijevu stranu ide lijeva strelica, desna strana ostaje u r1
	bl qsort        	@ pozivamo sami sebe (rekurzivno), r0, r1 i r2 moramo cuvati na stogu
	mov r0, r2        	@ vracamo lijevu stranu iz r2
	pop {r0-r2,lr}     	@ vracamo registre koji cuvaju strane sa stoga i lr
qsort_L6:            	@ kraj - ako se registri nisu koristili, nismo ih ni cuvali na stogu
	bx lr            	@ lr je vec vracen ili nije ni mijenjan, samo ga prebacujemo u pc
	
@ ispisuje polje proslijedjeno u r0 duljine r1
@ sve upotrebljavane registre prvo spremamo na stog 	
ispisi:
	push {r0-r4,lr}
	mov r2, r0      	@ polje, ulazni parametar u r0
	mov r4, r1	    	@ velicina polja, ulazni parametar u r1
	mov r0, #1        	@ stdout
	mov r3, #0      	@ brojac
ispisi_L1:
	ldrh r1, [r2], #2
	swi 0x6b
	add r3, r3, #2
	cmp r3, r4      	@ usporedba s velicinom polja
	bge ispisi_L2
	ldr r1, .L1     	@ razmak sa zarezom
	swi 0x69
	b ispisi_L1
ispisi_L2:
	ldr r1, .L1+4
	swi 0x69
	pop {r0-r4,pc}
.align 2
.L1:
	.word razmak
	.word novi
	.size ispisi, .-ispisi

.data
.align 2
polje:
	.2byte 7,3,5,1,8,4,2,9,6,99,11,80,81,45,44,43,41,40,16,5,9,1000,999,200,201 
	.equ dPolje, .-polje
	.align 2
razmak:
	.asciz ", "
	.align 2
novi:
	.asciz "\n"
	.align 2
ispis_nesortirano:
	.asciz "Nesortirano polje:\n"
	.align 2
ispis_sortirano:
	.asciz "Sortirano polje:\n"
	