#include <sstream>

#include "ControlDependency.h"

int MyASTVisitor::getStmtid(clang::Stmt *s) {
  branchidsty::iterator it =
      std::find_if(branchids.begin(), branchids.end(), isidExist(s));
  int stmtid = it->second;
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

std::pair<clang::Stmt *, clang::Stmt *> MyASTVisitor::getDep(clang::Stmt *s) {
  branchdepty::iterator it =
      std::find_if(branchdeps.begin(), branchdeps.end(), isDepExist(s));
  if (it == branchdeps.end()) {
    return std::pair<clang::Stmt *, clang::Stmt *>(s, NULL);
  }

  return *it;
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
  cfg.push_back(std::tuple<int, int, bool>(stmtid, parentid, cond));
}

int MyASTVisitor::assignDep(clang::Stmt *s, clang::Stmt *parent, bool cond) {
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

void MyASTVisitor::insertbranchlog(clang::Expr *Cond, int stmtid) {
  if (Cond != nullptr) {
    std::string str;
    llvm::raw_string_ostream S(str);
    S << "inst(" << stmtid << ", ";
    convertCompositePredicate(Cond, S, TheRewriter);
    S << ")";
    TheRewriter.ReplaceText(Cond->getSourceRange(), S.str());
  }
}

void MyASTVisitor::convertCompositePredicate(clang::Expr *Cond,
                                             llvm::raw_string_ostream &S,
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
    clang::ImplicitCastExpr *c = clang::dyn_cast<clang::ImplicitCastExpr>(Cond);
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

bool MyASTVisitor::VisitStmt(clang::Stmt *s) {
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
    if (Else)
      assignDep(Else, s, false);

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

bool MyASTVisitor::VisitFunctionDecl(clang::FunctionDecl *f) {
  TheRewriter.InsertTextAfter(f->getLocStart(), "extern \"C\"\n");
  return true;
}
