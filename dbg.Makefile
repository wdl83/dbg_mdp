include Makefile.defs

CFLAGS += $(DEFS)
CXXFLAGS += $(DEFS) 

TARGET = dbg_mdp

# CSRCS =

CXXSRCS = \
	../mdp/Client.cpp \
	../mdp/MutualHeartbeatMonitor.cpp \
	../mdp/ZMQClientContext.cpp \
	../mdp/ZMQIdentity.cpp \
	HijackSignal.cpp \
	ModbusLogFetcher.cpp \
	dbg.cpp

include Makefile.rules
