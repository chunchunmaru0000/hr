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
	prep.c prep_include.c prep_se.c prep_some.c \
	pser.c eep.c \
		pser_type.c \
		pser_global_inst.c pser_local_inst.c \
		pser_eg.c pser_el.c pser_tpes.c \
		pser_g_bin.c \
	tzer.c ee.c lsts.c
	chmod +x ср
	./ср

# #WARNS := 0
# #DEBUG := 0
# #OPT := 3
# TESTS := ../tests/
# BUILD := ../tests/build
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
