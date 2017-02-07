// Copyright 2017 COINSE Lab.
#include <iostream>
#include "./buildcfg.cpp"
#include "llvm/Support/raw_ostream.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    llvm::errs() << "Usage: main <filename> <function name>\n";
    return 1;
  }
  CAVM c(argv[1]);
  ControlDependency cfg = c.instrument(argv[2]);
  for (const auto &i : cfg) {
    std::cout << std::get<0>(i) << std::get<1>(i) << std::get<2>(i)
              << std::endl;
  }
  std::cout << std::get<0>(c.getDeclaration(argv[2])) << std::endl;
  return 0;
}
