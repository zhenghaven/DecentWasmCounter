// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <DecentWasmCounter/DecentWasmCounter.hpp>

#include <src/error.h>
#include <src/feature.h>
#include <src/result.h>
#include <src/shared-validator.h>
#include <src/validator.h>

#include "BlockGenerator.hpp"
#include "CodeInjector.hpp"
#include "WeightCalculator.hpp"

namespace DecentWasmCounter
{

static void InstrumentFunc(
	wabt::Func& func,
	const ImportFuncInfo& funcInfo,
	const InjectedSymbolInfo& symInfo)
{
	// Generate block flow graph
	Graph gr = GenerateGraph(func);

	// Calculate weight for each block
	WeightCalculator wCalc(GetDefaultExprWeightCalcMap(), 0);
	wCalc.CalcWeight(gr.m_head, funcInfo);

	// Inject counting code
	InjectBlockCounter(gr.m_head, symInfo.m_funcIncrId);
}

static void PostValidateModule(const wabt::Module& mod)
{
	wabt::Features features;
	wabt::ValidateOptions options(features);
	wabt::Errors errors;
	wabt::Result result = wabt::ValidateModule(&mod, &errors, options);
	if (!wabt::Succeeded(result))
	{
		std::string errMsg;
		for (const auto& err : errors)
		{
			errMsg += (err.message + '\n');
		}
		throw Exception(
			"Failed to validate the generated module:\n" +
			errMsg);
	}
}

static ImportFuncListType GetImportFuncList(
	const std::vector<wabt::Import*>& imps)
{
	ImportFuncListType list;
	for (const wabt::Import* imp : imps)
	{
		if (imp->kind() == wabt::ExternalKind::Func)
		{
			const wabt::FuncImport* funcImp =
				wabt::cast<const wabt::FuncImport>(imp);
			list.emplace_back(funcImp->module_name, funcImp->field_name);
		}
	}

	return list;
}

} // namespace DecentWasmCounter

void DecentWasmCounter::Instrument(wabt::Module& mod)
{
	// Inject counter and functions
	auto symInfo = InjectCounterAndFunc(mod);

	// Generate import function info
	auto impFuncList = GetImportFuncList(mod.imports);
	ImportFuncInfo funcInfo{ mod.func_bindings, impFuncList };

	// Instrument code
	size_t funcIdx = 0;
	for (wabt::ModuleField& field : mod.fields)
	{
		switch (field.type())
		{
		case wabt::ModuleFieldType::Func:
			if (funcIdx != symInfo.m_funcIncrId)
			{
				wabt::Func& func =
					wabt::cast<wabt::FuncModuleField>(&field)->func;
				InstrumentFunc(func, funcInfo, symInfo);
			}
			++funcIdx;
			break;
		case wabt::ModuleFieldType::Import:
			if (wabt::cast<wabt::ImportModuleField>(&field)->import->kind() ==
				wabt::ExternalKind::Func)
			{
				++funcIdx;
			}
			break;
		default:
			break;
		}
	}

	// validate generated module
	PostValidateModule(mod);
}
