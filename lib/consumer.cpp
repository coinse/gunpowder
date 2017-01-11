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
            if (FunctionDecl *f = dyn_cast<FunctionDecl>(*b)) {
              if (f->hasBody() && f->getName() == target) {
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
                //Get the source range and manager.
                SourceRange range = f->getCanonicalDecl()->getSourceRange();

                //Use LLVM's lexer to get source text.
                llvm::StringRef ref = Lexer::getSourceText(CharSourceRange::getCharRange(range), rewriter.getSourceMgr(), rewriter.getLangOpts());
                decl = ss.str();
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
    Rewriter rewriter;
    std::string decl;
    std::vector<std::string> params;
};
