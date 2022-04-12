// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once

#include <cstdio>

#include <stdexcept>
#include <string>

template<typename _RetType>
inline _RetType ReadFile2Buffer(const std::string& filename)
{
	FILE *file;
	size_t file_size, read_size;

	if ((file = fopen(filename.c_str(), "rb")) == nullptr)
	{
		throw std::runtime_error(
			"Read file to buffer failed: open file " + filename + " failed");
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	_RetType buffer;
	buffer.resize(file_size);

	read_size = fread(&(buffer[0]), 1, file_size, file);
	fclose(file);

	if (read_size < file_size)
	{
		throw std::runtime_error(
			"Read file " + filename + " to buffer failed: read file content failed");
	}

	return buffer;
}
