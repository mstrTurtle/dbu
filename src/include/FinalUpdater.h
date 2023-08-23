#include "Sniffer.h"
#include "Option.h"

inline SniffHint
convertOptionToSniffHint(const Option& option)
{
  SniffHint sniffHint = { .branch = option.branch_,
                          .subbranch = option.sub_branch_,
                          .option = option.build_,
                          .arch = option.arch_,
                          .product = option.product_ };
  return sniffHint;
}
