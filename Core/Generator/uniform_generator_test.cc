#include "gtest/gtest.h"
#include "uniform_generator.h"
#include <fstream>

TEST(UniformGenerator, Next){
	Ycsb::Core::UniformGenerator uniform_generator(1, 100);
	std::ofstream ofile;
	ofile.open("./udata");
	for(int i=0; i<50000; ++i){
		ofile << uniform_generator.Next() << std::endl;
	}
	ofile.close();
	std::remove("./udata");
}
