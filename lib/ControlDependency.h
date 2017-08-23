/*
 * Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
 * Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
 * Copyright (C) 2017 by Gabin An <agb94@kaist.ac.kr>
 *
 * Licensed under the MIT License:
 * See the LICENSE file at the top-level directory of this distribution.
 */

#ifndef __CONTROLDEPENDENCY_H__
#define __CONTROLDEPENDENCY_H__

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "Type.h"

class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
public:
  explicit MyASTVisitor(clang::Rewriter &R, ControlDependency &out)
      : TheRewriter(R), cfg(out) {
    id = 1;
  }

  typedef std::vector<std::pair<clang::Stmt *, int>> branchidsty;
  branchidsty branchids;

  struct isidExist {
    explicit isidExist(clang::Stmt *s) : _s(s) {}
    bool operator()(std::pair<clang::Stmt *, int> const &p) {
      return (p.first == _s);
    }
    clang::Stmt *_s;
  };

  bool hasReturnStmt(clang::Stmt *s);
  
  int getStmtid(clang::Stmt *s);

  int assignStmtid(clang::Stmt *s);

  struct isDepExist {
    explicit isDepExist(clang::Stmt *s) : _s(s) {}
    bool operator()(std::pair<clang::Stmt *, clang::Stmt *> const &p) {
      return (p.first == _s);
    }
    clang::Stmt *_s;
  };

  std::pair<clang::Stmt *, clang::Stmt *> getDep(clang::Stmt *s);

  void insertdep(clang::SourceLocation Loc, int stmtid, int parentid,
                 bool cond);

  int assignDep(clang::Stmt *s, clang::Stmt *parent, bool cond);

  void insertbranchlog(clang::Expr *Cond, int stmtid);

  void makeUntypedPointer(clang::Expr *Cond, llvm::raw_string_ostream &S,
                          clang::Rewriter TheRewriter);

  void handleImplicitCasting(clang::Expr *Cond, llvm::raw_string_ostream &S,
                             clang::Rewriter TheRewriter);

  void convertCompositePredicate(clang::Expr *Cond, llvm::raw_string_ostream &S,
                                 clang::Rewriter TheRewriter);

  bool VisitStmt(clang::Stmt *s);

private:
  clang::Rewriter &TheRewriter;
  int id;
  ControlDependency &cfg;
};

#endif // __CONTROLDEPENDENCY_H__
