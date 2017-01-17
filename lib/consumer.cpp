#include <string>

#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTConsumer.h"

using namespace clang;

class DeclarationConsumer : public ASTConsumer {
public:
    DeclarationConsumer(StringRef functionName, Rewriter &R)
      : target(functionName), rewriter(R) {}

    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
            //(*b)->dump();
            if (NamedDecl *d = dyn_cast<NamedDecl>(*b)) {
              if (d->getName() == target) {
                if (FunctionDecl *f = dyn_cast<FunctionDecl>(*b)) {
                  if(f->hasBody()) {
                    std::stringstream ss;
                    ss << QualType::getAsString(f->getReturnType().split()) << ' ';
                    ss << f->getNameAsString();
                    ss << '(';
                    int i = 0;
                    for (auto &it : f->parameters()) {
                        if (i > 0)
                          ss << ',';
                        std::string t = QualType::getAsString(it->getType().split());
                        params.push_back(t);
                        ss << t << ' ';
                        ss << it->getNameAsString();

                        i++;
                    }
                    ss << ");";
                    decl = ss.str();
                  }
                } else if (RecordDecl *f = dyn_cast<RecordDecl>(*b)) {
                  //Use LLVM's lexer to get source text.
                  SourceLocation b(f->getLocStart()), _e(f->getLocEnd());
                  SourceLocation e(Lexer::getLocForEndOfToken(_e, 0, rewriter.getSourceMgr(), rewriter.getLangOpts()));
                  SourceRange r(b, e);
                  llvm::StringRef ref = Lexer::getSourceText(CharSourceRange::getCharRange(r), rewriter.getSourceMgr(), rewriter.getLangOpts());
                  decl = ref.str() + ";";
                  for (const auto &i : f->fields()) {
                    std::string t = QualType::getAsString(i->getType().split());
                    params.push_back(t);
                  }
                }
              }
            }
        }
        return true;
    }

    std::string getDeclarationString() {
        return decl;
    }

    std::vector<std::string> getParams() {
        return params;
    }
private:
    StringRef target;
    Rewriter &rewriter;
    std::string decl;
    std::vector<std::string> params;
};
