//
//  basic_db.cc
//  YCSB-C
//
//  Created by Jinglei Ren on 12/17/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#include "DB/db_factory.h"

#include "tabledb/basic_db.h"
#include "tabledb/lock_stl_db.h"
#include "tabledb/tbb_rand_db.h"
#include "tabledb/tbb_scan_db.h"
#include "kvdb/dtranx_db.h"
#include "kvdb/hyperdex_db.h"
#include "kvdb/btree_db.h"
#include "kvdb/rtree_db.h"
#include "kvdb/bang_db.h"
#include "kvdb/cockroach_db.h"
namespace Ycsb {
namespace DB {

DB_BASE* DBFactory::CreateDB(std::string dbname, Ycsb::Core::CoreWorkload &wl) {
	if (dbname == "basic") {
		return new BasicDB;
	} else if (dbname == "lock_stl") {
		return new LockStlDB;
	} else if (dbname == "tbb_rand") {
		return new TbbRandDB;
	} else if (dbname == "tbb_scan") {
		return new TbbScanDB;
	} else if (dbname == "dtranx") {
		return new DtranxDB;
	} else if (dbname == "hyperdex") {
		return new HyperdexDB;
	} else if (dbname == "bangdb") {
		return new BangDB;
	} else if (dbname == "cockroach") {
		return new CockroachDB;
	} else if (dbname == "btree") {
		return new BtreeDB(wl);
	} else if (dbname == "rtree") {
		return new RtreeDB(wl);
	} else
		return NULL;
}

}
}
