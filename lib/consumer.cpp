// Copyright 2017 COINSE Lab.
#include <string>
#include <vector>

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Analysis/CFG.h"

class DeclarationConsumer : public clang::ASTConsumer {
 public:
  DeclarationConsumer(StringRef functionName, clang::Rewriter &R)
      : target(functionName), rewriter(R) {}

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
                std::string t =
                    clang::QualType::getAsString(it->getType().split());
                params.push_back(t);
                ss << t << ' ';
                ss << it->getNameAsString();

                i++;
              }
              ss << ");";
              decl = ss.str();
            }
          } else if (clang::RecordDecl *f =
                         clang::dyn_cast<clang::RecordDecl>(*b)) {
            // Use LLVM's lexer to get source text.
            clang::SourceLocation b(f->getLocStart()), _e(f->getLocEnd());
            clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(
                _e, 0, rewriter.getSourceMgr(), rewriter.getLangOpts()));
            clang::SourceRange r(b, e);
            llvm::StringRef ref = clang::Lexer::getSourceText(
                clang::CharSourceRange::getCharRange(r),
                rewriter.getSourceMgr(), rewriter.getLangOpts());
            decl = ref.str() + ";";
            for (const auto &i : f->fields()) {
              std::string t =
                  clang::QualType::getAsString(i->getType().split());
              params.push_back(t);
            }
          }
        }
      }
    }
    return true;
  }

  std::string getDeclarationString() { return decl; }

  std::vector<std::string> getParams() { return params; }

 private:
  StringRef target;
  clang::Rewriter &rewriter;
  std::string decl;
  std::vector<std::string> params;
};

class FunctionConsumer : public clang::ASTConsumer {
 public:
  FunctionConsumer() {}

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR) {
    for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
         ++b) {
      if (clang::FunctionDecl *f = clang::dyn_cast<clang::FunctionDecl>(*b)) {
          decls.push_back(f->getNameAsString());
          std::cout << f->getNameAsString() << std::endl;
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
