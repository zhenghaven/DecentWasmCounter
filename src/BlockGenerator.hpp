// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <memory>
#include <vector>

#include <src/cast.h>
#include <src/ir.h>

#include <DecentWasmCounter/Exceptions.hpp>

#include "Block.hpp"
#include "Classification.hpp"
#include "make_unique.hpp"

namespace DecentWasmCounter
{

inline BrType CheckContBlockBrType(
	const std::vector<BrBinding>& scopeStack,
	size_t contBlockLvl)
{
	// search backwards
	bool passLoop = false;
	size_t idx = contBlockLvl;
	for (auto it = scopeStack.rbegin();
		(it != scopeStack.rend()) && (idx < scopeStack.size()); ++it, ++idx)
	{
		passLoop = passLoop || (it->m_blk->m_isLoopHead);
	}

	return passLoop ? BrType::OutOfLoop : BrType::Normal;
}

inline BlockChild FindBrDestination(
	const std::vector<BrBinding>& scopeStack,
	const wabt::Index& var)
{
	bool passLoop = false;

	wabt::Index idx = var;

	for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it, --idx)
	{
		if (idx == 0)
		{
			BrType brType = (it->m_blk->m_isLoopHead) ? BrType::IntoLoop :
				(passLoop ? BrType::OutOfLoop : BrType::Normal);
			BrType cntType = (it->m_blk->m_isLoopHead) ? BrType::IntoLoop :
				CheckContBlockBrType(scopeStack, it->m_blkLvl);

			return BlockChild(brType, cntType, it->m_blk);
		}

		passLoop = passLoop || (it->m_blk->m_isLoopHead);
	}

	throw Exception("Branch to an index that is out of range");
}

inline BlockChild FindBrDestination(
	const std::vector<BrBinding>& scopeStack,
	const std::string& name)
{
	bool passLoop = false;

	for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it)
	{
		if (it->m_name == name)
		{
			BrType brType = (it->m_blk->m_isLoopHead) ? BrType::IntoLoop :
				(passLoop ? BrType::OutOfLoop : BrType::Normal);
			BrType cntType = (it->m_blk->m_isLoopHead) ? BrType::IntoLoop :
				CheckContBlockBrType(scopeStack, it->m_blkLvl);

			return BlockChild(brType, cntType, it->m_blk);
		}

		passLoop = passLoop || (it->m_blk->m_isLoopHead);
	}

	throw Exception("Branch to an name that is not found");
}

inline BlockChild FindBrDestination(
	const std::vector<BrBinding>& scopeStack,
	const wabt::Var& var)
{
	bool passLoop = false;

	if (var.is_index())
	{
		return FindBrDestination(scopeStack, var.index());
	}
	else if (var.is_name())
	{
		return FindBrDestination(scopeStack, var.name());
	}
	else
	{
		throw Exception("Unkown var type");
	}
}

