#ifndef __CONSUMERS_H__
#define __CONSUMERS_H__

#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/StringRef.h"

#include "ControlDependency.h"
#include "Type.h"

class MyASTConsumer : public clang::ASTConsumer {
public:
  MyASTConsumer(llvm::StringRef functionName, clang::Rewriter &R,
                ControlDependency &out)
      : Visitor(R, out), target(functionName) {}
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR);

  MyASTVisitor Visitor;

private:
  llvm::StringRef target;
};

class DeclarationConsumer : public clang::ASTConsumer {
public:
  explicit DeclarationConsumer(llvm::StringRef functionName, Decl &out)
      : target(functionName), decl(out) {}

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR);

private:
  llvm::StringRef target;
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
