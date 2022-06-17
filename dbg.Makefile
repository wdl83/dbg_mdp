include Makefile.defs

TARGET = dbg_mdp

CXXSRCS = \
	HijackSignal.cpp \
	ModbusLogFetcher.cpp \
	dbg.cpp \
	mdp/Client.cpp \
	mdp/MutualHeartbeatMonitor.cpp \
	mdp/ZMQClientContext.cpp \
	mdp/ZMQIdentity.cpp

include Makefile.rules
