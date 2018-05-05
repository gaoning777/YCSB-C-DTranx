#include "gtest/gtest.h"
#include "zipfian_generator.h"

TEST(ZipfianGenerator, Next){
	Ycsb::Core::ZipfianGenerator zipfian_generator(1, 10);
	zipfian_generator.Next();
}
