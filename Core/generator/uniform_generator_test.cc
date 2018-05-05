#include "gtest/gtest.h"
#include "uniform_generator.h"

TEST(UniformGenerator, Next){
	Ycsb::Core::UniformGenerator uniform_generator(1, 10);
	uniform_generator.Next();
}
