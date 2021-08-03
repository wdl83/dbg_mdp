#include <cstring>

#include "HijackSignal.h"
#include "Trace.h"

void hijackSignal(int signalNo, SignalHandler handler)
{
     struct sigaction action;

     sigemptyset(&action.sa_mask);
     action.sa_handler = handler;
     action.sa_flags = SA_RESETHAND | SA_NODEFER;

     if(0 != ::sigaction(signalNo, &action, nullptr))
     {
          TRACE(
              TraceLevel::Error,
              strsignal(signalNo), '(', signalNo, ')');
     }
}
