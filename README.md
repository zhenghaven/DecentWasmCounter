# Decent WASM Instruction Counter

# Intro

This library accepts a `wabt::Module` instance, and insert additional WASM code
to count the instructions that have been executed.

The idea of WASM instruction counting is similar to
[https://github.com/ibr-ds/AccTEE](https://github.com/ibr-ds/AccTEE/),
while we use a different approach to analysis the code and insert the counting
code.
Besides, we not only count the number of instructions executed, but we also
allow the WASM runtime to be notified when the this counter has exceeded some
pre-defined threshold.

[Detailed Documentation can be found at docs/README.md](docs/README.md)

## Code Status
- [![Unit Tests](https://github.com/zhenghaven/DecentWasmCounter/actions/workflows/unit-tests.yaml/badge.svg?branch=main)](https://github.com/zhenghaven/DecentWasmCounter/actions/workflows/unit-tests.yaml)
	- Testing environments
		- OS: `ubuntu-latest`, `windows-latest`
