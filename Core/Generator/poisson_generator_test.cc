#include "gtest/gtest.h"
#include "poisson_generator.h"
#include <fstream>

TEST(PoissonGenerator, Next){
	Ycsb::Core::PoissonGenerator poisson_generator(50);
	std::ofstream ofile;
	ofile.open("./pdata");
	for(int i=0; i<50000; ++i){
		ofile << poisson_generator.Next() << std::endl;
	}
	ofile.close();
	std::remove("./pdata");
}
