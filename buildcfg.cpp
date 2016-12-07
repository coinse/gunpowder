#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>

#include "clang/Analysis/CFG.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
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

using namespace clang;

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
    MyASTVisitor(Rewriter &R) : TheRewriter(R) {id = 0;}

    typedef std::vector<std::pair<Stmt*, int>> branchidsty;
    branchidsty branchids;

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

    void insertdep(SourceLocation Loc, int stmtid, int parentid) {
        std::stringstream ss;
        ss << "/*";
        ss << parentid;
        ss << "->";
        ss << stmtid;
        ss << "*/\n";
        TheRewriter.InsertText(Loc, ss.str(), true, true);

        std::ofstream ofs;
        ofs.open("controldep.txt", std::ofstream::out | std::ofstream::app);
        ofs << stmtid;
        ofs << " ";
        ofs << parentid;
        ofs << "\n";
        ofs.close();
    }

    int assignDep (Stmt *s, Stmt *parent) {
        int stmtid;
        int parentid;

        if(isa<DoStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid);
        }
        else if(isa<ForStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid);
        }
        else if(isa<IfStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid);
        }
        else if(isa<WhileStmt>(s)) {
            stmtid = assignStmtid(s);
            parentid = getStmtid(parent);
            branchdeps.push_back(std::pair<Stmt*, Stmt*>(s, parent));

            insertdep(s->getLocStart(), stmtid, parentid);
        }
        else if(isa<CompoundStmt>(s)) {
            CompoundStmt *C = cast<CompoundStmt>(s);
            for(auto *I: C->body()){
                 assignDep(I, parent);
            }
        }

        return stmtid;
    }

    void insertbranchlog(Expr *Cond, int stmtid) {
        std::stringstream ss;
        ss << "tracelog(";
        ss << stmtid;
        ss << ") && ";

        TheRewriter.InsertTextAfter(Cond->getLocStart(), ss.str());
    }

    bool VisitStmt(Stmt *s) {
        if(isa<DoStmt>(s)) {
            DoStmt *F = cast<DoStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = F->getCond();
            Stmt *Body = F->getBody();

            assignDep(Body, s);

            insertbranchlog(Cond, stmtid);
        }
        else if(isa<ForStmt>(s)) {
            ForStmt *F = cast<ForStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = F->getCond();
            Stmt *Body = F->getBody();

            assignDep(Body, s);

            insertbranchlog(Cond, stmtid);
        }
        else if(isa<IfStmt>(s)) {
            IfStmt *I = cast<IfStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = I->getCond();
            Stmt *Then = I->getThen();
            Stmt *Else = I->getElse();

            assignDep(Then, s);
            if(Else)
                assignDep(Else, s);

            insertbranchlog(Cond, stmtid);
        }
        else if(isa<WhileStmt>(s)) {
            WhileStmt *F = cast<WhileStmt>(s);
            int stmtid = assignStmtid(s);

            Expr *Cond = F->getCond();
            Stmt *Body = F->getBody();

            assignDep(Body, s);

            insertbranchlog(Cond, stmtid);
        }

        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *f) {
        return true;
    }

private:
    Rewriter &TheRewriter;
    int id;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &R) : Visitor(R) {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
            // Traverse the declaration using our AST visitor.
            Visitor.TraverseDecl(*b);
        return true;
    }

private:
    MyASTVisitor Visitor;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        llvm::errs() << "Usage: main <filename>\n";
        return 1;
    }

    // CompilerInstance will hold the instance of the Clang compiler for us,
    // managing the various objects needed to run the compiler.
    CompilerInstance TheCompInst;
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
    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

    // Set the main file handled by the source manager to the input file.
    const FileEntry *FileIn = FileMgr.getFile(argv[1]);
    SourceMgr.setMainFileID(
            SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
    TheCompInst.getDiagnosticClient().BeginSourceFile(
            TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

    // Create an AST consumer instance which is going to get called by
    // ParseAST.
    MyASTConsumer TheConsumer(TheRewriter);

	// Parse the file to AST, registering our consumer as the AST consumer.
	ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
			TheCompInst.getASTContext());

	// At this point the rewriter's buffer should be full with the rewritten
	// file contents.
	const RewriteBuffer *RewriteBuf =
		TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
	llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());


	return 0;
}
