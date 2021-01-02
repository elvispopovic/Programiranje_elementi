main:
	mov r0, #1        	@ standardni izlaz
	ldr r1, .L0+4    	@ niz znakova nesortirano_polje
	swi 0x69        	@ ispis
	ldr r0, .L0        	@ adresa polja
	mov r1, #dPolje		@ velicina polja u bajtovima
	lsr r1, r1, #1  	@ velicina polja po broju elemenata
	bl ispisi        	@ pozivanje potprograma za ispis
	
	ldr r0, .L0        	@ adresa polja
	mov r1, #dPolje		@ velicina polja u bajtovima
	mov r1, r1, lsr #1  @ velicina polja po broju elemenata
	bl hsort        	@ poziv potprograma za heap sort

	mov r0, #1        	@ standardni izlaz
	ldr r1, .L0+8    	@ niz znakova sortirano_polje
	swi 0x69        	@ ispis	
	ldr r0, .L0        	@ adresa polja
	mov r1, #dPolje
	lsr r1, r1, #1  	@ velicina polja
	bl ispisi        	@ pozivanje potprograma za ispis
	swi 0x11        	@ kraj
	
.L0:            		@ varijable u mainu (pokazivaci na podatke) u strukturi
	.word polje
	.word ispis_nesortirano
	.word ispis_sortirano
	.size main, .-main	@ velicina funkcije main na kraju strukture podataka

@ heap sort algoritam
@ u r0 adresa polja, u r1 velicina
hsort:
	push {lr}    	@ zove druge potprograme pa moramo spremiti lr
	mov r2, #2    	@ prvi dio, gradi se hrpa. Pocevsi od korjena prema listovima
hsort_L0:        	
	cmp r2, r1    	@ usporedjuje se s krajem
	bge hsort_L1   	@ ako je doslo do kraja preskace se
	bl pomakni_gore @ za svaki element, ide se uz stablo do svog mjesta
	add r2, r2, #1 	@ povecava se 
	b hsort_L0
hsort_L1:	

	sub r2, r1, #1 	@ drugi dio skidaju se elementi s vrha hrpe i stavljaju na kraj
hsort_L2:        	@ polja, a stari element u polju ide na vrh pa se pomice dolje
	cmp r2, #1    	@ ide od kraja prema pocetku
	ble hsort_L3
	mov r3, #1    	@ indeks prvog elementa u r3
	
	lsl r4, r2, #1		@ zamjena elemenata, zadnji nesortirani se zamjenjuje sa korjenom
	ldrh r6, [r0, r4]	@ mnozimo indeks sa 2 jer je polje dvobajtno, te dodajemo
	lsl r5, r3, #1    	@ adresu polja - nemamo ekvivalent intelovoj instrukciji LEA
	ldrh r7, [r0, r5]
	strh r6, [r0, r5]
	strh r7, [r0, r4]	@ kraj zamjene
	
	bl pomakni_dolje@ vrh se pomice dolje do svog mjesta
	sub r2, r2, #1
	b hsort_L2
hsort_L3:
	pop {pc}

@ pomicanje elementa uz binarno stablo
@ polje u r0, element koji se pomice u r2
pomakni_gore:
	push {r1,r2}
pomakni_gore_L0:
	cmp r2, #1
	ble pomakni_gore_L2
	lsr r1, r2, #1    	@ dijeli se sa 2, roditelj
	lsl r5, r1, #1    	@ mjesto roditelja u memoriji polja
	lsl r6, r2, #1    	@ mjesto elementa u memoriji polja
	ldrh r3, [r0, r5]  	@ sadrzaj roditelja
	ldrh r4, [r0, r6]  	@ sadrzaj elementa
	cmp r3, r4
	bge pomakni_gore_L1
	strh r3, [r0, r6]  	@ zamjena sadrzaja roditelja i elementa
	strh r4, [r0, r5]
pomakni_gore_L1:
	mov r2, r1        	@ element se premjesta na roditelja
	b pomakni_gore_L0
pomakni_gore_L2:
	pop {r1,r2}
	bx lr

@ pomicanje 1 elementa niz binarno stablo
@ polje u r0, u r2 je granica
pomakni_dolje:
	push {r1, lr}
	mov r1, #1        	@ tekuci element, pocinje od vrha
	mov r3, #2        	@ lijevo dijete
	mov r4, #3        	@ desno dijete
