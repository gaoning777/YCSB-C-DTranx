#include "gtest/gtest.h"
#include "scrambled_zipfian_generator.h"
#include <fstream>

TEST(ScrambledZipfianGenerator, Next){
	Ycsb::Core::ScrambledZipfianGenerator szipfian_generator(1, 100);
	std::ofstream ofile;
	ofile.open("./szdata");
	for(int i=0; i<50000; ++i){
		ofile << szipfian_generator.Next() << std::endl;
	}
	ofile.close();
	std::remove("./szdata");
}
