#pragma once

#include <signal.h>

using SignalHandler = void (*)(int);

void hijackSignal(int signalNo, SignalHandler handler);