// Returns the head block of the given expr
// We only return 1 pointer to head block,
// since is only one entry point to Func/Block/Loop
inline Block* GenerateGraph(
	BlockType blkType,
	wabt::ExprList& exprList,
	BlockStorage& storage,
	std::vector<BrBinding>& scopeStack,
	size_t contBlockLvl,
	Block* contBlock)
{
	auto exprBegin = exprList.begin();
	auto exprEnd = exprList.end();

	// # create a stack of blocks, where the last block is on the top
	//   so it will be processed first
	std::vector<std::unique_ptr<Block> > blockStack;

	auto it = exprBegin;
	while (it != exprEnd)
	{
		// Create new block
		std::unique_ptr<Block> blk = Internal::make_unique<Block>(
			blkType,
			exprList,
			it);

		// expand block
		blk->ExpandBlock();

		// Set the next begining to the end of this block
		it = blk->m_blkEnd;

		if (!blk->IsEmpty())
		{
			// We need to keep the block if:
			// !isEmpty || (!notEnd && isEffectiveCtrlFlow)
			blockStack.emplace_back(std::move(blk));
		}
	}

	// Work from the stack top
	// check blkBegin is block/loop
	// -> if so, recursive call
	// -> if not, check blkEnd is br/br_if/return
	// -> -> if so, connect child
	//       (return has 0 child, br has 1, br_if has 2)
	// -> -> if not,

	// This should point to the block that we should flow to
	// when all current expr are executed
	Block* head = contBlock;
	size_t headLvl = contBlockLvl;

	while (blockStack.size() > 0)
	{
		std::unique_ptr<Block> blk = std::move(blockStack.back());
		blockStack.pop_back();

		if (IsBlockLikeDecl(blk->m_blkFstExprType))
		{
			// It is some type of block
			switch (blk->m_blkFstExprType)
			{
			case wabt::ExprType::Block:
			{
				const wabt::BlockExpr* blkExpr =
					wabt::cast<const wabt::BlockExpr>(&(*(blk->m_blkBegin)));
				wabt::ExprList& blkExprList =
					const_cast<wabt::ExprList&>(blkExpr->block.exprs);

				// For block, br/br_if 0 should also points to head
				scopeStack.push_back({ blkExpr->block.label, head, headLvl });

				Block* tmpHead = GenerateGraph(BlockType::Block, blkExprList,
					storage, scopeStack,
					headLvl, head);

				scopeStack.pop_back();

				if (tmpHead != head)
				{
					// head is updated
					head = tmpHead;
					headLvl = scopeStack.size();
				}
				break;
			}
			case wabt::ExprType::Loop:
			{
				const wabt::LoopExpr* lpExpr =
					wabt::cast<const wabt::LoopExpr>(&(*(blk->m_blkBegin)));
				wabt::ExprList& lpExprList =
					const_cast<wabt::ExprList&>(lpExpr->block.exprs);

				// For loop, br/br_if 0 should points to loop itself
				blk->m_isLoopHead = true;
				Block* lpPtr = blk.get();
				storage.Append(std::move(blk));
				scopeStack.push_back({ lpExpr->block.label, lpPtr, scopeStack.size() });

				Block* tmpHead = GenerateGraph(BlockType::Loop, lpExprList,
					storage, scopeStack,
					headLvl, head);

				scopeStack.pop_back();

				if (tmpHead != head)
				{
					// head is updated
					head = tmpHead;
					headLvl = scopeStack.size();
				}
				break;
			}
			default:
				throw Exception("Unimplemented feature");
			}
		}
		else
		{
			// It is a list of expr
			if (blk->IsEmpty())
			{
				// empty block, skip
			}
			else
			{
				auto lastExpr = blk->GetBlkLastExpr(1);
				if (IsEffectiveControlFlowExpr(lastExpr->type()))
				{
					// block ends with control flow expr

					// Keep this block
					Block* blkPtr = blk.get();
					storage.Append(std::move(blk));

					switch (lastExpr->type())
					{
					case wabt::ExprType::Br:
					{
						// br always jump, so there is only 1 child
						const wabt::BrExpr* brExpr =
							wabt::cast<const wabt::BrExpr>(&(*lastExpr));

						auto foundRes = FindBrDestination(scopeStack, brExpr->var);

						// set up block flow link
						blkPtr->m_children.push_back(foundRes);
						foundRes.m_ptr->m_parents.push_back({ blkPtr });

						head = blkPtr;
						headLvl = scopeStack.size();
						break;
					}
					case wabt::ExprType::BrIf:
					{
						// br_if may/may not jump, so there are 2 children
						const wabt::BrIfExpr* brExpr =
							wabt::cast<const wabt::BrIfExpr>(&(*lastExpr));

						auto foundRes = FindBrDestination(scopeStack, brExpr->var);

						// set up block flow link
						// -> flow when jump
						blkPtr->m_children.push_back(foundRes);
						foundRes.m_ptr->m_parents.push_back({ blkPtr });
						// -> flow when not jump
						if (head != nullptr)
						{
							blkPtr->m_children.push_back(BlockChild(
								BrType::Normal,
								CheckContBlockBrType(scopeStack, headLvl),
								head
							));
							head->m_parents.push_back({ blkPtr });
						}

						head = blkPtr;
						headLvl = scopeStack.size();
						break;
					}
					//case wabt::ExprType::BrTable:
					case wabt::ExprType::Return:
					{
						// return directly terminate the func, so there is no child

						head = blkPtr;
						headLvl = scopeStack.size();
						break;
					}
					default:
						throw Exception("Unimplemented feature");
					}
				}
				else
				{
					// block ends with non-control-flow expr

					// Keep this block
					Block* blkPtr = blk.get();
					storage.Append(std::move(blk));

					// set up block flow link
					// -> flow to the previous head
					if (head != nullptr)
					{
						blkPtr->m_children.push_back(BlockChild(
							BrType::Normal,
							CheckContBlockBrType(scopeStack, headLvl),
							head
						));
						head->m_parents.push_back({ blkPtr });
					}

					head = blkPtr;
					headLvl = scopeStack.size();
				}
			}
		}
	}

	return head;
}

inline Graph GenerateGraph(wabt::Func& func)
{
	Graph gr;

	std::vector<BrBinding> scopeStack;
	gr.m_head = GenerateGraph(BlockType::Func,
		func.exprs, // expr list
		gr.m_storage, // Storage to keep blocks
		scopeStack, // scope stack for nested blocks/loops
		0, // continous block level 0 i.e., it's outer most level
		nullptr // there is no blocks after all exprs in this func
	);

	return gr;
}

} // namespace DecentWasmCounter
