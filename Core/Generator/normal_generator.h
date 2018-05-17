/*
 * normal_generator.h
 *
 *  Created on: May 17, 2018
 *      Author: neal
 */

#ifndef NORMAL_GENERATOR_H_
#define NORMAL_GENERATOR_H_


#include "generator.h"
#include <random>

namespace Ycsb {
namespace Core {

class NormalGenerator: public Generator<uint64_t> {
public:
	// zscore 3.72 represent 99.99% to data within min to max
	constexpr static double zscore = 3.72;
	/*
	 * Both min and max are inclusive
	 */
	NormalGenerator(uint64_t min, uint64_t max)
			: dist_((min+max)/2.0, (max-min)/(2.0*zscore)) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC_RAW, &now);
		generator_ = std::default_random_engine(now.tv_nsec);
		Next();
	}

	uint64_t Next() {
		return last_int_ = uint64_t(dist_(generator_));
	}
	uint64_t Last() {
		return last_int_;
	}

private:
	uint64_t last_int_;
	std::default_random_engine generator_;
	std::normal_distribution<double> dist_;
};

} // Core
} // Ycsb


#endif /* NORMAL_GENERATOR_H_ */
