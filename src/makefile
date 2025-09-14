тестовый: ср
	@-./ср
	@../../rus_asm/src/асм тест.асм тест
	@chmod +x тест
	@-./тест

тест:
	@-./ср #> /dev/null
	@-../../rus_asm/src/асм тест.асм тест > /dev/null
	@chmod +x тест
	@-./тест

cat: ср
	@-./ср
	@../../rus_asm/src/асм тест.асм тест
	@cat тест.асм
	@chmod +x тест
	@-./тест

ср:
	gcc -o ср main.c \
	gner.c \
		gner_global_Асм_Linux_x64.c \
		gner_local_Асм_Linux_x64.c \
	prep.c prep_include.c prep_se.c prep_some.c prep_sent.c \
	pser.c eep.c pser_type.c \
		pser_global_inst.c pser_tpes.c pser_g_bin.c \
		pser_local_inst.c pser_eg.c pser_el.c \
	tzer.c ee.c lsts.c
	chmod +x ср


TESTS := ../tests
BUILD := $(TESTS)/build
all: ср
	mkdir -p $(BUILD)
	mv тест.ср _тест.ср
	@echo "---------------------------------------------------------------------------------------------"

	cp $(TESTS)/тест_эээ_world.ср тест.ср ; $(MAKE) -s тест
	cp $(TESTS)/тест_глоб_выражение.ср тест.ср ; $(MAKE) -s тест
	cp $(TESTS)/тест_препроцессор.ср тест.ср ; $(MAKE) -s тест

	@echo "---------------------------------------------------------------------------------------------"
	mv _тест.ср тест.ср
	rm -rf $(BUILD)

end:
	mv _тест.ср тест.ср
	rm -rf $(BUILD)

# #WARNS := 0
# #DEBUG := 0
# #OPT := 3
# HRS := $(wildcard $(TESTS)/*.ср)
# ASMS := $(wildcard $(TESTS)/*.асм)
# BS := $(patsubst $(TESTS)/%.ср, $(BUILD)/%, $(TS))
# 
# b:
# 	mkdir -p $(BUILD)
# 
# $(BUILD)/%: $(TESTS)%.асм асм b
# 	@./асм $< $@ -о $(DEBUG) -п $(WARNS) -с $(OPT)
# 	chmod +x $@ ; $@ ;
# 
# all: $(BS)
# 	@./асм тест.асм тест -о $(DEBUG) -п $(WARNS) -с $(OPT)
# 	@./асм тест_заглавными.асм тест_заглавными -о $(DEBUG) -п $(WARNS) -с $(OPT)
# 	@chmod +x тест_заглавными
# 	./тест_заглавными < t.txt
# 	rm -rf $(BUILD)
# 	rm тест
# 	rm тест_заглавными
# 	
