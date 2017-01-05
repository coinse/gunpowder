#include <iostream>
#include "llvm/Support/raw_ostream.h"
#include "./buildcfg.cpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        llvm::errs() << "Usage: main <filename>\n";
        return 1;
    }
    ControlDependency cfg = instrument(argv[1]);
    for (const auto &i : cfg) {
      std::cout<<std::get<0>(i)<<std::get<1>(i)<<std::get<2>(i)<<std::endl;
    }
    return 0;
}
