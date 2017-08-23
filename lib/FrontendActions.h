/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#ifndef __FRONTACTIONS_H__
#define __FRONTACTIONS_H__

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Tooling.h"

#include "Consumers.h"

template <typename C, typename T>
class ActionFactory : public clang::tooling::FrontendActionFactory {
public:
  ActionFactory(StringRef funcName) : funcName(funcName) {}
  clang::FrontendAction *create() override { return new C(funcName, result); }
  T getResult() { return result; }

private:
  StringRef funcName;
  T result;
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
  MyFrontendAction(StringRef funcName, ControlDependency &out)
      : funcName(funcName), out(out) {}
  virtual void EndSourceFileAction();
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);

private:
  StringRef funcName;
  ControlDependency &out;
  clang::Rewriter r;
};

class DeclarationAction : public clang::ASTFrontendAction {
public:
  DeclarationAction(StringRef funcName, Decl &out)
      : funcName(funcName), out(out) {}
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);

private:
  StringRef funcName;
  Decl &out;
};

class FunctionListAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);
};

#endif // __FRONTACTIONS_H__
