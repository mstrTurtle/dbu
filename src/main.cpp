#include "ace/Log_Msg.h"
#include "Updater.h"
#include "Option.h"

int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
  Updater updater;

  if (updater.run (argc, argv) == -1)
    ACE_ERROR_RETURN ((LM_ERROR, "%p\n", "updater.run()"), 1);
  return 0;
}
