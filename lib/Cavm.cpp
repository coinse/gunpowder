// Copyright 2017 COINSE Lab.

#include "Cavm.h"
#include "Consumers.h"
#include "ControlDependency.h"
#include "FrontendActions.h"

ControlDependency Cavm::instrument(std::string functionName) {
  std::unique_ptr<ActionFactory<MyFrontendAction, ControlDependency>> f;
  f = std::unique_ptr<ActionFactory<MyFrontendAction, ControlDependency>>(
      new ActionFactory<MyFrontendAction, ControlDependency>(functionName));
  tool->run(f.get());

  return (f.get())->getResult();
}

Decl Cavm::getDeclaration(std::string functionName) {
  std::unique_ptr<ActionFactory<DeclarationAction, Decl>> f;
  f = std::unique_ptr<ActionFactory<DeclarationAction, Decl>>(
      new ActionFactory<DeclarationAction, Decl>(functionName));
  tool->run(f.get());

  return (f.get())->getResult();
}

void Cavm::printFunctions() {
  tool->run(
      clang::tooling::newFrontendActionFactory<FunctionListAction>().get());
}
