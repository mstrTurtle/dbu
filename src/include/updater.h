#include "sniffer.h"
#include "option.h"

class Updater
{
public:
    Updater(Option& option_): option(option_) {}
    [[nodiscard]] int run();

private:
    Option option;
};
