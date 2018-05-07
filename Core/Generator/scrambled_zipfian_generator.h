//
//  scrambled_zipfian_generator.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/8/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#ifndef YCSB_C_SCRAMBLED_ZIPFIAN_GENERATOR_H_
#define YCSB_C_SCRAMBLED_ZIPFIAN_GENERATOR_H_

#include "generator.h"

#include <cstdint>
#include "zipfian_generator.h"
#include "Util/funcs.h"

namespace Ycsb {
namespace Core {

class ScrambledZipfianGenerator : public Generator<uint64_t> {
 public:
  ScrambledZipfianGenerator(uint64_t min, uint64_t max,
      double zipfian_const = ZipfianGenerator::kZipfianConst) :
      base_(min), num_items_(max - min + 1),
      generator_(min, max, zipfian_const) { }
  
  ScrambledZipfianGenerator(uint64_t num_items) :
	  base_(0), num_items_(num_items),
	        generator_(0, num_items -1 , ZipfianGenerator::kZipfianConst) { }
  
  uint64_t Next();
  uint64_t Last() { return last_; }
  
 private:
  uint64_t base_;
  uint64_t num_items_;
  ZipfianGenerator generator_;
  uint64_t last_;
};

inline uint64_t ScrambledZipfianGenerator::Next() {
  uint64_t value = generator_.Next();
  value = base_ + Util::FNVHash64(value) % num_items_;
	return last_ = value;
}

} // Core
} // Ycsb

#endif // YCSB_C_SCRAMBLED_ZIPFIAN_GENERATOR_H_
