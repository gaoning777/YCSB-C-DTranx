/*
 * poisson_generator.h
 *
 *  Created on: May 7, 2018
 *      Author: neal
 */

#ifndef POISSON_GENERATOR_H_
#define POISSON_GENERATOR_H_


#include "generator.h"
#include <random>

namespace Ycsb {
namespace Core {

class PoissonGenerator: public Generator<uint64_t> {
public:
	/*
	 * Both min and max are inclusive
	 */
	PoissonGenerator(uint64_t avg)
			: dist_(avg) {
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
	std::default_random_engine generator_;
	std::poisson_distribution<uint64_t> dist_;
};

} // Core
} // Ycsb


#endif /* POISSON_GENERATOR_H_ */
