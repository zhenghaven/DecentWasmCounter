// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <functional>
#include <unordered_map>
#include <vector>

#include "Block.hpp"

namespace DecentWasmCounter
{

using ImportFuncListType = std::vector<std::pair<std::string, std::string> >;
struct ImportFuncInfo
{
	wabt::BindingHash m_nameBinding;
	ImportFuncListType m_funcList;
}; // struct ImportFuncInfo

using ExprWeightCalcFunc = std::function<size_t(
	wabt::ExprList::iterator, // expr to be calculated on
	const Block*, // Block where the expr is from
	const ImportFuncInfo& // Import function info
	)>;
using WeightMapType = std::unordered_map<wabt::ExprType, ExprWeightCalcFunc>;

using CallWeightCalcFunc = std::function<size_t(
	wabt::ExprList::iterator, // expr to be calculated on
	const Block* // Block where the expr is from
)>;
using ImportFuncWeightMap = std::unordered_map<
	std::string,
	std::unordered_map<std::string, CallWeightCalcFunc> >;

template<size_t _weight>
inline constexpr size_t RetConstExprWeight(
	wabt::ExprList::iterator,
	const Block*,
	const ImportFuncInfo&)
{
	return _weight;
}

template<size_t _weight>
inline constexpr size_t RetConstCallWeight(
	wabt::ExprList::iterator,
	const Block*)
{
	return _weight;
}

inline const ImportFuncWeightMap& GetDefaultFuncWeightCalcMap()
{
	static const ImportFuncWeightMap m =
	{
		{ "env",
			{
				{ "decent_wasm_test_log", RetConstCallWeight<10> },
			}
		},
	};
	return m;
}

template<size_t _defaultWeight>
inline size_t RetDefaultCallWeight(
	wabt::ExprList::iterator exprIt,
	const Block* blk,
	const ImportFuncInfo& funcInfo)
{
	const wabt::Expr& expr = *exprIt;
	const auto& impFuncWgtMap = GetDefaultFuncWeightCalcMap();

	if (expr.type() == wabt::ExprType::Call)
	{
		const wabt::CallExpr* callExpr =
			wabt::cast<const wabt::CallExpr>(&expr);
		wabt::Index funcIdx = funcInfo.m_nameBinding.FindIndex(callExpr->var);
		if (funcIdx < funcInfo.m_funcList.size())
		{
			const auto& impFuncName = funcInfo.m_funcList[funcIdx];

			auto itModName = impFuncWgtMap.find(impFuncName.first);
			if (itModName != impFuncWgtMap.cend())
			{
				auto itFieldName = itModName->second.find(impFuncName.second);
				if (itFieldName != itModName->second.cend())
				{
					return itFieldName->second(exprIt, blk);
				}
			}

			// weight is not found in the map
			return _defaultWeight;
		}
		else
		{
			// It's calling in-module func
			return 0;
		}
	}
	else
	{
		throw Exception("The given expr is not a call expr");
	}
}

inline const WeightMapType& GetDefaultExprWeightCalcMap()
{
	static const WeightMapType m =
	{
		{ wabt::ExprType::Binary,  RetConstExprWeight<1> },
		{ wabt::ExprType::Compare, RetConstExprWeight<1> },
		{ wabt::ExprType::Call,    RetDefaultCallWeight<2> },
	};
	return m;
}

class WeightCalculator
{
public:

	WeightCalculator(const WeightMapType& m, size_t defWeight):
		m_weightMap(m),
		m_defaultWeight(defWeight)
	{}

	virtual ~WeightCalculator() = default;

	void CalcWeight(Block* head, const ImportFuncInfo& funcInfo) const
	{
		if ((head != nullptr) && !head->m_isWeightCalc)
		{
			// Calculate weight for this block
			head->m_isWeightCalc = true;
			head->m_weight = 0;
			for (auto it = head->m_blkBegin; it != head->m_blkEnd; ++it)
			{
				const auto& expr = *it;
				auto exprType = expr.type();

				auto itWeight = m_weightMap.find(exprType);
				if (itWeight != m_weightMap.cend())
				{
					head->m_weight += itWeight->second(it, head, funcInfo);
				}
				else
				{
					head->m_weight += m_defaultWeight;
				}
			}

			// Recursive on children
			for (auto& child : head->m_children)
			{
				CalcWeight(child.m_ptr, funcInfo);
			}
		}
	}

private:
	WeightMapType m_weightMap;
	size_t m_defaultWeight;

}; // class WeightCalculator

} // namespace DecentWasmCounter
