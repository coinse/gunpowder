#include <fstream>

#include "clang/Frontend/CompilerInstance.h"

#include "Consumers.h"
#include "FrontendActions.h"

void MyFrontendAction::EndSourceFileAction() {
  const clang::RewriteBuffer *RewriteBuf =
      r.getRewriteBufferFor(r.getSourceMgr().getMainFileID());
  std::string f = std::string(this->getCurrentFile());
  std::string filename = f.substr(0, f.find_last_of('.'));
  filename = filename + ".inst.c";
  std::ofstream out(filename.c_str());
  out << std::string(RewriteBuf->begin(), RewriteBuf->end());
}
std::unique_ptr<clang::ASTConsumer>
MyFrontendAction::CreateASTConsumer(clang::CompilerInstance &Compiler,
                                    llvm::StringRef InFile) {
  clang::SourceManager &SourceMgr = Compiler.getSourceManager();
  r.setSourceMgr(SourceMgr, Compiler.getLangOpts());
  r.InsertTextAfter(SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID()),
                    "#include \"./branch_distance.h\"\n");
  r.InsertTextAfter(SourceMgr.getLocForStartOfFile(SourceMgr.getMainFileID()),
                    "#include \"../util/strcmp2.c\"\n");
  return std::unique_ptr<clang::ASTConsumer>(
      new MyASTConsumer(funcName, r, out));
}

std::unique_ptr<clang::ASTConsumer>
DeclarationAction::CreateASTConsumer(clang::CompilerInstance &Compiler,
                                     llvm::StringRef InFile) {
  return std::unique_ptr<clang::ASTConsumer>(
      new DeclarationConsumer(funcName, out));
}

std::unique_ptr<clang::ASTConsumer>
FunctionListAction::CreateASTConsumer(clang::CompilerInstance &Compiler,
                                      llvm::StringRef InFile) {
  return std::unique_ptr<clang::ASTConsumer>(new FunctionConsumer);
}
