// Copyright 2017 COINSE Lab.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/OperationKinds.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Analysis/CFG.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "./consumer.cpp"

typedef std::vector<std::tuple<int, int, bool>> ControlDependency;

class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
 public:
  explicit MyASTVisitor(clang::Rewriter &R, ControlDependency &out) : TheRewriter(R), cfg(out) { id = 0; }

  typedef std::vector<std::pair<clang::Stmt *, int>> branchidsty;
  branchidsty branchids;

  ControlDependency getControlDep() { return cfg; }

  struct isidExist {
    explicit isidExist(clang::Stmt *s) : _s(s) {}
    bool operator()(std::pair<clang::Stmt *, int> const &p) {
      return (p.first == _s);
    }
    clang::Stmt *_s;
  };

  int getStmtid(clang::Stmt *s) {
    branchidsty::iterator it =
        std::find_if(branchids.begin(), branchids.end(), isidExist(s));
    int stmtid = it->second;
    return stmtid;
  }

  int assignStmtid(clang::Stmt *s) {
    int stmtid;
    branchidsty::iterator it =
        std::find_if(branchids.begin(), branchids.end(), isidExist(s));
    if (it == branchids.end()) {
      stmtid = id;
      branchids.push_back(std::pair<clang::Stmt *, int>(s, id++));
    } else {  // if already assigned return assigned id
      stmtid = it->second;
    }
    return stmtid;
  }

  typedef std::vector<std::pair<clang::Stmt *, clang::Stmt *>> branchdepty;
  branchdepty branchdeps;

  struct isDepExist {
    explicit isDepExist(clang::Stmt *s) : _s(s) {}
    bool operator()(std::pair<clang::Stmt *, clang::Stmt *> const &p) {
      return (p.first == _s);
    }
    clang::Stmt *_s;
  };

  std::pair<clang::Stmt *, clang::Stmt *> getDep(clang::Stmt *s) {
    branchdepty::iterator it =
        std::find_if(branchdeps.begin(), branchdeps.end(), isDepExist(s));
    if (it == branchdeps.end()) {
      return std::pair<clang::Stmt *, clang::Stmt *>(s, NULL);
    }

    return *it;
  }

  void insertdep(clang::SourceLocation Loc, int stmtid, int parentid,
                 bool cond) {
    std::stringstream ss;
    ss << "/*";
    ss << parentid;
    ss << "->";
    ss << stmtid;
    ss << "*/\n";
    TheRewriter.InsertText(Loc, ss.str(), true, true);
    cfg.push_back(std::tuple<int, int, bool>(stmtid, parentid, cond));
  }

  int assignDep(clang::Stmt *s, clang::Stmt *parent, bool cond) {
    int stmtid = 0;
    int parentid = 0;

    if (clang::isa<clang::DoStmt>(s)) {
      stmtid = assignStmtid(s);
      parentid = getStmtid(parent);
      branchdeps.push_back(std::pair<clang::Stmt *, clang::Stmt *>(s, parent));

      insertdep(s->getLocStart(), stmtid, parentid, cond);
    } else if (clang::isa<clang::ForStmt>(s)) {
      stmtid = assignStmtid(s);
      parentid = getStmtid(parent);
      branchdeps.push_back(std::pair<clang::Stmt *, clang::Stmt *>(s, parent));

      insertdep(s->getLocStart(), stmtid, parentid, cond);
    } else if (clang::isa<clang::IfStmt>(s)) {
      stmtid = assignStmtid(s);
      parentid = getStmtid(parent);
      branchdeps.push_back(std::pair<clang::Stmt *, clang::Stmt *>(s, parent));

      insertdep(s->getLocStart(), stmtid, parentid, cond);
    } else if (clang::isa<clang::WhileStmt>(s)) {
      stmtid = assignStmtid(s);
      parentid = getStmtid(parent);
      branchdeps.push_back(std::pair<clang::Stmt *, clang::Stmt *>(s, parent));

      insertdep(s->getLocStart(), stmtid, parentid, cond);
    } else if (clang::isa<clang::CompoundStmt>(s)) {
      clang::CompoundStmt *C = clang::cast<clang::CompoundStmt>(s);
      for (auto *I : C->body()) {
        assignDep(I, parent, cond);
      }
    }

    return stmtid;
  }

  void insertbranchlog(clang::Expr *Cond, int stmtid) {
		if (Cond != nullptr) {
    std::string str;
    llvm::raw_string_ostream S(str);
    S << "inst(" << stmtid << ", ";
    convertCompositePredicate(Cond, S, TheRewriter);
    S << ")";
    TheRewriter.ReplaceText(Cond->getSourceRange(), S.str());
		}
  }

  void convertCompositePredicate(clang::Expr *Cond, llvm::raw_string_ostream &S,
                                 clang::Rewriter TheRewriter) {
    if (clang::isa<clang::BinaryOperator>(Cond)) {
      clang::BinaryOperator *o = clang::dyn_cast<clang::BinaryOperator>(Cond);
      clang::BinaryOperator::Opcode Opc = o->getOpcode();
      switch (Opc) {
        case clang::BO_GT:
          S << "isGreater(";
          break;
        case clang::BO_GE:
          S << "isEqGreater(";
          break;
        case clang::BO_LT:
          S << "isLess(";
          break;
        case clang::BO_LE:
          S << "isEqLess(";
          break;
        case clang::BO_EQ:
          S << "isEqual(";
          break;
        case clang::BO_NE:
          S << "isNotEqual(";
          break;
        case clang::BO_LAnd:
          S << "l_and(";
          break;
        case clang::BO_LOr:
          S << "l_or(";
          break;
        default:
          Cond->printPretty(S, nullptr,
                            clang::PrintingPolicy(TheRewriter.getLangOpts()));
          return;
      }
      convertCompositePredicate(o->getLHS(), S, TheRewriter);
      S << ", ";
      convertCompositePredicate(o->getRHS(), S, TheRewriter);
      S << ")";
    } else if (clang::isa<clang::ImplicitCastExpr>(Cond)) {
      clang::ImplicitCastExpr *c =
          clang::dyn_cast<clang::ImplicitCastExpr>(Cond);
      switch (c->getCastKind()) {
        case clang::CK_MemberPointerToBoolean:
        case clang::CK_PointerToBoolean:
        case clang::CK_IntegralToBoolean:
        case clang::CK_FloatingToBoolean:
        case clang::CK_FloatingComplexToBoolean:
          S << "isNotEqual(";
          Cond->printPretty(S, nullptr,
                            clang::PrintingPolicy(TheRewriter.getLangOpts()));
          S << ", 0)";
          break;
        default:
          convertCompositePredicate(c->getSubExpr(), S, TheRewriter);
      }
    } else if (clang::isa<clang::UnaryOperator>(Cond)) {
      clang::UnaryOperator *o = clang::dyn_cast<clang::UnaryOperator>(Cond);
      clang::UnaryOperator::Opcode Opc = o->getOpcode();
      switch (Opc) {
        case clang::UO_PreInc:
          S << "++";
          break;
        case clang::UO_PreDec:
          S << "--";
          break;
        case clang::UO_AddrOf:
          S << "&";
          break;
        case clang::UO_Deref:
          S << "*";
          break;
        case clang::UO_Plus:
          S << "+";
          break;
        case clang::UO_Minus:
          S << "-";
          break;
        case clang::UO_PostInc:
          convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
          S << "++";
          return;
        case clang::UO_PostDec:
          convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
          S << "--";
          return;
        case clang::UO_LNot:
          S << "l_not(";
          convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
          S << ")";
          return;
        default:
          return;
      }
      convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
    } else if (clang::isa<clang::ParenExpr>(Cond)) {
      clang::ParenExpr *o = clang::dyn_cast<clang::ParenExpr>(Cond);
      convertCompositePredicate(o->getSubExpr(), S, TheRewriter);
    } else {
      Cond->printPretty(S, nullptr,
                        clang::PrintingPolicy(TheRewriter.getLangOpts()));
    }
  }

  bool VisitStmt(clang::Stmt *s) {
    if (clang::isa<clang::DoStmt>(s)) {
      clang::DoStmt *F = clang::cast<clang::DoStmt>(s);
      int stmtid = assignStmtid(s);

      clang::Expr *Cond = F->getCond();
      clang::Stmt *Body = F->getBody();

      assignDep(Body, s, true);

      insertbranchlog(Cond, stmtid);
    } else if (clang::isa<clang::ForStmt>(s)) {
      clang::ForStmt *F = clang::cast<clang::ForStmt>(s);
      int stmtid = assignStmtid(s);

      clang::Expr *Cond = F->getCond();
      clang::Stmt *Body = F->getBody();

      assignDep(Body, s, true);

      insertbranchlog(Cond, stmtid);
    } else if (clang::isa<clang::IfStmt>(s)) {
      clang::IfStmt *I = clang::cast<clang::IfStmt>(s);
      int stmtid = assignStmtid(s);

      clang::Expr *Cond = I->getCond();
      clang::Stmt *Then = I->getThen();
      clang::Stmt *Else = I->getElse();

      assignDep(Then, s, true);
      if (Else) assignDep(Else, s, false);

      insertbranchlog(Cond, stmtid);
    } else if (clang::isa<clang::WhileStmt>(s)) {
      clang::WhileStmt *F = clang::cast<clang::WhileStmt>(s);
      int stmtid = assignStmtid(s);

      clang::Expr *Cond = F->getCond();
      clang::Stmt *Body = F->getBody();

      assignDep(Body, s, true);

      insertbranchlog(Cond, stmtid);
    }

    return true;
  }

  bool VisitFunctionDecl(clang::FunctionDecl *f) {
    TheRewriter.InsertTextAfter(f->getLocStart(), "extern \"C\"\n");
    return true;
  }

 private:
  clang::Rewriter &TheRewriter;
  int id;
  ControlDependency &cfg;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public clang::ASTConsumer {
 public:
  MyASTConsumer(StringRef functionName, clang::Rewriter &R, ControlDependency &out)
      : target(functionName), Visitor(R, out) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR) {
    for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
         ++b) {
      if (clang::FunctionDecl *f = clang::dyn_cast<clang::FunctionDecl>(*b)) {
        if (f->getName() == target) {
          // Traverse the declaration using our AST visitor.
          (*b)->dumpColor();
          Visitor.TraverseDecl(*b);
        }
      }
    }
    for (auto &it : Visitor.branchids) {
      if (Visitor.getDep(it.first).second == NULL) {
        int stmtid = Visitor.getStmtid(it.first);
        Visitor.insertdep(it.first->getLocStart(), stmtid, -1, 0);
      }
    }

    return true;
  }

  ControlDependency getControlDep() { return Visitor.getControlDep(); }

  MyASTVisitor Visitor;

 private:
  StringRef target;
};