pomakni_dolje_L0:
	cmp r3, r2        	@ usporedba lijevog i granice, while petlja
	bge pomakni_dolje_L10

	cmp r4, r2        	@ desno i granica
	bge pomakni_dolje_L5 @ ako je desno dijete izvan granice idemo dalje
	

	lsl r5, r3, #1 		@ usporedba sadrzaja u polju, pripremamo indeks
	ldrh r5, [r0, r5]  	@ mnozimo ga sa 2 (dvobajtno polje) i dodajemo adresi polja
	lsl r6, r4, #1    	@ isto i za drugi element, te ucitavamo za njih sadrzaj
	ldrh r6, [r0, r6]
	cmp r5, r6        	@ sada mozemo sadrzaje usporediti
	bge pomakni_dolje_L5 @ ako lijevo dijete nije manje od desnog idemo dalje
	
	lsl r5, r1, #1 		@ usporedba sadrzaja u polju, pripremamo indeks
	ldrh r5, [r0, r5]  	@ mnozimo ga sa 2 (dvobajtno polje) i dodajemo adresi polja
	lsl r6, r4, #1    	@ isto i za drugi element, te ucitavamo za njih sadrzaj
	ldrh r6, [r0, r6]
	cmp r5, r6        	@ sada mozemo sadrzaje usporediti
	bge pomakni_dolje_L1 @ ako tekuci nije manji od desnog djeteta preskacemo

	lsl r5, r1, #1		@ zamjena elemenata, desno dijete i tekuci element
	ldrh r7, [r0, r5]	@ mnozimo indeks sa 2 jer je polje dvobajtno, te dodajemo
	lsl r6, r4, #1    	@ adresu polja - nemamo ekvivalent intelovoj instrukciji LEA
	ldrh r8, [r0, r6]  	@ to cinimo za obje varijable
	strh r7, [r0, r6]
	strh r8, [r0, r5]	@ kraj zamjene
	
pomakni_dolje_L1:
	mov r1, r4        	@ tekuci se seli na desno dijete
	b pomakni_dolje_L9
	
pomakni_dolje_L5:

	lsl r5, r1, #1 		@ usporedba sadrzaja u polju, pripremamo indeks
	ldrh r5, [r0, r5]  	@ mnozimo ga sa 2 (dvobajtno polje) i dodajemo adresi polja
	lsl r6, r3, #1    	@ isto i za drugi element, te ucitavamo za njih sadrzaj
	ldrh r6, [r0, r6]
	cmp r5, r6        	@ sada mozemo sadrzaje usporediti
	bge pomakni_dolje_L6
	
	lsl r5, r1, #1		@ zamjena elemenata, lijevo dijete i tekuci element
	ldrh r7, [r0, r5]	@ mnozimo indeks sa 2 jer je polje dvobajtno, te dodajemo
	lsl r6, r3, #1    	@ adresu polja - nemamo ekvivalent intelovoj instrukciji LEA
	ldrh r8, [r0, r6]  	@ to cinimo za obje varijable
	strh r7, [r0, r6]
	strh r8, [r0, r5]	@ kraj zamjene
	
pomakni_dolje_L6:
	mov r1, r3        	@ tekuci se seli na lijevo dijete
	
pomakni_dolje_L9:
	lsl r3, r1, #1    	@ azuriramo novo lijevo dijete
	add r4, r3, #1    	@ azuriramo novo desno dijete
	b pomakni_dolje_L0  @ kraj while petlje
pomakni_dolje_L10:
	pop {r1, pc}
	bx lr

@ ispisuje polje proslijedjeno u r0 duljine r1
@ sve upotrebljavane registre prvo spremamo na stog 	
ispisi:
	push {r0-r4,lr}
	add r2, r0, #2    	@ polje, ulazni parametar u r0, prvi element se ne koristi
	lsl r4, r1, #1    	@ velicina polja, ulazni parametar u r1, dvobajtni elementi
	sub r4, r4, #1    	@ zadnji element je duljina - 1 
	mov r0, #1        	@ stdout
	mov r3, #1      	@ brojac
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
	.2byte 0,100,7,3,5,1,8,4,2,9,6,11,17,4,5,0,9,4,5,3,7,23,12,15,2,99,44 
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