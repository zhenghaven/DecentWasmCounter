// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <DecentWasmWat/WasmWat.h>

namespace DecentWasmCounter
{

void Instrument(wabt::Module& mod);

} // namespace DecentWasmCounter