class MyFrontendAction : public clang::ASTFrontendAction {
public:
	MyFrontendAction(StringRef funcName, ControlDependency &out)
		: funcName(funcName), out(out) {}
	virtual void EndSourceFileAction() {
		const clang::RewriteBuffer *RewriteBuf = r.getRewriteBufferFor(r.getSourceMgr().getMainFileID());
    std::string f = std::string(this->getCurrentFile());
    std::string filename = f.substr(0, f.find_last_of('.'));
    filename = filename + ".inst.cpp";
    std::ofstream out(filename.c_str());
    out << std::string(RewriteBuf->begin(), RewriteBuf->end());
	}
	virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
		clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    clang::SourceManager &SourceMgr = Compiler.getSourceManager();
		r.setSourceMgr(SourceMgr, Compiler.getLangOpts());
    r.InsertTextAfter(SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID())
      , "#include \"../util/branchdistance.cpp\"\n");
		return std::unique_ptr<clang::ASTConsumer>(new MyASTConsumer(funcName, r, out));
	}
private:
	StringRef funcName;
  ControlDependency &out;
	clang::Rewriter r;
};

template <typename C, typename T>
class ActionFactory : public clang::tooling::FrontendActionFactory {
public:
	ActionFactory(StringRef funcName)
		: funcName(funcName) {}
	clang::FrontendAction *create() override { return new C(funcName, result); }
  T getResult() { return result; }
private:
	StringRef funcName;
  T result;
};

