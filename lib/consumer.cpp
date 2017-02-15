// Copyright 2017 COINSE Lab.
#include <string>
#include <vector>

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Analysis/CFG.h"

typedef std::tuple<std::string, std::vector<std::string>> Decl;

class DeclarationConsumer : public clang::ASTConsumer {
 public:
  DeclarationConsumer(StringRef functionName, clang::Rewriter &R, Decl &out)
      : target(functionName), rewriter(R), decl(out) {}

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR) {
    for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
         ++b) {
      // (*b)->dump();
      if (clang::NamedDecl *d = clang::dyn_cast<clang::NamedDecl>(*b)) {
        if (d->getName() == target) {
          if (clang::FunctionDecl *f =
                  clang::dyn_cast<clang::FunctionDecl>(*b)) {
            if (f->hasBody()) {
              std::stringstream ss;
              ss << clang::QualType::getAsString(f->getReturnType().split())
                 << ' ';
              ss << f->getNameAsString();
              ss << '(';
              int i = 0;
              for (auto &it : f->parameters()) {
                if (i > 0) ss << ',';
                std::string t = clang::QualType::getAsString(
                    it->getType().getCanonicalType().split());
                std::get<1>(decl).push_back(t);
                ss << t << ' ';
                ss << it->getNameAsString();

                i++;
              }
              ss << ");";
              std::get<0>(decl) = ss.str();
            }
          } else if (clang::RecordDecl *f =
                         clang::dyn_cast<clang::RecordDecl>(*b)) {
            std::stringstream ss;
            ss << "struct ";
            ss << f->getNameAsString();
            ss << "{";
            for (const auto &it : f->fields()) {
              std::string t = clang::QualType::getAsString(
                  it->getType().getCanonicalType().split());
              std::get<1>(decl).push_back(t);
              ss << t << ' ';
              ss << it->getNameAsString();
              ss << "; ";
            }
            ss << "};";
            std::get<0>(decl) = ss.str();
          }
        }
      }
    }
    return true;
  }

 private:
  StringRef target;
  clang::Rewriter &rewriter;
  Decl &decl;
};

class DeclarationAction : public clang::ASTFrontendAction {
  public:
    DeclarationAction(StringRef funcName, Decl &out)
      : funcName(funcName), out(out) {}
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
      r.setSourceMgr(Compiler.getSourceManager(), Compiler.getLangOpts());
      return std::unique_ptr<clang::ASTConsumer>(new DeclarationConsumer(funcName, r, out));
    }
  private:
    StringRef funcName;
    Decl &out;
    clang::Rewriter r;
};

class FunctionConsumer : public clang::ASTConsumer {
 public:
  FunctionConsumer() {}

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR) {
    for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
         ++b) {
      if (clang::FunctionDecl *f = clang::dyn_cast<clang::FunctionDecl>(*b)) {
          decls.push_back(f->getNameAsString());
          std::cout << "\t" << f->getNameAsString() << std::endl;
      }
    }
    return true;
  }

  std::vector<std::string> getFunctions() { return decls; }

 private:
  std::vector<std::string> decls;
};

class FunctionListAction : public clang::ASTFrontendAction {
  public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
      return std::unique_ptr<clang::ASTConsumer>(new FunctionConsumer);
    }
};
