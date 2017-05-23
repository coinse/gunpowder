#ifndef __CAVM_H__
#define __CAVM_H__

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "Type.h"

static llvm::cl::OptionCategory Category("options");

class Cavm {
public:
  Cavm(std::string fileName, std::vector<const char *> &opts)
      : fileName(fileName)/*, opts(opts)*/ {
    int argc = opts.size();

    p = new clang::tooling::CommonOptionsParser(argc, opts.data(), Category);
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
  // std::vector<const char *> &opts;
};

#endif // __CAVM_H__
