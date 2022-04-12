// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <src/ir.h>

#include <DecentWasmCounter/Exceptions.hpp>

namespace DecentWasmCounter
{

inline bool IsEffectiveControlFlowExpr(wabt::ExprType exprType)
{
	// TODO: double check these instruction types
	switch (exprType)
	{
	// TODO: check if these instruction has effect on the execution flow
	case wabt::ExprType::AtomicLoad:
	case wabt::ExprType::AtomicRmw:
	case wabt::ExprType::AtomicRmwCmpxchg:
	case wabt::ExprType::AtomicStore:
	case wabt::ExprType::AtomicNotify:
	case wabt::ExprType::AtomicFence:
	case wabt::ExprType::AtomicWait:
		throw Exception("Unimplemented feature");

	// non-control flow
	case wabt::ExprType::Binary:
		return false;

	// control flow
	case wabt::ExprType::Block:
	case wabt::ExprType::Br:
	case wabt::ExprType::BrIf:
	case wabt::ExprType::BrTable:
		return true;

	// these ARE control flow expr, but it doesn't affect our block flow
	case wabt::ExprType::Call:
	case wabt::ExprType::CallIndirect:
	case wabt::ExprType::CallRef:
		return false;

	// non-control flow
	case wabt::ExprType::CodeMetadata:
	case wabt::ExprType::Compare:
	case wabt::ExprType::Const:
	case wabt::ExprType::Convert:
	case wabt::ExprType::Drop:
	case wabt::ExprType::GlobalGet:
	case wabt::ExprType::GlobalSet:
		return false;

	// TODO: check if these instruction has effect on the execution flow
	case wabt::ExprType::If:
		throw Exception("Unimplemented feature");

	// non-control flow
	case wabt::ExprType::Load:
	case wabt::ExprType::LocalGet:
	case wabt::ExprType::LocalSet:
	case wabt::ExprType::LocalTee:
		return false;

	// control flow
	case wabt::ExprType::Loop:
		return true;

	// non-control flow
	case wabt::ExprType::MemoryCopy:
	case wabt::ExprType::DataDrop:
	case wabt::ExprType::MemoryFill:
	case wabt::ExprType::MemoryGrow:
	case wabt::ExprType::MemoryInit:
	case wabt::ExprType::MemorySize:
	case wabt::ExprType::Nop:
	case wabt::ExprType::RefIsNull:
	case wabt::ExprType::RefFunc:
	case wabt::ExprType::RefNull:
		return false;

	// TODO: check if these instruction has effect on the execution flow
	case wabt::ExprType::Rethrow:
		throw Exception("Unimplemented feature");

	// control flow
	case wabt::ExprType::Return:
		return true;

	// TODO: check if these instruction has effect on the execution flow
	case wabt::ExprType::ReturnCall:
	case wabt::ExprType::ReturnCallIndirect:
	case wabt::ExprType::Select:
	case wabt::ExprType::SimdLaneOp:
	case wabt::ExprType::SimdLoadLane:
	case wabt::ExprType::SimdStoreLane:
	case wabt::ExprType::SimdShuffleOp:
	case wabt::ExprType::LoadSplat:
	case wabt::ExprType::LoadZero:
		throw Exception("Unimplemented feature");

	// non-control flow
	case wabt::ExprType::Store:
	case wabt::ExprType::TableCopy:
	case wabt::ExprType::ElemDrop:
	case wabt::ExprType::TableInit:
	case wabt::ExprType::TableGet:
	case wabt::ExprType::TableGrow:
	case wabt::ExprType::TableSize:
	case wabt::ExprType::TableSet:
	case wabt::ExprType::TableFill:
	case wabt::ExprType::Ternary:
		return false;

	// TODO: check if these instruction has effect on the execution flow
	case wabt::ExprType::Throw:
	case wabt::ExprType::Try:
		throw Exception("Unimplemented feature");

	// non-control flow
	case wabt::ExprType::Unary:
	case wabt::ExprType::Unreachable:
		return false;

	default:
		throw Exception("Unimplemented feature");
	}
}

inline bool IsBlockLikeDecl(wabt::ExprType exprType)
{
	if (IsEffectiveControlFlowExpr(exprType))
	{
		switch (exprType)
		{
		case wabt::ExprType::Block:
		case wabt::ExprType::Loop:
			return true;
		default:
			return false;
		}
	}
	return false;
}

} // namespace DecentWasmCounter
