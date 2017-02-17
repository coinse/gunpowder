// Copyright 2017 COINSE Lab.
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/OperationKinds.h"
#include "clang/Analysis/CFG.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "Consumers.h"
#include "ControlDependency.h"
#include "FrontendActions.h"

static llvm::cl::OptionCategory Category("options");

ControlDependency instrument(StringRef fileName, StringRef functionName) {
  int argc = 3;
  const char *argv[] = {"clang", "input.c", "--"};

  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, Category);
  std::vector<std::string> Sources;
  Sources.push_back(fileName);

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), Sources);
  std::unique_ptr<ActionFactory<MyFrontendAction, ControlDependency>> f;
  f = std::unique_ptr<ActionFactory<MyFrontendAction, ControlDependency>>(
      new ActionFactory<MyFrontendAction, ControlDependency>(functionName));
  Tool.run(f.get());

  return (f.get())->getResult();
}

Decl getDeclaration(StringRef fileName, StringRef functionName) {
  int argc = 3;
  const char *argv[] = {"clang", "input.c", "--"};

  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, Category);
  std::vector<std::string> Sources;
  Sources.push_back(fileName);

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), Sources);
  std::unique_ptr<ActionFactory<DeclarationAction, Decl>> f;
  f = std::unique_ptr<ActionFactory<DeclarationAction, Decl>>(
      new ActionFactory<DeclarationAction, Decl>(functionName));
  Tool.run(f.get());

  return (f.get())->getResult();
}

void getFunctions(StringRef fileName) {
  int argc = 3;
  const char *argv[] = {"clang", "input.c", "--"};

  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, Category);
  std::vector<std::string> Sources;
  Sources.push_back(fileName);

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), Sources);
  Tool.run(
      clang::tooling::newFrontendActionFactory<FunctionListAction>().get());
}
