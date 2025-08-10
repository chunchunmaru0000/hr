тест: ср
	../../rus_asm/src/асм тест.асм тест
	chmod +x тест
	./тест

cat: ср
	../../rus_asm/src/асм тест.асм тест
	cat тест.асм
	chmod +x тест
	./тест

ср:
	gcc -o ср main.c \
	gner.c \
		gner_global_Асм_Linux_x64.c \
		gner_local_Асм_Linux_x64.c \
	pser.c eep.c \
		pser_type.c \
		pser_global_inst.c pser_local_inst.c \
		pser_eg.c pser_el.c pser_tpes.c \
	tzer.c ee.c lsts.c
	chmod +x ср
	./ср