static llvm::cl::OptionCategory Category("options");

ControlDependency instrument(StringRef fileName, StringRef functionName) {
  int argc = 3;
  const char *argv[] = { "clang", "input.c", "--" };

  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, Category);
  std::vector<std::string> Sources;
  Sources.push_back(fileName);

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), Sources);
	std::unique_ptr<ActionFactory<MyFrontendAction, ControlDependency>> f;
	f = std::unique_ptr<ActionFactory<MyFrontendAction, ControlDependency>>(
		new ActionFactory<MyFrontendAction, ControlDependency>(functionName));
  Tool.run(f.get());

  return (f.get())->getResult();
}

Decl getDeclaration(StringRef fileName, StringRef functionName) {
  int argc = 3;
  const char *argv[] = { "clang", "input.c", "--" };

  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, Category);
  std::vector<std::string> Sources;
  Sources.push_back(fileName);

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), Sources);
	std::unique_ptr<ActionFactory<DeclarationAction, Decl>> f;
	f = std::unique_ptr<ActionFactory<DeclarationAction, Decl>>(
		new ActionFactory<DeclarationAction, Decl>(functionName));
  Tool.run(f.get());

  return (f.get())->getResult();
}

void getFunctions(StringRef fileName) {
  int argc = 3;
  const char *argv[] = { "clang", "input.c", "--" };

  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, Category);
  std::vector<std::string> Sources;
  Sources.push_back(fileName);

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), Sources);
  Tool.run(clang::tooling::newFrontendActionFactory<FunctionListAction>().get());
}
