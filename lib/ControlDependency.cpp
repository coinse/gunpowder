#include <iostream>
#include <sstream>
#include "ControlDependency.h"

bool MyASTVisitor::hasReturnStmt(clang::Stmt *s) {
  bool result = false;
  if (s) {
    if (clang::isa<clang::CompoundStmt>(s)) {
      clang::CompoundStmt *C = clang::cast<clang::CompoundStmt>(s);
      for (auto *I : C->body()) {
        if (clang::isa<clang::ReturnStmt>(I))
          result = true;
      }
    }
    else if (clang::isa<clang::ReturnStmt>(s)) {
      result = true;
    }
  }
  return result;
}

int MyASTVisitor::getStmtid(clang::Stmt *s) {
  int stmtid;
  branchidsty::iterator it =
      std::find_if(branchids.begin(), branchids.end(), isidExist(s));
  if (it == branchids.end()) {
    stmtid = 0;
  } else { // if already assigned return assigned id
    stmtid = it->second;
  }
  return stmtid;
}

int MyASTVisitor::assignStmtid(clang::Stmt *s) {
  int stmtid;
  branchidsty::iterator it =
      std::find_if(branchids.begin(), branchids.end(), isidExist(s));
  if (it == branchids.end()) {
    stmtid = id;
    branchids.push_back(std::pair<clang::Stmt *, int>(s, id++));
  } else { // if already assigned return assigned id
    stmtid = it->second;
  }
  return stmtid;
}

void MyASTVisitor::insertdep(clang::SourceLocation Loc, int stmtid,
                             int parentid, bool cond) {
  std::stringstream ss;
  ss << "/*";
  ss << parentid;
  ss << "->";
  ss << stmtid;
  ss << "*/\n";
  TheRewriter.InsertText(Loc, ss.str(), true, true);
}

int MyASTVisitor::assignDep(clang::Stmt *s, clang::Stmt *parent, bool cond) {
  int stmtid = 0;
  int parentid = 0;
  clang::Expr *Cond = nullptr;

  if (clang::isa<clang::DoStmt>(s)) {
    clang::DoStmt *F = clang::cast<clang::DoStmt>(s);
    Cond = F->getCond();
  }
  if (clang::isa<clang::ForStmt>(s)) {
    clang::ForStmt *F = clang::cast<clang::ForStmt>(s);
    Cond = F->getCond();
  }
  if (clang::isa<clang::IfStmt>(s)) {
    clang::IfStmt *F = clang::cast<clang::IfStmt>(s);
    Cond = F->getCond();
  }
  if (clang::isa<clang::WhileStmt>(s)) {
    clang::WhileStmt *F = clang::cast<clang::WhileStmt>(s);
    Cond = F->getCond();
  }
  if (Cond) {
    stmtid = assignStmtid(s);
    parentid = getStmtid(parent);
    if (cfg.find(stmtid) == cfg.end()) {
      cfg[stmtid] = std::pair<int, bool>(parentid, cond);
    }
  }
  if (clang::isa<clang::CompoundStmt>(s)) {
    clang::CompoundStmt *C = clang::cast<clang::CompoundStmt>(s);
    for (auto *I : C->body()) {
      assignDep(I, parent, cond);
    }
  }
  return stmtid;
}

void MyASTVisitor::insertbranchlog(clang::Expr *Cond, int stmtid) {
  if (Cond != nullptr) {
    std::string str;
    llvm::raw_string_ostream S(str);
    S << "inst(" << stmtid << ", ";
    handleImplicitCasting(Cond, S, TheRewriter);
    S << ")";
    TheRewriter.ReplaceText(
        (TheRewriter.getSourceMgr()).getExpansionRange(Cond->getSourceRange()),
        S.str());
  }
}

void MyASTVisitor::makeUntypedPointer(clang::Expr *expr,
                                      llvm::raw_string_ostream &S,
                                      clang::Rewriter rewriter) {
  if (clang::isa<clang::PointerType>(expr->getType().getCanonicalType()))
    S << "(void *)";
}

void MyASTVisitor::handleImplicitCasting(clang::Expr *expr,
                                         llvm::raw_string_ostream &S,
                                         clang::Rewriter rewriter) {
  if (clang::isa<clang::ParenExpr>(expr)) {
    clang::ParenExpr *e = clang::dyn_cast<clang::ParenExpr>(expr);
    return handleImplicitCasting(e->getSubExpr(), S, rewriter);
  }

  if (clang::isa<clang::BinaryOperator>(expr)) {
    clang::BinaryOperator *o = clang::dyn_cast<clang::BinaryOperator>(expr);
    clang::BinaryOperator::Opcode Opc = o->getOpcode();
    switch (Opc) {
    case clang::BO_Mul:
    case clang::BO_Div:
    case clang::BO_Rem:
    case clang::BO_Add:
    case clang::BO_Sub:
    case clang::BO_Shl:
    case clang::BO_Shr:
    case clang::BO_And:
    case clang::BO_Xor:
    case clang::BO_Or:
      break;
    default:
      makeUntypedPointer(expr, S, rewriter);
      return convertCompositePredicate(expr, S, rewriter);
    }
  }
  if (clang::isa<clang::UnaryOperator>(expr)) {
    clang::UnaryOperator *o = clang::dyn_cast<clang::UnaryOperator>(expr);
    clang::UnaryOperator::Opcode Opc = o->getOpcode();
    if (Opc == clang::UO_LNot) {
      S << "l_not(";
      handleImplicitCasting(o->getSubExpr(), S, rewriter);
      S << ")";
      return;
    }
  }
  S << "isNotEqual(";
  makeUntypedPointer(expr, S, rewriter);
  expr->printPretty(S, nullptr, clang::PrintingPolicy(rewriter.getLangOpts()));
  if (clang::isa<clang::PointerType>(expr->getType().getCanonicalType()))
    S << ", NULL)";
  else
    S << ", 0)";
}

