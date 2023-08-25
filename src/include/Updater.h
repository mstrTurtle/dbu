#include "Sniffer.h"
#include "Option.h"

class Updater
{
public:
    Updater(Option& option_): option(option_) {}
    int run();

private:
    Option option;
};
