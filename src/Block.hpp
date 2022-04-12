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

#include "Classification.hpp"

namespace DecentWasmCounter
{

enum class BlockType
{
	Func,
	Block,
	Loop,
}; // enum class BlockType

enum class BrType
{
	Normal, // Normal branch that doesn't involve loop
	IntoLoop, // Branch into the loop
	OutOfLoop, // Branch out of the loop
}; // enum class BrType

struct Block;

struct BlockChild
{
	BlockChild(BrType brType, BrType cntType, Block* ptr) :
		m_brType(brType),
		m_cntType(cntType),
		m_ptr(ptr)
	{}

	BrType m_brType; // What is the type of the branch?
	BrType m_cntType;
	Block* m_ptr; // pointer to the child block
}; // struct BlockChild

struct BlockParent
{
	Block* m_ptr; // pointer to the parent block
}; // struct BlockParent

struct Block
{
	Block(BlockType type,
		wabt::ExprList& expr,
		wabt::ExprList::iterator blkBegin) :
		m_type(type),
		m_isLoopHead(false),
		m_exprList(&expr),
		m_exprBegin(m_exprList->begin()),
		m_exprEnd(m_exprList->end()),
		m_blkBegin(blkBegin),
		m_blkEnd(blkBegin),
		m_blkFstExprType(
			blkBegin != m_exprEnd ?
			blkBegin->type() :
			wabt::ExprType::Unreachable),
		m_blkLstExprType(m_blkFstExprType),
		m_isWeightCalc(false),
		m_weight(0),
		m_isCtrInjected(false),
		m_parents(),
		m_children()
	{}

	/**
	 * @brief: Starting from m_blkBegin until m_exprEnd
	 *         to look for the end of this block
	*/
	void ExpandBlock()
	{
		if ((m_blkBegin != m_exprEnd) &&
			IsBlockLikeDecl(m_blkFstExprType))
		{
			// If the beginning is block declaration
			// we want to advance by 1
			m_blkLstExprType = m_blkFstExprType;
			m_blkEnd = m_blkBegin;
			++m_blkEnd;
		}
		else
		{
			// Else, we search for the end
			for (m_blkEnd = m_blkBegin; (m_blkEnd != m_exprEnd); ++m_blkEnd)
			{
				if (IsEffectiveControlFlowExpr(m_blkEnd->type()))
				{
					// If it's a control flow expr, we found an end
					if (IsBlockLikeDecl(m_blkEnd->type()))
					{
						// If it's block declaration, don't include it (stop here)
						return;
					}
					else
					{
						// else, it's branch/jump expr, include it (advance by 1)
						m_blkLstExprType = m_blkEnd->type();
						++m_blkEnd;
						return;
					}
				}
				m_blkLstExprType = m_blkEnd->type();
			}
		}
	}

	bool IsEmpty() const
	{
		return m_blkBegin == m_blkEnd;
	}

	bool IsBlkEndsOnExprList() const
	{
		return m_blkEnd == m_exprEnd;
	}

	wabt::ExprList::iterator GetBlkLastExpr(size_t i) const
	{
		auto it = m_blkEnd;
		while ((it != m_blkBegin) && i > 0)
		{
			--it;
			--i;
		}
		if (i > 0)
		{
			throw Exception("The expr looking for is out of range");
		}
		return it;
	}

	BlockType m_type;
	bool m_isLoopHead;
	wabt::ExprList* m_exprList;
	wabt::ExprList::iterator m_exprBegin;
	wabt::ExprList::iterator m_exprEnd;
	wabt::ExprList::iterator m_blkBegin;
	wabt::ExprList::iterator m_blkEnd;
	wabt::ExprType m_blkFstExprType;
	wabt::ExprType m_blkLstExprType;

	bool m_isWeightCalc;
	size_t m_weight;

	bool m_isCtrInjected;

	std::vector<BlockParent> m_parents;
	std::vector<BlockChild> m_children;

}; // struct Block

struct BlockStorage
{
	void Append(std::unique_ptr<Block> b)
	{
		m_vec.emplace_back(std::move(b));
	}

	std::vector<std::unique_ptr<Block> > m_vec;
};

struct Graph
{
	Graph() :
		m_storage(),
		m_head(nullptr)
	{}

	BlockStorage m_storage;
	Block* m_head;
}; // struct Graph

struct BrBinding
{
	std::string m_name;
	Block* m_blk;
	size_t m_blkLvl;
}; // struct BrBinding

} // namespace DecentWasmCounter
