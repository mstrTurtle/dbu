#include <cstdio>
#include <cstdlib>
#include <iostream>

using std::cout;

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    {
      cout << "Try 'dbu --help' for more information.\n";
      exit (status);
    }

  cout << "\
Use '-D help' for a description of the options, or see find(1)\n\
\n";

  exit (status);
}
