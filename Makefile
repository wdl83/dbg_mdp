all: dbg.Makefile
	mkdir -p zmqpp-root
	make -C zmqpp
	make PREFIX=${PWD}/zmqpp-root install -C zmqpp
	make -f dbg.Makefile

clean:
	make -f dbg.Makefile clean
