#ifndef JUICYC_VISITOR_SYNTAX_CONTEXT_H_
#define JUICYC_VISITOR_SYNTAX_CONTEXT_H_

#include "juicyc/status.h"
#include "juicyc/env.h"
#include "util/util.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>

#include <iostream>

namespace juicyc {
namespace syntax {

struct Context {
  Env* env = nullptr;
  std::string name;
  llvm::LLVMContext llvm;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module;
  Logger logger;

  Context(std::string _name) : name(_name), builder(llvm) {
    module = std::unique_ptr<llvm::Module>(new llvm::Module(name, llvm));
    Init();
  }
  void Init() {}
  Status ExportIR(std::string filename) {
    FunctionGuard g(std::cout, "ExportIR");
    if (llvm::verifyModule(*module, &llvm::errs())) {
      return Status::Corruption("Module corrupted");
    }
    // std::error_code ec;
    // llvm::raw_string_ostream dest("content");
    // llvm::raw_os_ostream dest(*env->fopen("out.ir"));
    // llvm::raw_fd_ostream dest("out.ir", ec, llvm::sys::fs::F_None);
    auto os = env->output_system()->fopen(filename);
    llvm::raw_os_ostream dest(*os);
    module->print(dest, nullptr);
    dest.flush();
    env->output_system()->fclose(os);
    return Status::OK();
  }
  Status ExportObj(std::string filename) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    assert(target);
    auto cpu = "generic";
    auto feature = "";
    llvm::TargetOptions opts;
    auto rm = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple,
                                                     cpu,
                                                     feature,
                                                     opts,
                                                     rm);
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::F_None);
    // auto os = env->output_system()->fopen(filename);
    // llvm::raw_os_ostream dest(*os);
    llvm::legacy::PassManager pass;
    auto fileType = llvm::TargetMachine::CGFT_ObjectFile;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
      return Status::IOError("file type failed");
    }
    pass.run(*module);
    dest.flush();
    // env->output_system()->fclose(os);
    return Status::OK();
  }
};

}  // namespace syntax
}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_CONTEXT_H_