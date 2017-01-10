#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/OperationKinds.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "./consumer.cpp"

using namespace clang;

typedef std::vector<std::tuple<int, int, bool>> ControlDependency;

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
    MyASTVisitor(Rewriter &R) : TheRewriter(R) {id = 0;}

    typedef std::vector<std::pair<Stmt*, int>> branchidsty;
    branchidsty branchids;

    ControlDependency getControlDep() {
        return cfg;
    }

    struct isidExist {
        isidExist(Stmt *s): _s(s) {}
        bool operator() (std::pair<Stmt*, int> const& p) {
            return (p.first == _s);
        }
        Stmt* _s;
    };

    int getStmtid(Stmt *s){
        branchidsty::iterator it = std::find_if(branchids.begin(), branchids.end(), isidExist(s));
        int stmtid = it->second;
        return stmtid;
    }

    int assignStmtid(Stmt *s){
        int stmtid;
        branchidsty::iterator it = std::find_if(branchids.begin(), branchids.end(), isidExist(s));
        if (it == branchids.end()) {
            stmtid = id;
            branchids.push_back(std::pair<Stmt*, int>(s, id++));
        }
        else {//if already assigned return assigned id
            stmtid = it->second;
        }
        return stmtid;
    }


    typedef std::vector<std::pair<Stmt*, Stmt*>> branchdepty;
    branchdepty branchdeps;

    struct isDepExist {
        isDepExist(Stmt *s): _s(s) {}
        bool operator() (std::pair<Stmt*, Stmt*> const& p) {
            return (p.first == _s);
        }
        Stmt* _s;
    };

    std::pair<Stmt*, Stmt*> getDep(Stmt *s) {
        branchdepty::iterator it = std::find_if(branchdeps.begin(), branchdeps.end(), isDepExist(s));
        if (it == branchdeps.end()) {
            return std::pair<Stmt*, Stmt*>(s, NULL);
        }

        return *it;
    }

    void insertdep(SourceLocation Loc, int stmtid, int parentid, bool cond) {
        std::stringstream ss;
        ss << "/*";
        ss << parentid;
        ss << "->";
        ss << stmtid;
        ss << "*/\n";
        TheRewriter.InsertText(Loc, ss.str(), true, true);
        cfg.push_back(std::tuple<int, int, bool>(stmtid, parentid, cond));
    }

    int assignDep (Stmt *s, Stmt *parent, bool cond) {
        int stmtid = 0;
        int parentid = 0;

        if(isa<DoStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid, cond);
        }
        else if(isa<ForStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid, cond);
        }
        else if(isa<IfStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid, cond);
        }
        else if(isa<WhileStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid, cond);
        }
        else if(isa<CompoundStmt>(s)) {
            CompoundStmt *C = cast<CompoundStmt>(s);
            for(auto *I: C->body()){
                 assignDep(I, parent, cond);
            }
        }

        return stmtid;
    }

    void insertbranchlog(Expr *Cond, int stmtid) {
      std::string str;
      llvm::raw_string_ostream S(str);
      S << "inst(" << stmtid << ", ";
      convertCompositePredicate(Cond, S, TheRewriter);
      S << ")" ;
      TheRewriter.ReplaceText(Cond->getSourceRange(), S.str());
    }

    void convertCompositePredicate(Expr *Cond, llvm::raw_string_ostream& S, Rewriter TheRewriter) {
      if (isa<BinaryOperator>(Cond)) {
        BinaryOperator *o = dyn_cast<BinaryOperator>(Cond);
        BinaryOperator::Opcode Opc = o->getOpcode();
        switch (Opc) {
          case BO_GT:
            S << "isGreater(";
            break;
          case BO_GE:
            S << "isEqGreater(";
            break;
          case BO_LT:
            S << "isLess(";
            break;
          case BO_LE:
            S << "isEqLess(";
            break;
          case BO_EQ:
            S << "isEqual(";
            break;
          case BO_NE:
            S << "isnotEqual(";
            break;
          case BO_LAnd:
            S << "l_and(";
            break;
          case BO_LOr:
            S << "l_or(";
            break;
          default:
            Cond->printPretty(S, nullptr, PrintingPolicy(TheRewriter.getLangOpts()));
            return;
        }
        convertCompositePredicate(o->getLHS(), S, TheRewriter);
        S << ", ";
        convertCompositePredicate(o->getRHS(), S, TheRewriter);
        S << ")";
      }
      else if(isa<UnaryOperator>(Cond)) {
        UnaryOperator *o = dyn_cast<UnaryOperator>(Cond);
        UnaryOperator::Opcode Opc = o->getOpcode();
        switch(Opc){
          case UO_LNot:
            S << "l_not(";
            break;
          default:
            return;
        }
        convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
        S << ")";

      }
      else if(isa<ParenExpr>(Cond)){
        ParenExpr *o = dyn_cast<ParenExpr>(Cond);
        convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
      }
      else{
        Cond->printPretty(S, nullptr, PrintingPolicy(TheRewriter.getLangOpts()));
      }
    }

    bool VisitStmt(Stmt *s) {
        if(isa<DoStmt>(s)) {
            DoStmt *F = cast<DoStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = F->getCond();
            Stmt *Body = F->getBody();

            assignDep(Body, s, true);

            insertbranchlog(Cond, stmtid);
        }
        else if(isa<ForStmt>(s)) {
            ForStmt *F = cast<ForStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = F->getCond();
            Stmt *Body = F->getBody();

            assignDep(Body, s, true);

            insertbranchlog(Cond, stmtid);
        }
        else if(isa<IfStmt>(s)) {
            IfStmt *I = cast<IfStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = I->getCond();
            Stmt *Then = I->getThen();
            Stmt *Else = I->getElse();

            assignDep(Then, s, true);
            if(Else)
                assignDep(Else, s, false);

            insertbranchlog(Cond, stmtid);
        }
        else if(isa<WhileStmt>(s)) {
            WhileStmt *F = cast<WhileStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = F->getCond();
            Stmt *Body = F->getBody();

            assignDep(Body, s, true);

            insertbranchlog(Cond, stmtid);
        }

        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *f) {
        TheRewriter.InsertTextAfter(f->getLocStart(), "extern \"C\"\n");
        return true;
    }

