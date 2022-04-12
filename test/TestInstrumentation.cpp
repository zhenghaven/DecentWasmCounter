// Copyright 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <gtest/gtest.h>

#include <DecentWasmWat/WasmWat.h>

#include <DecentWasmCounter/DecentWasmCounter.hpp>

#include "Common.hpp"

using namespace DecentWasmCounter;

namespace DecentWasmCounter_Test
{
	extern size_t g_numOfTestFile;
}

GTEST_TEST(TestInstrumentation, CountTestFile)
{
	static auto tmp = ++DecentWasmCounter_Test::g_numOfTestFile;
}

GTEST_TEST(TestInstrumentation, TestInput_01)
{
	auto testInWatStr_01 =
		ReadFile2Buffer<std::string>("../../test/test_wats/test-01.in.wat");
	auto testInWatStr_01_nopt =
		ReadFile2Buffer<std::string>("../../test/test_wats/test-01.out.nopt.wat");

	auto mod = DecentWasmWat::Wat2Mod(
		"filename.wat", testInWatStr_01, DecentWasmWat::Wat2WasmConfig());

	EXPECT_NO_THROW(DecentWasmCounter::Instrument(*(mod.m_ptr)));

	auto testOutWatStr_01 =
		DecentWasmWat::Mod2Wat(*(mod.m_ptr), DecentWasmWat::Wasm2WatConfig());

	EXPECT_EQ(testOutWatStr_01, testInWatStr_01_nopt);
}

GTEST_TEST(TestInstrumentation, TestInput_02)
{
	auto testInWatStr_02 =
		ReadFile2Buffer<std::string>("../../test/test_wats/test-02.in.wat");
	auto testInWatStr_02_nopt =
		ReadFile2Buffer<std::string>("../../test/test_wats/test-02.out.nopt.wat");

	auto mod = DecentWasmWat::Wat2Mod(
		"filename.wat", testInWatStr_02, DecentWasmWat::Wat2WasmConfig());

	EXPECT_NO_THROW(DecentWasmCounter::Instrument(*(mod.m_ptr)));

	auto testOutWatStr_02 =
		DecentWasmWat::Mod2Wat(*(mod.m_ptr), DecentWasmWat::Wasm2WatConfig());

	EXPECT_EQ(testOutWatStr_02, testInWatStr_02_nopt);
}

GTEST_TEST(TestInstrumentation, TestInput_03)
{
	auto testInWatStr_03 =
		ReadFile2Buffer<std::string>("../../test/test_wats/test-03.in.wat");
	auto testInWatStr_03_nopt =
		ReadFile2Buffer<std::string>("../../test/test_wats/test-03.out.nopt.wat");

	auto mod = DecentWasmWat::Wat2Mod(
		"filename.wat", testInWatStr_03, DecentWasmWat::Wat2WasmConfig());

	EXPECT_NO_THROW(DecentWasmCounter::Instrument(*(mod.m_ptr)));

	auto testOutWatStr_03 =
		DecentWasmWat::Mod2Wat(*(mod.m_ptr), DecentWasmWat::Wasm2WatConfig());

	EXPECT_EQ(testOutWatStr_03, testInWatStr_03_nopt);
}
