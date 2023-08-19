#include "ace/OS.h"
#include "ace/Process.h"

int install(){
    ACE_Process_Options options;
    char *n_env = 0;
    int n;

    options.command_line("rpm install a.rpm");

    ACE_Process child;
    child.spawn(options);
    child.wait();
    return child.exit_code();
}
