#include "sniffer.h"
#include "option.h"

class Updater
{
public:
    Updater(Option& option_): option(option_) {}
    int run();

private:
    Option option;
};
