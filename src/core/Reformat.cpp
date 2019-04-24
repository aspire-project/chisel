#include "Reformat.h"

#include "clang/Format/Format.h"
#include "clang/Tooling/Inclusions/HeaderIncludes.h"

#include "OptionManager.h"

void Reformat::Initialize(clang::ASTContext &Ctx) {
  Transformation::Initialize(Ctx);
}

std::vector<clang::tooling::Range>
Reformat::createRanges(llvm::MemoryBuffer *Code) {
  llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemoryFileSystem(
      new llvm::vfs::InMemoryFileSystem);
  clang::FileManager Files(clang::FileSystemOptions(), InMemoryFileSystem);
  clang::SourceManager *SM = &Context->getSourceManager();
  InMemoryFileSystem.get()->addFileNoOwn("temp", 0, Code);
  clang::FileID ID = SM->createFileID(
      Files.getFile("temp"), clang::SourceLocation(), clang::SrcMgr::C_User);

  unsigned Offset = SM->getFileOffset(SM->getLocForStartOfFile(ID));
  unsigned Length = SM->getFileOffset(SM->getLocForEndOfFile(ID)) - Offset;
  return {clang::tooling::Range(Offset, Length)};
}

void Reformat::doReformatting(std::string FileName,
                              clang::format::FormatStyle FS) {
  std::unique_ptr<llvm::MemoryBuffer> Code =
      std::move(llvm::MemoryBuffer::getFile(FileName).get());
  clang::tooling::Replacements Rs =
      reformat(FS, Code->getBuffer(), createRanges(Code.get()), FileName);
  clang::tooling::applyAllReplacements(Rs, TheRewriter);
  TheRewriter.overwriteChangedFiles();
}

void Reformat::HandleTranslationUnit(clang::ASTContext &Ctx) {
  doReformatting(OptionManager::InputFile, clang::format::getLLVMStyle());
}
