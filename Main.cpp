#include "CFGParser.hpp"
#include <chrono>
#include <iostream>


int main(int argc, char* argv[])
{
	const auto iter_begin = std::chrono::steady_clock::now();
	CFGParser cfg("test.cfg");
	const auto iter_end = std::chrono::steady_clock::now();

	const auto iter_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(iter_end - iter_begin).count();

	std::cout << "Elapsed time: " << iter_elapsed_time << std::endl;

	const auto& aray = cfg.getArray<int>("test", "array");
	const auto& str = cfg.getString("uqpjny", "test_string");

	cfg.set("test", "val", 24);

	std::cin.get();

	return 0;
}
