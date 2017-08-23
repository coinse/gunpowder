/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 * Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#include "./Cavm.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    llvm::errs() << "Usage: main <filename> <function name>\n";
    return 1;
  }
  Cavm c(argv[1]);
  ControlDependency cfg = c.instrument(argv[2]);
  for (const auto &i : cfg) {
    std::cout << std::get<0>(i) << std::get<1>(i) << std::get<2>(i)
              << std::endl;
  }
  std::cout << std::get<0>(c.getDeclaration(argv[2])) << std::endl;
  return 0;
}
