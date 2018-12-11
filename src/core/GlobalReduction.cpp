#include "GlobalReduction.h"

#include <spdlog/spdlog.h>

#include "FileManager.h"
#include "OptionManager.h"
#include "Profiler.h"

void GlobalReduction::Initialize(clang::ASTContext &Ctx) {
  Reduction::Initialize(Ctx);
  CollectionVisitor = new GlobalElementCollectionVisitor(this);
}

bool GlobalReduction::HandleTopLevelDecl(clang::DeclGroupRef D) {
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
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
  const clang::SourceManager *SM = &Context->getSourceManager();
  std::vector<clang::SourceRange> Ranges;
  std::vector<llvm::StringRef> Reverts;

  for (auto const &D : ToBeRemoved) {
    clang::SourceLocation Start =
        D.get<clang::Decl *>()->getSourceRange().getBegin();
    clang::SourceLocation End;
    clang::FunctionDecl *FD =
        llvm::dyn_cast<clang::FunctionDecl>(D.get<clang::Decl *>());
    if (FD && FD->isThisDeclarationADefinition()) {
      End = FD->getSourceRange().getEnd();
    } else if (clang::EmptyDecl *ED =
                   llvm::dyn_cast<clang::EmptyDecl>(D.get<clang::Decl *>())) {
      End = ED->getSourceRange().getEnd();
    } else {
      End = getEndLocationUntil(D.get<clang::Decl *>()->getSourceRange(), ';');
    }
    const clang::SourceRange Range(Start, End);
    Ranges.emplace_back(Range);
    llvm::StringRef CurrRevert = getSourceText(Start, End);
    Reverts.emplace_back(CurrRevert);
    removeSourceText(Start, End);
  }

  TheRewriter.overwriteChangedFiles();

  if (callOracle()) {
    return true;
  } else {
    for (int i = 0; i < Reverts.size(); i++) {
      TheRewriter.ReplaceText(Ranges[i], Reverts[i]);
    }
    TheRewriter.overwriteChangedFiles();
    return false;
  }
}

bool GlobalReduction::isInvalidChunk(DDElementVector &Chunk) {
  if (OptionManager::SkipGlobalDep)
    return false;
  return !(std::all_of(std::begin(Chunk), std::end(Chunk), [&](DDElement i) {
    return UseInfo[i.get<clang::Decl *>()].size() == 0;
  }));
}

bool GlobalElementCollectionVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DRE) {
  if (clang::FunctionDecl *FD =
          llvm::dyn_cast<clang::FunctionDecl>(DRE->getDecl())) {
    if (FD->isThisDeclarationADefinition()) {
      Consumer->UseInfo[FD].emplace_back(DRE);
    } else {
      Consumer->UseInfo[DRE->getDecl()].emplace_back(DRE);
    }
  }
  return true;
}

bool GlobalElementCollectionVisitor::VisitFunctionDecl(
    clang::FunctionDecl *FD) {
  spdlog::get("Logger")->debug("Visit Function Decl: {}",
                               FD->getNameInfo().getAsString());
  // hard rule : must contain main()
  if (!OptionManager::SkipGlobalDep && !FD->isMain())
    Consumer->Decls.emplace_back(FD);
  return true;
}

bool GlobalElementCollectionVisitor::VisitVarDecl(clang::VarDecl *VD) {
  if (VD->hasGlobalStorage()) {
    spdlog::get("Logger")->debug("Visit Var Decl: {}", VD->getNameAsString());
    Consumer->Decls.emplace_back(VD);
  }
  return true;
}

bool GlobalElementCollectionVisitor::VisitRecordDecl(clang::RecordDecl *RD) {
  spdlog::get("Logger")->debug("Visit Record Decl: {}", RD->getNameAsString());
  Consumer->Decls.emplace_back(RD);
  return true;
}

bool GlobalElementCollectionVisitor::VisitTypedefDecl(clang::TypedefDecl *TD) {
  spdlog::get("Logger")->debug("Visit Typedef Decl: {}", TD->getNameAsString());
  assert(Consumer != nullptr);
  Consumer->Decls.emplace_back(TD);
  return true;
}

bool GlobalElementCollectionVisitor::VisitEnumDecl(clang::EnumDecl *ED) {
  spdlog::get("Logger")->debug("Visit Enum Decl: {}", ED->getNameAsString());
  Consumer->Decls.emplace_back(ED);
  return true;
}

bool GlobalElementCollectionVisitor::VisitEmptyDecl(clang::EmptyDecl *ED) {
  spdlog::get("Logger")->debug("Visit Empty Decl");
  Consumer->Decls.emplace_back(ED);
  return true;
}
