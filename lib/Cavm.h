#ifndef __CAVM_H__
#define __CAVM_H__

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "Type.h"

static llvm::cl::OptionCategory Category("options");

class Cavm {
public:
  Cavm(std::string fileName) : fileName(fileName) {
    int argc = 3;
    const char *argv[] = {"clang", "input.c", "--"};

    p = new clang::tooling::CommonOptionsParser(argc, argv, Category);
    Sources.push_back(fileName);

    tool = new clang::tooling::ClangTool(p->getCompilations(), Sources);
  }

  ControlDependency instrument(std::string funcName);

  Decl getDeclaration(std::string funcName);

  void printFunctions();

private:
  clang::tooling::CommonOptionsParser *p;
  clang::tooling::ClangTool *tool;
  std::string fileName;
  std::vector<std::string> Sources;
};

#endif // __CAVM_H__
