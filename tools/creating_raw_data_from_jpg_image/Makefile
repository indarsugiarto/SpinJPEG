MF = Makefile.1 Makefile.2

RM := rm -rf
LS := ls -l

all: $(MF)
	@for p in $(MF); do ("$(MAKE)" -f $$p) || exit $$?; done
	gcc -o reduce reduce.c

clean: $(MF)
	@for p in $(MF); do ("$(MAKE)" -f $$p clean) || exit $$?; done
	$(RM) reduce

