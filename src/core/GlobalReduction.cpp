#include "GlobalReduction.h"

#include <spdlog/spdlog.h>

#include "FileManager.h"
#include "OptionManager.h"
#include "Profiler.h"
#include "SourceManager.h"

void GlobalReduction::Initialize(clang::ASTContext &Ctx) {
  Reduction::Initialize(Ctx);
  CollectionVisitor = new GlobalElementCollectionVisitor(this);
}

bool GlobalReduction::HandleTopLevelDecl(clang::DeclGroupRef D) {
  clang::SourceManager &SM = Context->getSourceManager();
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    if (SourceManager::IsInHeader(SM, *I))
      continue;
    CollectionVisitor->TraverseDecl(*I);
  }
  return true;
}

void GlobalReduction::HandleTranslationUnit(clang::ASTContext &Ctx) {
  std::vector<llvm::PointerUnion<clang::Decl *, clang::Stmt *>> elements;
  elements.resize(Decls.size());
  std::transform(Decls.begin(), Decls.end(), elements.begin(), CastElement);
  doDeltaDebugging(elements);
}

DDElement GlobalReduction::CastElement(clang::Decl *D) { return D; }

bool GlobalReduction::callOracle() {
  Profiler::GetInstance()->incrementGlobalReductionCounter();

  if (Reduction::callOracle()) {
    Profiler::GetInstance()->incrementSuccessfulGlobalReductionCounter();
    FileManager::GetInstance()->saveTemp("global", true);
    return true;
  } else {
    FileManager::GetInstance()->saveTemp("global", false);
    return false;
  }
}

bool GlobalReduction::test(DDElementVector &ToBeRemoved) {
  const clang::SourceManager &SM = Context->getSourceManager();
  std::vector<clang::SourceRange> Ranges;
  std::vector<llvm::StringRef> Reverts;

  for (auto const &Element : ToBeRemoved) {
    if (Element.isNull())
      continue;

    clang::Decl *D = Element.get<clang::Decl *>();

    clang::SourceLocation Start = D->getSourceRange().getBegin();
    clang::SourceLocation End;

    if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(D)) {
      if (FD->isThisDeclarationADefinition())
        End = FD->getSourceRange().getEnd();
    } else if (clang::EmptyDecl *ED = llvm::dyn_cast<clang::EmptyDecl>(D)) {
      End = ED->getSourceRange().getEnd();
    } else {
      End = SourceManager::GetEndLocationUntil(SM, D->getSourceRange(), ';');
    }

    if (Start.isMacroID())
      Start = SM.getFileLoc(Start);
    if (End.isMacroID())
      End = SM.getFileLoc(End);

    if (End.isInvalid() || Start.isInvalid())
      return false;

    clang::SourceRange Range(Start, End);
    Ranges.emplace_back(Range);
    llvm::StringRef Revert = SourceManager::GetSourceText(SM, Start, End);
    Reverts.emplace_back(Revert);
    removeSourceText(Start, End);
  }

  TheRewriter.overwriteChangedFiles();

  if (callOracle()) {
    return true;
  } else {
    for (int i = 0; i < Reverts.size(); i++)
      TheRewriter.ReplaceText(Ranges[i], Reverts[i]);
    TheRewriter.overwriteChangedFiles();
    return false;
  }
}

bool GlobalReduction::isInvalidChunk(DDElementVector &Chunk) {
  if (OptionManager::SkipGlobalDep)
    return false;
  return !(std::all_of(std::begin(Chunk), std::end(Chunk), [&](DDElement i) {
    return !UseInfo[i.get<clang::Decl *>()];
  }));
}

bool GlobalElementCollectionVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DRE) {
  auto *D = DRE->getDecl();
  if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(D)) {
    if (VD->hasGlobalStorage())
      Consumer->UseInfo[VD] = true;
  } else if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(D)) {
    Consumer->UseInfo[FD->getDefinition()] = true;
  }

  return true;
}

bool GlobalElementCollectionVisitor::VisitFunctionDecl(
    clang::FunctionDecl *FD) {
  spdlog::get("Logger")->debug("Visit Function Decl: {}",
                               FD->getNameInfo().getAsString());
  // hard rule : must contain main()
  if (FD->isMain() && !OptionManager::SkipGlobalDep)
    return true;
  Consumer->Decls.emplace_back(FD->getDefinition());
  return true;
}

bool GlobalElementCollectionVisitor::VisitVarDecl(clang::VarDecl *VD) {
  if (VD->hasGlobalStorage()) {
    spdlog::get("Logger")->debug("Visit Var Decl: {}", VD->getNameAsString());
    Consumer->Decls.emplace_back(VD);
  }
  return true;
}

bool GlobalElementCollectionVisitor::VisitEmptyDecl(clang::EmptyDecl *ED) {
  spdlog::get("Logger")->debug("Visit Empty Decl");
  Consumer->Decls.emplace_back(ED);
  return true;
}
