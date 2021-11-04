#pragma once

#include <chrono>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <random>

#include "src/Compiler/Tokenizer.h"

namespace benchmark {
	namespace tokenizer {
		constexpr int benchSize = 100'000;
		constexpr int repeats = 20;

		std::string scale(long double ticks) {
			std::ostringstream o;
			if (ticks > 1'000'000'000) {
				o << (ticks / 1'000'000'000) << "s";
			} else if (ticks > 1'000'000) {
				o << (ticks / 1'000'000) << "ms";
			} else if (ticks > 1'000) {
				o << (ticks / 1'000) << "us";
			} else {
				o << ticks << "ns";
			}

			return o.str();
		}

		long double avg(const std::vector<long double>& v) {
			return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
		}

		void printResults(const std::string& name, std::vector<long double>& times) {
			const auto mid = times.begin() + (times.size() / 2);
			std::nth_element(times.begin(), mid, times.end());

			std::cout << "Benchmark results (" << name << "):\n";
			std::cout << "\tlongest:  " << scale(*std::max_element(times.begin(), times.end())) << "\n";
			std::cout << "\tshortest: " << scale(*std::min_element(times.begin(), times.end())) << "\n";
			std::cout << "\tmedian:   " << scale(*mid) << "\n";
			std::cout << "\taverage:  " << scale(std::accumulate(times.begin(), times.end(), 0.0) / times.size()) << "\n\n";
		}

		void test(std::string&& code, const std::string& name) {
			std::vector<long double> times;
			times.reserve(repeats);
			for (int i = 0; i < repeats; i++) {
				compiler::Tokenizer tokenizer(code);

				const auto start = std::chrono::steady_clock::now();
				while (tokenizer.next(base::OpCode::LOAD_LITERAL).opCode != base::OpCode::END_PROGRAM) {
					;
				}
				const auto end = std::chrono::steady_clock::now();
				times.push_back((end - start).count());
			}
			printResults(name, times);
		}

		void testInts() {
			std::mt19937 gen(0);
			std::uniform_int_distribution<base::sm_uint> distrib(0, std::numeric_limits<base::sm_int>::max());

			std::string code;
			for (int i = 0; i < benchSize; i++) {
				code += " " + std::to_string(distrib(gen));
			}

			test(std::move(code), __func__);
		}

		void testUints() {
			std::mt19937 gen(0);
			std::uniform_int_distribution<base::sm_uint> distrib(0, std::numeric_limits<base::sm_uint>::max());

			std::string code;
			for (int i = 0; i < benchSize; i++) {
				code += " " + std::to_string(distrib(gen)) + "u";
			}

			test(std::move(code), __func__);
		}

		void testFloats() {
			std::mt19937 gen(0);
			std::uniform_real_distribution<sm_float> distrib(0.0, std::numeric_limits<base::sm_float>::max());

			std::string code;
			for (int i = 0; i < benchSize; i++) {
				code += " " + std::to_string(distrib(gen));
			}

			test(std::move(code), __func__);
		}

		void testBools() {
			std::mt19937 gen(0);
			std::uniform_int_distribution<int> distrib(0, 1);

			std::string code;
			for (int i = 0; i < benchSize; i++) {
				code += (distrib(gen) == 1 ? " true" : " false");
			}

			test(std::move(code), __func__);
		}

		void run() {
			testInts();
			testUints();
			testFloats();
			testBools();
		}
	}
}