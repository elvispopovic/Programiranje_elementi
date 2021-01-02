.cpu    cortex-a72
.fpu    neon-fp-armv8
.syntax unified  
.global quick_sort
.type quick_sort, %function

quick_sort:
	@ prolog
	sub sp, sp, #8		@ podizemo stog za dvije varijable
	str lr, [sp,#4]		@ spremamo lr pozivatelja na stog
	str fp, [sp]		@ spremamo fp pozivatelja na stog
	add fp, sp, #4		@ postavljamo fp na lokalno dno stoga
	sub sp, sp, #8		@ podizemo stog za jos dvije varijable
	str r0, [fp, #-8]	@ spremamo pokazivac polja na stog
	str r1, [fp, #-12]	@ spremamo velicinu polja na stog

	@ tijelo
	ldr r0, [fp, #-8]	@ adresa prvog elementa polja
	ldr r2, [fp, #-12]	@ ucitavamo velicinu sa stoga
	sub r2, r2, #1		@ smanjujemo velicinu za 1
	lsl r2, r2, #1		@ mnozimo velicinu sa 2 (dvobajtno polje)
	add r1, r0, r2		@ adresa zadnjeg elementa polja
	bl qsort		@ pozivamo quicksort rekurziju

	@ epilog
	add sp, sp, #8		@ dealociramo polje i velicinu
	ldr lr, [sp, #4]	@ vracamo lr pozivatelja
	ldr fp, [sp]		@ vracamo fp pozivatelja
	add sp, sp, #8		@ spustamo stog (dealociramo sve)
	bx lr			@ lr je vracen, prebacujemo ga u pc

.size quick_sort, .-quick_sort

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
	sub r1, r4, #2	    	@ u desnu stranu ide desna strelica bez pivota, on je na svom mjestu
	bl qsort	        @ pozivamo sami sebe (rekurzivno), r0, r1 i r2 moramo cuvati na stogu
	mov r1, r2        	@ vracamo desnu stranu stranu iz r2

	@ podijeli pa ovladaj od lijeve strelice do desne strane
	mov r2, r0        	@ spremamo lijevu stranu u r2 (ne treba nam vise pivot)
	mov r0, r3       	@ u lijevu stranu ide lijeva strelica, desna strana ostaje u r1
	bl qsort	        @ pozivamo sami sebe (rekurzivno), r0, r1 i r2 moramo cuvati na stogu
	mov r0, r2        	@ vracamo lijevu stranu iz r2

	pop {r0-r2,lr}     	@ vracamo registre koji cuvaju strane sa stoga i lr
qsort_L6:            	@ kraj - ako se registri nisu koristili, nismo ih ni cuvali na stogu
	bx lr            	@ lr je vec vracen ili nije ni mijenjan, samo ga prebacujemo u pc