void MyASTVisitor::convertCompositePredicate(clang::Expr *Cond,
                                             llvm::raw_string_ostream &S,
                                             clang::Rewriter TheRewriter) {
  if (clang::isa<clang::BinaryOperator>(Cond)) {
    clang::BinaryOperator *o = clang::dyn_cast<clang::BinaryOperator>(Cond);
    clang::BinaryOperator::Opcode Opc = o->getOpcode();
    bool checkOperands = false;
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
      checkOperands = true;
      break;
    case clang::BO_LOr:
      S << "l_or(";
      checkOperands = true;
      break;
    default:
      Cond->printPretty(S, nullptr,
                        clang::PrintingPolicy(TheRewriter.getLangOpts()));
      return;
    }
    if (checkOperands) {
      handleImplicitCasting(o->getLHS(), S, TheRewriter);
    } else {
      makeUntypedPointer(o->getLHS(), S, TheRewriter);
      convertCompositePredicate(o->getLHS(), S, TheRewriter);
    }
    S << ", ";
    if (checkOperands) {
      handleImplicitCasting(o->getRHS(), S, TheRewriter);
    } else {
      makeUntypedPointer(o->getRHS(), S, TheRewriter);
      convertCompositePredicate(o->getRHS(), S, TheRewriter);
    }
    S << ")";
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
  } else if (clang::isa<clang::CallExpr>(Cond)) {
    clang::CallExpr *c = clang::dyn_cast<clang::CallExpr>(Cond);
    clang::FunctionDecl *f =
        clang::dyn_cast<clang::CallExpr>(Cond)->getDirectCallee();
    std::string funcName = f->getNameInfo().getName().getAsString();
    if (funcName == "strcmp") {
      S << "strcmp2(";
      for (auto &it : c->arguments()) {
        if (it != *(c->arg_begin()))
          S << ", ";
        it->printPretty(S, nullptr,
                        clang::PrintingPolicy(TheRewriter.getLangOpts()));
      }
      S << ")";

    } else {
      Cond->printPretty(S, nullptr,
                        clang::PrintingPolicy(TheRewriter.getLangOpts()));
    }
  } else {
    Cond->printPretty(S, nullptr,
                      clang::PrintingPolicy(TheRewriter.getLangOpts()));
  }
}

bool MyASTVisitor::VisitStmt(clang::Stmt *s) {
  if (clang::isa<clang::DoStmt>(s)) {
    clang::DoStmt *F = clang::cast<clang::DoStmt>(s);
    int stmtid = assignDep(s, nullptr, false);

    clang::Expr *Cond = F->getCond();
    clang::Stmt *Body = F->getBody();

    assignDep(Body, s, true);

    insertbranchlog(Cond, stmtid);
  } else if (clang::isa<clang::ForStmt>(s)) {
    clang::ForStmt *F = clang::cast<clang::ForStmt>(s);
    int stmtid = assignDep(s, nullptr, false);

    clang::Expr *Cond = F->getCond();
    clang::Stmt *Body = F->getBody();

    assignDep(Body, s, true);

    insertbranchlog(Cond, stmtid);
  } else if (clang::isa<clang::IfStmt>(s)) {
    clang::IfStmt *I = clang::cast<clang::IfStmt>(s);
    int stmtid = assignDep(s, nullptr, false);

    clang::Expr *Cond = I->getCond();
    clang::Stmt *Then = I->getThen();
    clang::Stmt *Else = I->getElse();

    if (hasReturnStmt(Then) != hasReturnStmt(Else)) { // Exclusive OR
      cfg[-stmtid] = std::pair<int, bool>(-stmtid, hasReturnStmt(Then));
    }

    assignDep(Then, s, true);
    if (Else)
      assignDep(Else, s, false);

    insertbranchlog(Cond, stmtid);
  } else if (clang::isa<clang::WhileStmt>(s)) {
    clang::WhileStmt *F = clang::cast<clang::WhileStmt>(s);
    int stmtid = assignDep(s, nullptr, false);

    clang::Expr *Cond = F->getCond();
    clang::Stmt *Body = F->getBody();

    assignDep(Body, s, true);

    insertbranchlog(Cond, stmtid);
  }

  return true;
}
