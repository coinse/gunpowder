#ifndef __CONSUMERS_H__
#define __CONSUMERS_H__

#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Tooling.h"

#include "ControlDependency.h"

typedef std::tuple<std::string, std::vector<std::string>> Decl;


class MyASTConsumer : public clang::ASTConsumer {
public:
  MyASTConsumer(StringRef functionName, clang::Rewriter &R,
                ControlDependency &out)
      : target(functionName), Visitor(R, out) {}
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR);

  MyASTVisitor Visitor;

private:
  StringRef target;
};

class DeclarationConsumer : public clang::ASTConsumer {
public:
  explicit DeclarationConsumer(StringRef functionName, Decl &out)
      : target(functionName), decl(out) {}

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR);

private:
  StringRef target;
  Decl &decl;
};

class FunctionConsumer : public clang::ASTConsumer {
public:
  FunctionConsumer() {}

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR);

  std::vector<std::string> getFunctions();

private:
  std::vector<std::string> decls;
};

#endif // __CONSUMERS_H__
