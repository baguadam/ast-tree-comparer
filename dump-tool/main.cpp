#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Index/USRGeneration.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <fstream>

class TreeBuilder : public clang::RecursiveASTVisitor<TreeBuilder> {
public:
  explicit TreeBuilder(clang::ASTContext* Context, std::ofstream& outFile)
    : Context(Context), depth(-1), outFile(outFile) {}

  bool shouldVisitImplicitCode() const {
    return true;
  }

  bool TraverseDecl(clang::Decl* decl) {
    ++depth;
    bool result = clang::RecursiveASTVisitor<TreeBuilder>::TraverseDecl(decl);
    --depth;

    return result;
  }

  bool TraverseStmt(clang::Stmt* stmt) {
    ++depth;
    bool result = clang::RecursiveASTVisitor<TreeBuilder>::TraverseStmt(stmt);
    --depth;

    return result;
  }

  bool VisitDecl(clang::Decl* decl) {
    indent();

    clang::SourceLocation loc = decl->getBeginLoc();
    clang::SourceManager& sm = Context->getSourceManager();
    std::string filePath = sm.getFilename(loc).str();
    if (filePath.empty()) {
      filePath = "N/A";
    }

    outFile
      << "Declaration\t"
      << decl->getDeclKindName() << '\t'
      << getUSR(decl) << '\t'
      << filePath << '\t'
      << sm.getSpellingLineNumber(loc) << '\t'
      << sm.getSpellingColumnNumber(loc) << '\t'
      << (decl->isImplicit() ? "(implicit)" : "") << '\n';

    return true;
  }

  bool VisitStmt(clang::Stmt* stmt) {
    indent();

    clang::SourceLocation loc = stmt->getBeginLoc();
    clang::SourceManager& sm = Context->getSourceManager();
    std::string filePath = sm.getFilename(loc).str();
    if (filePath.empty()) {
      filePath = "N/A";
    }

    outFile
      << "Statement\t"
      << stmt->getStmtClassName() << '\t'
      << "N/A" << '\t'
      << filePath << '\t'
      << sm.getSpellingLineNumber(loc) << '\t'
      << sm.getSpellingColumnNumber(loc) << '\n';

    return true;
  }

private:
  clang::ASTContext* Context;
  int depth;
  std::ofstream& outFile;

  void indent() const {
    for (int i = 0; i < depth; ++i)
      outFile << ' ';
  }

  std::string getUSR(clang::Decl* decl) const {
    llvm::SmallVector<char> Usr;
    clang::index::generateUSRForDecl(decl, Usr);
    return std::string(Usr.begin(), Usr.end());
  }
};

class CustomASTComsumer : public clang::ASTConsumer {
public:
  explicit CustomASTComsumer(clang::ASTContext* Context, std::ofstream& outFile) 
    : Visitor(Context, outFile) { }

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  TreeBuilder Visitor;
};

class CustomFrontendAction : public clang::ASTFrontendAction {
public:
  CustomFrontendAction(std::ofstream& outFile) : outFile(outFile) {}

  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) override {
    return std::make_unique<CustomASTComsumer>(&Compiler.getASTContext(), outFile);
  }

private:
  std::ofstream& outFile;
};

class CustomFrontendActionFactory : public clang::tooling::FrontendActionFactory {
public:
  CustomFrontendActionFactory(std::ofstream& outFile) : outFile(outFile) {}

  std::unique_ptr<clang::FrontendAction> create() override {
    return std::make_unique<CustomFrontendAction>(outFile);
  }

private:
  std::ofstream& outFile;
};

int main(int argc, const char* argv[]) {
  llvm::cl::OptionCategory MyToolCategory("my-tool options");
  
  llvm::cl::opt<std::string> OutputFileName(
    "o",
    llvm::cl::desc("Specify output file name"),
    llvm::cl::value_desc("filename"),
    llvm::cl::init("output_ast.txt"),
    llvm::cl::cat(MyToolCategory)
  );

  auto ExpectedParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);

  if (!ExpectedParser) {
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }

  clang::tooling::CommonOptionsParser& OptionsParser = ExpectedParser.get();
  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

  std::vector<std::string> Args = {
    "-IC:/msys64/mingw64/include",
    "-IC:/msys64/mingw64/lib/clang/18/include", // adjust version if needed
    "-IC:/msys64/mingw64/include/c++/14.2.0",
  };

  Tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(Args, clang::tooling::ArgumentInsertPosition::END));

  std::ofstream outFile(OutputFileName);
  if (!outFile.is_open()) {
    llvm::errs() << "Error: Could not open output file " << OutputFileName << " for writing.\n";
    return 1;
  }

  // Use the custom factory to create actions
  CustomFrontendActionFactory factory(outFile);
  int result = Tool.run(&factory);

  outFile.close();

  return result;
}