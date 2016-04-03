ab: memorymapper experimentsB

memorymapper: memorymapper.c
	@gcc -o memorymapper memorymapper.c

experimentsB: experimentsB.c
	@gcc -o experimentsB experimentsB.c

clean:
	@rm -rf experimentsB memorymapper fileGenerator

distclean: clean
	@rm -rf fileGenerator
	@rm -rf files

files:
	@g++ -o fileGenerator fileGenerator.cpp
	@[ -d files ] || mkdir files
	@./fileGenerator 25

all: distclean files ab
