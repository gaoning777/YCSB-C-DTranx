#include "gtest/gtest.h"
#include "normal_generator.h"
#include <fstream>

TEST(NormalGenerator, Next){
	Ycsb::Core::NormalGenerator normal_generator(1, 100);
	std::ofstream ofile;
	ofile.open("./ndata");
	for(int i=0; i<50000; ++i){
		ofile << normal_generator.Next() << std::endl;
	}
	ofile.close();
	//std::remove("./ndata");
}
