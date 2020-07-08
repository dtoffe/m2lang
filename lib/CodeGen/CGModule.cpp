//===--- CGModule.cpp - Code Generator for Modules --------------*- C++ -*-===//
//
// Part of the M2Lang Project, under the Apache License v2.0 with
// LLVM Exceptions. See LICENSE file for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines the code generator implementation for modules.
///
//===----------------------------------------------------------------------===//

#include "m2lang/CodeGen/CGModule.h"
#include "m2lang/CodeGen/CGProcedure.h"
#include "m2lang/CodeGen/CGUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

using namespace m2lang;

void CGModule::initialize() {
  VoidTy = llvm::Type::getVoidTy(getLLVMCtx());
  Int1Ty = llvm::Type::getInt1Ty(getLLVMCtx());
  Int8Ty = llvm::Type::getInt8Ty(getLLVMCtx());
  Int32Ty = llvm::Type::getInt32Ty(getLLVMCtx());
  Int64Ty = llvm::Type::getInt64Ty(getLLVMCtx());
  FloatTy = llvm::Type::getFloatTy(getLLVMCtx());
  DoubleTy = llvm::Type::getDoubleTy(getLLVMCtx());
  Int32Zero = llvm::ConstantInt::get(Int32Ty, 0, /*isSigned*/ true);
}

llvm::Type *CGModule::convertType(TypeDenoter *TyDe) {
  if (auto *P = llvm::dyn_cast<PervasiveType>(TyDe)) {
    switch (P->getTypeKind()) {
      case pervasive::Void:
        return VoidTy;
      case pervasive::Boolean:
        return Int1Ty;
      case pervasive::Char:
        return Int8Ty;
      case pervasive::Cardinal:
      case pervasive::Integer:
      case pervasive::WholeNumber:
        return Int64Ty;
      case pervasive::Real:
        return FloatTy;
      case pervasive::LongReal:
      case pervasive::RealNumber:
        return DoubleTy;
      default:
        break;
    }
  }
  // TODO Implement.
  return Int32Ty;
}

llvm::Type *CGModule::convertType(Type *Ty) {
  return convertType(Ty->getTypeDenoter());
}

void CGModule::run(CompilationModule *CM) {
  this->CM = CM;
  ImplementationModule *PM = llvm::cast<ImplementationModule>(CM);
  for (auto *Decl : PM->getDecls()) {
    if (auto *Var = llvm::dyn_cast<Variable>(Decl)) {
      llvm::GlobalVariable *V = new llvm::GlobalVariable(
          *M, convertType(Var->getTypeDenoter()),
          /*isConstant=*/false, llvm::GlobalValue::PrivateLinkage, nullptr,
          utils::mangleName(Var));
      Globals[Var] = V;
    } else if (auto *Proc = llvm::dyn_cast<Procedure>(Decl)) {
      CGProcedure CGP(*this);
      CGP.run(Proc);
    }
  }
}