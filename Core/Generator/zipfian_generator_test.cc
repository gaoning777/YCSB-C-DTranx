#include "gtest/gtest.h"
#include "zipfian_generator.h"
#include <fstream>

TEST(ZipfianGenerator, Next){
	Ycsb::Core::ZipfianGenerator zipfian_generator(1, 100);
	std::ofstream ofile;
	ofile.open("./zdata");
	for(int i=0; i<50000; ++i){
		ofile << zipfian_generator.Next() << std::endl;
	}
	ofile.close();
	std::remove("./zdata");
}
