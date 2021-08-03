all: dbg.Makefile
	make -f dbg.Makefile

clean:
	rm *.o -f
	rm *.elf -f
