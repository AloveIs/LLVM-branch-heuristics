//===- LazyBranchProbabilityInfo.cpp - Lazy Branch Probability Analysis ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is an alternative analysis pass to BranchProbabilityInfoWrapperPass.
// The difference is that with this pass the branch probabilities are not
// computed when the analysis pass is executed but rather when the BPI results
// is explicitly requested by the analysis client.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LazyBranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"

using namespace llvm;

#define DEBUG_TYPE "lazy-branch-prob"

INITIALIZE_PASS_BEGIN(LazyBranchProbabilityInfoPass, DEBUG_TYPE,
                      "Lazy Branch Probability Analysis", true, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(LazyBranchProbabilityInfoPass, DEBUG_TYPE,
                    "Lazy Branch Probability Analysis", true, true)

char LazyBranchProbabilityInfoPass::ID = 0;

LazyBranchProbabilityInfoPass::LazyBranchProbabilityInfoPass()
    : FunctionPass(ID) {
  initializeLazyBranchProbabilityInfoPassPass(*PassRegistry::getPassRegistry());
}

void LazyBranchProbabilityInfoPass::print(raw_ostream &OS,
                                          const Module *) const {
  LBPI->getCalculated().print(OS);
}

void LazyBranchProbabilityInfoPass::getAnalysisUsage(AnalysisUsage &AU) const {
  // We require DT so it's available when LI is available. The LI updating code
  // asserts that DT is also present so if we don't make sure that we have DT
  // here, that assert will trigger.
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  //AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.setPreservesAll();
}

void LazyBranchProbabilityInfoPass::releaseMemory() { LBPI.reset(); }

bool LazyBranchProbabilityInfoPass::runOnFunction(Function &F) {
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  TargetLibraryInfo &TLI =
      getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  const DominatorTree &DT =  getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  const PostDominatorTree &PDT =  getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  LBPI = std::make_unique<LazyBranchProbabilityInfo>(&F, &LI, &TLI, &DT, &PDT);
  return false;
}

void LazyBranchProbabilityInfoPass::getLazyBPIAnalysisUsage(AnalysisUsage &AU) {
  AU.addRequired<LazyBranchProbabilityInfoPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

void llvm::initializeLazyBPIPassPass(PassRegistry &Registry) {
  INITIALIZE_PASS_DEPENDENCY(LazyBranchProbabilityInfoPass);
  INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass);
  INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass);
}