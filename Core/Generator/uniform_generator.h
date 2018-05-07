//
//  uniform_generator.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/6/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#ifndef YCSB_C_UNIFORM_GENERATOR_H_
#define YCSB_C_UNIFORM_GENERATOR_H_

#include "generator.h"
#include <random>

namespace Ycsb {
namespace Core {

/*
 * UniformGenerator is adapted by constructing std::mt19937_64 with
 * different int numbers.
 * Now if multiple threads create multiple UniformGenerators, the random
 * number series will be also different
 */
class UniformGenerator: public Generator<uint64_t> {
public:
	/*
	 * Both min and max are inclusive
	 */
	UniformGenerator(uint64_t min, uint64_t max)
			: dist_(min, max) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC_RAW, &now);
		generator_ = std::default_random_engine(now.tv_nsec);
		Next();
	}

	uint64_t Next() {
		return last_int_ = dist_(generator_);
	}
	uint64_t Last() {
		return last_int_;
	}

private:
	uint64_t last_int_;
	//std::mt19937_64 generator_;
	std::default_random_engine generator_;
	std::uniform_int_distribution<uint64_t> dist_;
};

} // Core
} // Ycsb

#endif // YCSB_C_UNIFORM_GENERATOR_H_