private:
    Rewriter &TheRewriter;
    int id;
    ControlDependency cfg;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(StringRef functionName, Rewriter &R) 
      : target(functionName), Visitor(R) {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b){
            if (FunctionDecl *f = dyn_cast<FunctionDecl>(*b)) {
              if (f->getName() == target) {
                // Traverse the declaration using our AST visitor.
                Visitor.TraverseDecl(*b);
              }
            }
        }
        return true;
    }

    ControlDependency getControlDep() {
        return Visitor.getControlDep();
    }

    MyASTVisitor Visitor;
private:
    StringRef target;
};

class CAVM {
public:
    CAVM(StringRef fileName) : fileName(fileName) {
        // CompilerInstance will hold the instance of the Clang compiler for us,
        // managing the various objects needed to run the compiler.
        TheCompInst.createDiagnostics();

        LangOptions &lo = TheCompInst.getLangOpts();
        lo.CPlusPlus = 1;

        // Initialize target info with the default triple for our platform.
        auto TO = std::make_shared<TargetOptions>();
        TO->Triple = llvm::sys::getDefaultTargetTriple();
        TargetInfo *TI =
                TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
        TheCompInst.setTarget(TI);

        TheCompInst.createFileManager();
        FileManager &FileMgr = TheCompInst.getFileManager();
        TheCompInst.createSourceManager(FileMgr);
        SourceManager &SourceMgr = TheCompInst.getSourceManager();
        TheCompInst.createPreprocessor(TU_Module);
        TheCompInst.createASTContext();

        // A Rewriter helps us manage the code rewriting task.
        TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

        // Set the main file handled by the source manager to the input file.
        const FileEntry *FileIn = FileMgr.getFile(fileName);
        SourceMgr.setMainFileID(
                SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
        TheCompInst.getDiagnosticClient().BeginSourceFile(
                TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

    }

    ControlDependency instrument(StringRef functionName) {
        // Create an AST consumer instance which is going to get called by
        // ParseAST.
        MyASTConsumer TheConsumer(functionName, TheRewriter);
        SourceManager &SourceMgr = TheCompInst.getSourceManager();

        TheRewriter.InsertTextAfter(SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID()), "#include \"../util/branchdistance.cpp\"\n");
        // Parse the file to AST, registering our consumer as the AST consumer.
        ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
            TheCompInst.getASTContext());
        MyASTVisitor Visitor = TheConsumer.Visitor;
        for(auto &it : Visitor.branchids){
            if (Visitor.getDep(it.first).second == NULL){
                int stmtid = Visitor.getStmtid(it.first);
                Visitor.insertdep(it.first->getLocStart(), stmtid, -1, 0);
            }
        }

        // At this point the rewriter's buffer should be full with the rewritten
        // file contents.
        const RewriteBuffer *RewriteBuf =
          TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
        std::string f = std::string(fileName);
        std::string filename = f.substr(0, f.find_last_of('.'));
        filename = filename + ".inst.cpp";
        std::ofstream out(filename.c_str());
        out << std::string(RewriteBuf->begin(), RewriteBuf->end());

        return TheConsumer.getControlDep();
    }

    std::string getDeclaration(StringRef functionName) {
        // TODO: Find a way to avoid resetting ASTContext.
        TheCompInst.createASTContext();
        DeclarationConsumer TheConsumer(functionName, TheRewriter);

        // Parse the file to AST, registering our consumer as the AST consumer.
        ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
            TheCompInst.getASTContext());

        return TheConsumer.getResult();
    }

private:
    CompilerInstance TheCompInst;
    Rewriter TheRewriter;
    StringRef fileName;
};

