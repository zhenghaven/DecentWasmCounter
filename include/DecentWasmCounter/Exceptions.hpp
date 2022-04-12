// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <stdexcept>

namespace DecentWasmCounter
{

class Exception : public std::runtime_error
{
public:
	explicit Exception(const std::string errMsg) :
		std::runtime_error(errMsg)
	{}

	virtual ~Exception() = default;
};

} // namespace DecentWasmCounter
