ifndef OBJ_DIR
OBJ_DIR = ${PWD}/obj
export OBJ_DIR
endif

ifndef DST_DIR
DST_DIR = ${PWD}/dst
export DST_DIR
endif

all: dbg.Makefile zmqpp/Makefile
	make -C zmqpp
	mkdir -p ${OBJ_DIR}/zmqpp
	make PREFIX=${OBJ_DIR}/zmqpp install -C zmqpp
	make -f dbg.Makefile

install: dbg.Makefile
	make -C zmqpp
	mkdir -p ${OBJ_DIR}/zmqpp
	make PREFIX=${OBJ_DIR}/zmqpp install -C zmqpp
	make PREFIX=${DST_DIR} install -C zmqpp
	make -f dbg.Makefile install

clean: dbg.Makefile
	make -f dbg.Makefile clean

purge:
	rm $(OBJ_DIR) -rf
