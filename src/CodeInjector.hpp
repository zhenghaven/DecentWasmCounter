// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>
#include <vector>

#include <src/ir.h>
#include <src/cast.h>

#include <DecentWasmCounter/Exceptions.hpp>

#include "Block.hpp"
#include "Classification.hpp"
#include "make_unique.hpp"

namespace DecentWasmCounter
{

struct InjectedSymbolInfo
{
	size_t m_thrId;
	size_t m_ctrId;

	size_t m_funcExceedId;
	size_t m_funcIncrId;
}; // struct InjectedSymbolInfo

inline bool IsFuncTypeFieldExist(
	const wabt::FuncSignature& sig,
	const std::vector<wabt::TypeEntry*>& types)
{
	for (const auto& t : types)
	{
		if (t->kind() == wabt::TypeEntryKind::Func)
		{
			const wabt::FuncType* ft =
				wabt::cast<const wabt::FuncType>(t);

			if (ft->sig == sig)
			{
				return true;
			}
		}
	}
	return false;
}

inline void AddFuncTypeIfNotExist(
	const wabt::FuncSignature& sig,
	wabt::Module& mod)
{
	if (!IsFuncTypeFieldExist(sig, mod.types))
	{
		std::unique_ptr<wabt::TypeModuleField> typeField =
			Internal::make_unique<wabt::TypeModuleField>();
		std::unique_ptr<wabt::FuncType> funcType =
			Internal::make_unique<wabt::FuncType>();

		funcType->sig = sig;

		typeField->type = std::move(funcType);
		mod.AppendField(std::move(typeField));
	}
}

inline InjectedSymbolInfo InjectCounterAndFunc(wabt::Module& mod)
{
	InjectedSymbolInfo info;

	// # threshold
	info.m_thrId = mod.globals.size();
	std::unique_ptr<wabt::GlobalModuleField> globalThr =
		Internal::make_unique<wabt::GlobalModuleField>();
	globalThr->global.type = wabt::Type::I64;
	globalThr->global.mutable_ = true;
	globalThr->global.init_expr.push_back(
		Internal::make_unique<wabt::ConstExpr>(wabt::Const::I64(0)));

	mod.AppendField(std::move(globalThr));

	// # global counter
	info.m_ctrId = mod.globals.size();
	std::unique_ptr<wabt::GlobalModuleField> globalCtr =
		Internal::make_unique<wabt::GlobalModuleField>();
	globalCtr->global.type = wabt::Type::I64;
	globalCtr->global.mutable_ = true;
	globalCtr->global.init_expr.push_back(
		Internal::make_unique<wabt::ConstExpr>(wabt::Const::I64(0)));

	mod.AppendField(std::move(globalCtr));

	// # modify import function decent_wasm_counter_exceed
	// - -> Looking for import statement
	wabt::FuncImport* funcExceed = nullptr;
	for (wabt::Import* im : mod.imports)
	{
		if (im->kind() == wabt::ExternalKind::Func)
		{
			wabt::FuncImport* imFunc = wabt::cast<wabt::FuncImport>(im);
			if (imFunc->module_name == "env" &&
				imFunc->field_name == "decent_wasm_counter_exceed")
			{
				if (funcExceed != nullptr)
				{
					throw Exception("There are more than one import of decent_wasm_counter_exceed function");
				}
				else
				{
					funcExceed = imFunc;
				}
			}
		}
	}
	if (funcExceed == nullptr)
	{
		throw Exception("Couldn't find import to decent_wasm_counter_exceed function");
	}
	// - -> Looking for function index
	info.m_funcExceedId = wabt::kInvalidIndex;
	for (size_t i = 0; i < mod.funcs.size(); ++i)
	{
		if (mod.funcs[i] == &(funcExceed->func))
		{
			info.m_funcExceedId = i;
		}
	}
	if (info.m_funcExceedId == wabt::kInvalidIndex)
	{
		throw Exception("Couldn't find the index to decent_wasm_counter_exceed function");
	}
	// - -> validate function format that can't be fixed
	if (funcExceed->func.local_types.size())
	{
		throw Exception("Import to decent_wasm_counter_exceed function has wrong format");
	}
	// - -> force to fix function format
	funcExceed->func.decl.sig.param_types.clear();
	funcExceed->func.decl.sig.param_type_names.clear();
	funcExceed->func.decl.sig.result_types.clear();
	funcExceed->func.decl.sig.result_type_names.clear();
	funcExceed->func.decl.sig.param_types.push_back(wabt::Type::I64);
	funcExceed->func.decl.has_func_type = false;
	funcExceed->func.exprs.clear();

	AddFuncTypeIfNotExist(funcExceed->func.decl.sig, mod);

	// # function to check
	info.m_funcIncrId = mod.funcs.size();
	std::unique_ptr<wabt::FuncModuleField> funcIncr =
		Internal::make_unique<wabt::FuncModuleField>();

	funcIncr->func.decl.sig.param_types.push_back(wabt::Type::I64);
	// - -> local.get 0
	// - -> global.get $counter
	// - -> i64.add
	// - -> global.set $counter
	// - -> block
	// - ->		global.get $counter
	// - ->		global.get $threshold
	// - ->		i64.lt_u
	// - -> 	br_if 0
	// - ->		global.get $counter
	// - ->		call $ctr_exceed
	// - -> end
	funcIncr->func.exprs.push_back(
		Internal::make_unique<wabt::LocalGetExpr>(
			wabt::Var(wabt::Index(0))));
	funcIncr->func.exprs.push_back(
		Internal::make_unique<wabt::GlobalGetExpr>(
			wabt::Var(static_cast<wabt::Index>(info.m_ctrId))));
	funcIncr->func.exprs.push_back(
		Internal::make_unique<wabt::BinaryExpr>(
			wabt::Opcode::I64Add));
	funcIncr->func.exprs.push_back(
		Internal::make_unique<wabt::GlobalSetExpr>(
			wabt::Var(static_cast<wabt::Index>(info.m_ctrId))));
	// block
	funcIncr->func.exprs.push_back(
		Internal::make_unique<wabt::BlockExpr>());
	wabt::Block& funcIncrBlock01 =
		wabt::cast<wabt::BlockExpr>(&funcIncr->func.exprs.back())->block;
	funcIncrBlock01.exprs.push_back(
		Internal::make_unique<wabt::GlobalGetExpr>(
			wabt::Var(static_cast<wabt::Index>(info.m_ctrId))));
	funcIncrBlock01.exprs.push_back(
		Internal::make_unique<wabt::GlobalGetExpr>(
			wabt::Var(static_cast<wabt::Index>(info.m_thrId))));
	funcIncrBlock01.exprs.push_back(
		Internal::make_unique<wabt::BinaryExpr>(
			wabt::Opcode::I64LeU));
	funcIncrBlock01.exprs.push_back(
		Internal::make_unique<wabt::BrIfExpr>(
			wabt::Var(wabt::Index(0))));
	funcIncrBlock01.exprs.push_back(
		Internal::make_unique<wabt::GlobalGetExpr>(
			wabt::Var(static_cast<wabt::Index>(info.m_ctrId))));
	funcIncrBlock01.exprs.push_back(
		Internal::make_unique<wabt::CallExpr>(
			wabt::Var(static_cast<wabt::Index>(info.m_funcExceedId))));

	AddFuncTypeIfNotExist(funcIncr->func.decl.sig, mod);

	mod.AppendField(std::move(funcIncr));

	return info;
}

inline void InjectBlockCounterExpr(
	wabt::ExprList& exprList,
	wabt::ExprList::iterator exprIt,
	size_t weight,
	wabt::Index ctrFuncIdx)
{
	// i64.const weight
	// call $incr

	exprIt = exprList.insert(exprIt,
		Internal::make_unique<wabt::CallExpr>(
			wabt::Var(ctrFuncIdx)));
	exprIt = exprList.insert(exprIt,
		Internal::make_unique<wabt::ConstExpr>(
			wabt::Const::I64(weight)));
}

inline void InjectBlockCounter(Block* head, wabt::Index ctrFuncIdx)
{
	if ((head != nullptr))
	{
		if (!head->m_isWeightCalc)
		{
			throw Exception("The block weight is not calculated");
		}

		if (!head->m_isCtrInjected)
		{
			head->m_isCtrInjected = true;

			if (head->m_weight > 0)
			{
				// Only inject if weight > 0

				if (IsEffectiveControlFlowExpr(head->m_blkLstExprType) &&
					!IsBlockLikeDecl(head->m_blkLstExprType))
				{
					// Last statement is a branch expr
					auto exprBeforeBr = head->GetBlkLastExpr(1);

					InjectBlockCounterExpr(*head->m_exprList,
						exprBeforeBr,
						head->m_weight,
						ctrFuncIdx);
				}
				else
				{
					InjectBlockCounterExpr(*head->m_exprList,
						head->m_blkEnd,
						head->m_weight,
						ctrFuncIdx);
				}
			}

			// Recursive on children
			for (auto& child : head->m_children)
			{
				InjectBlockCounter(child.m_ptr, ctrFuncIdx);
			}
		}
	}
}

} // namespace DecentWasmCounter
