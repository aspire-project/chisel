#include "DeadcodeElimination.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/Utils.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/TargetSelect.h"

#include "FileManager.h"
#include "OptionManager.h"

void DeadcodeElimination::Initialize(clang::ASTContext &Ctx) {
  Transformation::Initialize(Ctx);
  CollectionVisitor = new DeadcodeElementCollectionVisitor(this);
}

bool DeadcodeElimination::HandleTopLevelDecl(clang::DeclGroupRef D) {
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
    CollectionVisitor->TraverseDecl(*I);
  return true;
}

clang::SourceRange
DeadcodeElimination::getRemoveRange(clang::SourceLocation Loc) {
  for (auto Entry : LocationMapping) {
    clang::SourceLocation Begin = Entry.second.getBegin();
    clang::SourceLocation End;
    if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(Entry.first)) {
      if (VD->hasInit()) {
        if (isConstant(VD->getInit()))
          End = getEndLocationUntil(VD->getSourceRange(), ';');
      } else
        End = getEndLocationUntil(VD->getSourceRange(), ';');
    } else if (clang::LabelDecl *LD =
                   llvm::dyn_cast<clang::LabelDecl>(Entry.first))
      End = LD->getStmt()->getSubStmt()->getBeginLoc().getLocWithOffset(-1);

    if ((Begin < Loc || Begin == Loc) && (Loc < End || Loc == End))
      return clang::SourceRange(Begin, End);
  }
  return clang::SourceRange();
}

bool DeadcodeElimination::isConstant(clang::Stmt *S) {
  if (clang::StringLiteral *L = llvm::dyn_cast<clang::StringLiteral>(S))
    return true;
  if (clang::IntegerLiteral *L = llvm::dyn_cast<clang::IntegerLiteral>(S))
    return true;
  if (clang::CharacterLiteral *L = llvm::dyn_cast<clang::CharacterLiteral>(S))
    return true;
  if (clang::CompoundLiteralExpr *L =
          llvm::dyn_cast<clang::CompoundLiteralExpr>(S))
    return true;
  if (clang::FloatingLiteral *L = llvm::dyn_cast<clang::FloatingLiteral>(S))
    return true;
  if (clang::ImaginaryLiteral *L = llvm::dyn_cast<clang::ImaginaryLiteral>(S))
    return true;
  if (clang::CastExpr *L = llvm::dyn_cast<clang::CastExpr>(S)) {
    clang::Stmt *FirstChild;
    for (auto Child : S->children()) {
      FirstChild = Child;
      break;
    }
    return isConstant(FirstChild);
  }
  return false;
}

void DeadcodeElimination::removeUnusedElements() {
  for (auto Loc : UnusedLocations) {
    clang::SourceRange Range = getRemoveRange(Loc);
    if (Range.isInvalid())
      continue;
    removeSourceText(Range.getBegin(), Range.getEnd());
  }
  TheRewriter.overwriteChangedFiles();
}

bool DeadcodeElementCollectionVisitor::VisitVarDecl(clang::VarDecl *VD) {
  if (clang::ParmVarDecl *PVD = llvm::dyn_cast<clang::ParmVarDecl>(VD))
    return true;
  Consumer->LocationMapping.insert(std::make_pair(VD, VD->getSourceRange()));
  return true;
}

bool DeadcodeElementCollectionVisitor::VisitLabelStmt(clang::LabelStmt *LS) {
  Consumer->LocationMapping.insert(
      std::make_pair(LS->getDecl(), LS->getDecl()->getSourceRange()));
  return true;
}

bool DCEFrontend::Parse(std::string &InputFile, DeadcodeElimination *R) {
  std::unique_ptr<clang::CompilerInstance> CI(new clang::CompilerInstance);
  clang::DiagnosticOptions *Opts = new clang::DiagnosticOptions();
  Opts->Warnings.push_back("unused-variable");
  Opts->Warnings.push_back("unused-label");
  clang::TextDiagnosticBuffer *TextDiagBuffer =
      new clang::TextDiagnosticBuffer();
  llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> D =
      CI->createDiagnostics(Opts, TextDiagBuffer);
  CI->setDiagnostics(D.get());
  clang::TargetOptions &TO = CI->getTargetOpts();
  TO.Triple = llvm::sys::getDefaultTargetTriple();
  clang::TargetInfo *Target = clang::TargetInfo::CreateTargetInfo(
      CI->getDiagnostics(), CI->getInvocation().TargetOpts);
  CI->setTarget(Target);

  CI->createFileManager();
  CI->createSourceManager(CI->getFileManager());
  CI->createPreprocessor(clang::TU_Complete);
  CI->createASTContext();
  CI->setASTConsumer(std::unique_ptr<clang::ASTConsumer>(R));
  clang::Preprocessor &PP = CI->getPreprocessor();
  PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());

  if (!CI->InitializeSourceManager(
          clang::FrontendInputFile(InputFile, clang::InputKind::C))) {
    return false;
  }
  CI->createSema(clang::TU_Complete, 0);
  clang::DiagnosticsEngine &Diag = CI->getDiagnostics();
  CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(),
                                            &CI->getPreprocessor());
  ParseAST(CI->getSema());

  for (clang::TextDiagnosticBuffer::const_iterator DiagnosticIterator =
           TextDiagBuffer->warn_begin();
       DiagnosticIterator != TextDiagBuffer->warn_end(); ++DiagnosticIterator)
    R->UnusedLocations.emplace_back(DiagnosticIterator->first);
  R->removeUnusedElements();
  CI->getDiagnosticClient().EndSourceFile();
  return true;
}
