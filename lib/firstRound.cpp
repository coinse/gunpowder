//------------------------------------------------------------------------------
// Clang rewriter sample. Demonstrates:
//
// * How to use RecursiveASTVisitor to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
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
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "firstRound.h"

using namespace clang;

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class AddCodeASTVisitor : public RecursiveASTVisitor<AddCodeASTVisitor> {
 public:
  explicit AddCodeASTVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitStmt(clang::Stmt *s) {
    if (clang::isa<clang::SwitchStmt>(s)) {
      clang::SwitchStmt *F = clang::cast<clang::SwitchStmt>(s);
      clang::SwitchCase *cases = F->getSwitchCaseList();
      clang::Expr *Cond = F->getCond();

      clang::QualType cond_type = Cond->getType();
      std::string str = "";
      llvm::raw_string_ostream temp(str);
      Cond->printPretty(temp, nullptr,
                        clang::PrintingPolicy(TheRewriter.getLangOpts()));
      std::string SWITCH_TEMPORARY_VARIABLE =
          "__ins_switch_temp" +
          std::to_string(F->getLocStart().getRawEncoding());
      std::string lines = "";
      clang::SwitchCase *next_case = cases->getNextSwitchCase();
      while (cases != next_case && next_case &&
             clang::cast<clang::CaseStmt>(next_case)->getSubStmt() ==
                 cases) {  // rewinding
        cases = next_case;
        next_case = cases->getNextSwitchCase();
      }
      while (cases) {
        if (clang::CaseStmt::classof(cases)) {
          std::string predicate = "";
          clang::CaseStmt *case_stmt = clang::cast<clang::CaseStmt>(cases);
          predicate += SWITCH_TEMPORARY_VARIABLE + " == ";
          std::string str2 = "";
          llvm::raw_string_ostream temp_stream2(str2);
          case_stmt->getLHS()->printPretty(
              temp_stream2, nullptr,
              clang::PrintingPolicy(TheRewriter.getLangOpts()));
          predicate += temp_stream2.str();
          clang::Stmt *sub_stmt = cases->getSubStmt();
          while (sub_stmt && !clang::BreakStmt::classof(sub_stmt) &&
                 !clang::ReturnStmt::classof(sub_stmt)) {
            if (clang::CaseStmt::classof(sub_stmt)) {
              predicate =
                  predicate + " || " + SWITCH_TEMPORARY_VARIABLE + " == ";
              std::string str3 = "";
              llvm::raw_string_ostream temp_stream3(str3);
              clang::cast<clang::CaseStmt>(sub_stmt)->getLHS()->printPretty(
                  temp_stream3, nullptr,
                  clang::PrintingPolicy(TheRewriter.getLangOpts()));
              predicate += temp_stream3.str();
              sub_stmt = clang::cast<clang::CaseStmt>(sub_stmt)->getSubStmt();
            } else {
              sub_stmt = nullptr;
            }
          }

          lines = "if (" + predicate + ") { }\n" + lines;
        }
        next_case = cases->getNextSwitchCase();
        if (cases == next_case || !next_case) break;
        lines = "else " + lines;
        cases = next_case;
        next_case = cases->getNextSwitchCase();
        while (cases != next_case && next_case &&
               clang::cast<clang::CaseStmt>(next_case)->getSubStmt() ==
                   cases) {  // rewinding
          cases = next_case;
          next_case = cases->getNextSwitchCase();
        }
      }

      lines = cond_type.getAsString() + " " + SWITCH_TEMPORARY_VARIABLE +
              " = " + temp.str() + ";\n" + lines;
      lines = "{\n" + lines;
      clang::SourceLocation ST = F->getSwitchLoc();
      TheRewriter.InsertText(ST, lines, true, true);
      ST = F->getBody()->getLocEnd().getLocWithOffset(1);
      TheRewriter.InsertText(ST, "\n}", true, true);

      TheRewriter.ReplaceText((TheRewriter.getSourceMgr())
                                  .getExpansionRange(Cond->getSourceRange()),
                              SWITCH_TEMPORARY_VARIABLE);
    }

    return true;
  }

 private:
  Rewriter &TheRewriter;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class AddCodeASTConsumer : public ASTConsumer {
 public:
  explicit AddCodeASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
    return true;
  }

 private:
  AddCodeASTVisitor Visitor;
};

int firstRound(std::string infilename) {
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
  const FileEntry *FileIn = FileMgr.getFile(infilename);
  SourceMgr.setMainFileID(
      SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
  TheCompInst.getDiagnosticClient().BeginSourceFile(
      TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

  // Create an AST consumer instance which is going to get called by
  // ParseAST.
  AddCodeASTConsumer TheConsumer(TheRewriter);

  TheRewriter.InsertTextAfter(SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID()), "");
  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
           TheCompInst.getASTContext());

  // At this point the rewriter's buffer should be full with the rewritten
  // file contents.
  const RewriteBuffer *RewriteBuf =
      TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
  std::string filename = infilename + "_first_round.c";
  std::ofstream out(filename.c_str());
  out << std::string(RewriteBuf->begin(), RewriteBuf->end());

  return 0;
}
