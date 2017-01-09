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
                    ss << QualType::getAsString(it->getType().split()) << ' ';
                    ss << it->getNameAsString();

                    i++;
                }
                ss << ");";
                //Get the source range and manager.
                SourceRange range = f->getCanonicalDecl()->getSourceRange();

                //Use LLVM's lexer to get source text.
                llvm::StringRef ref = Lexer::getSourceText(CharSourceRange::getCharRange(range), rewriter.getSourceMgr(), rewriter.getLangOpts());
                result = ss.str();
              }
            }
        }
        return true;
    }

    std::string getResult() {
        return result;
    }
private:
    StringRef target;
    Rewriter rewriter;
    std::string result;
};
