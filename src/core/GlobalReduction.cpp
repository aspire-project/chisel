#include "GlobalReduction.h"

#include <spdlog/spdlog.h>

#include "FileManager.h"
#include "OptionManager.h"
#include "Profiler.h"
#include "StringUtils.h"

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
  std::string TempFileName =
      FileManager::GetInstance()->getTempFileName("global");

  if (Reduction::callOracle()) {
    FileManager::GetInstance()->updateBest();
    Profiler::GetInstance()->incrementSuccessfulGlobalReductionCounter();
    if (OptionManager::SaveTemp)
      writeToFile(TempFileName + ".success.c");
    return true;
  } else {
    if (OptionManager::SaveTemp)
      writeToFile(TempFileName + ".fail.c");
    return false;
  }
}

bool GlobalReduction::test(std::vector<DDElement> &ToBeRemoved) {
  const clang::SourceManager *SM = &Context->getSourceManager();
  std::vector<clang::SourceRange> Ranges;
  std::vector<std::string> Reverts;

  for (auto const &D : ToBeRemoved) {
    clang::SourceLocation Start =
        D.get<clang::Decl *>()->getSourceRange().getBegin();
    clang::SourceLocation End;

    clang::FunctionDecl *FD =
        llvm::dyn_cast<clang::FunctionDecl>(D.get<clang::Decl *>());
    if (FD && FD->isThisDeclarationADefinition()) {
      End = FD->getSourceRange().getEnd().getLocWithOffset(1);
    } else {
      End = getEndLocationUntil(D.get<clang::Decl *>()->getSourceRange(), ';')
                .getLocWithOffset(1);
    }
    clang::SourceRange Range(Start, End);
    Ranges.emplace_back(Range);
    std::string currRevert = getSourceText(Range);
    Reverts.emplace_back(currRevert);
    TheRewriter.ReplaceText(Range, StringUtils::placeholder(currRevert));
  }

  writeToFile(OptionManager::InputFile);

  if (callOracle()) {
    /* remove from RefList and WhereUsed */
    return true;
  } else {
    for (int i = 0; i < Reverts.size(); i++) {
      TheRewriter.ReplaceText(Ranges[i], Reverts[i]);
    }
    writeToFile(OptionManager::InputFile);
    return false;
  }
}

std::vector<DDElementVector>
GlobalReduction::refineSubsets(std::vector<DDElementVector> &Subsets) {
  std::vector<DDElementVector> result;
  for (auto const &subset : Subsets) {
    bool flag = true;
    for (auto const &i : subset) {
      auto key = i.get<clang::Decl *>();
      if (RefList[key].size() != 0) {
        flag = false;
        break;
      }
    }
    if (flag)
      result.emplace_back(subset);
  }
  return result;
}

bool GlobalElementCollectionVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DRE) {
  if (clang::FunctionDecl *FD =
          llvm::dyn_cast<clang::FunctionDecl>(DRE->getDecl())) {
    if (FD->isThisDeclarationADefinition()) {
      if (Consumer->RefList.find(FD) != Consumer->RefList.end()) { // found
        std::vector<clang::DeclRefExpr *> &v_ref = Consumer->RefList[FD];
        v_ref.emplace_back(DRE);
      } else { // not found
        std::vector<clang::DeclRefExpr *> v;
        v.emplace_back(DRE);
        Consumer->RefList.insert(std::make_pair(FD, v));
      }
    }
  } else if (clang::VarDecl *VD =
                 llvm::dyn_cast<clang::VarDecl>(DRE->getDecl())) {
    Consumer->WhereUsed.insert(std::make_pair(DRE, VD));
    if (Consumer->RefList.find(VD) != Consumer->RefList.end()) { // found
      std::vector<clang::DeclRefExpr *> &v_ref = Consumer->RefList[VD];
      v_ref.emplace_back(DRE);
    } else { // not found
      std::vector<clang::DeclRefExpr *> v;
      v.emplace_back(DRE);
      Consumer->RefList.insert(std::make_pair(VD, v));
    }
  }
  return true;
}

bool GlobalElementCollectionVisitor::VisitFunctionDecl(
    clang::FunctionDecl *FD) {
  spdlog::get("Logger")->debug("Visit Function Decl: {}",
                               FD->getNameInfo().getAsString());
  if (!FD->isMain()) {
    Consumer->Decls.emplace_back(FD);
  }
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
