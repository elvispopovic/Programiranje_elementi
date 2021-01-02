.cpu    cortex-a72
.fpu    neon-fp-armv8
.syntax unified  
.global heap_sort
.type heap_sort, %function

heap_sort:
	@ prolog
	sub sp, sp, #8		@ podizemo stog za dvije varijable
	str lr, [sp,#4]		@ spremamo lr pozivatelja na stog
	str fp, [sp]		@ spremamo fp pozivatelja na stog
	add fp, sp, #4		@ postavljamo fp na lokalno dno stoga

	@ tijelo
	sub r0, r0, #2		@ smanjujemo adresu za nulti element
	add r1, r1, #1		@ povecavamo broj elemenata za 1
	bl hsort			@ pozivamo heapsort

	@ epilog
	ldr lr, [sp, #4]	@ vracamo lr pozivatelja
	ldr fp, [sp]		@ vracamo fp pozivatelja
	add sp, sp, #8		@ spustamo stog (dealociramo sve)
	bx lr				@ lr je vracen, prebacujemo ga u pc

.size heap_sort, .-heap_sort

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


