//
//  ycsbc.cc
//  YCSB-C
//
//  Created by Jinglei Ren on 12/19/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <future>
#include "core/utils.h"
#include "core/timer.h"
#include "core/client.h"
#include "core/core_workload.h"
#include "db/db_factory.h"

using namespace std;

void UsageMessage(const char *command);
bool StrStartWith(const char *str, const char *pre);
string ParseCommandLine(int argc, const char *argv[], utils::Properties &props);

int DelegateClient(ycsbc::DB *db, ycsbc::CoreWorkload *wl, const int num_ops,
bool is_loading, bool isKV, std::string clusterFileName) {
	ycsbc::DtranxDB *dtranx_db = NULL;
	if (isKV) {
		dtranx_db = new ycsbc::DtranxDB();
	}
	if (db) {
		db->Init();
	}
	if (dtranx_db) {
		dtranx_db->Init(clusterFileName);
	}
	ycsbc::Client client(db, dtranx_db, *wl, isKV);
	int oks = 0;
	for (int i = 0; i < num_ops; ++i) {
		if (is_loading) {
			oks += client.DoInsert();
		} else {
			oks += client.DoTransaction();
		}
	}
	if (db) {
		db->Close();
	}
	if (dtranx_db) {
		dtranx_db->Close();
	}
	return oks;
}

int main(const int argc, const char *argv[]) {
	utils::Properties props;
	string file_name = ParseCommandLine(argc, argv, props);
	bool isKV = props["dbname"] == "dtranx";

	ycsbc::DB *db = ycsbc::DBFactory::CreateDB(props["dbname"]);
	/*
	 if (!db) {
	 cout << "Unknown database name " << props["dbname"] << endl;
	 exit(0);
	 }
	 */

	ycsbc::CoreWorkload wl;
	wl.Init(props);

	const int num_threads = stoi(props.GetProperty("threadcount", "1"));

	std::string clusterFileName = props.GetProperty("clusterfilename", "");

	// Loads data
	vector<future<int>> actual_ops;
	int total_ops = stoi(props[ycsbc::CoreWorkload::RECORD_COUNT_PROPERTY]);
	for (int i = 0; i < num_threads; ++i) {
		actual_ops.emplace_back(
				async(launch::async, DelegateClient, db, &wl,
						total_ops / num_threads, true, isKV, clusterFileName));
	}
	assert((int )actual_ops.size() == num_threads);

	int sum = 0;
	for (auto &n : actual_ops) {
		assert(n.valid());
		sum += n.get();
	}
	cerr << "# Loading records:\t" << sum << endl;

	// Peforms transactions
	actual_ops.clear();
	total_ops = stoi(props[ycsbc::CoreWorkload::OPERATION_COUNT_PROPERTY]);
	utils::Timer<double> timer;
	timer.Start();
	for (int i = 0; i < num_threads; ++i) {
		actual_ops.emplace_back(
				async(launch::async, DelegateClient, db, &wl,
						total_ops / num_threads, false, isKV, clusterFileName));
	}
	assert((int )actual_ops.size() == num_threads);

	sum = 0;
	for (auto &n : actual_ops) {
		assert(n.valid());
		sum += n.get();
	}
	double duration = timer.End();
	cerr << "# Transaction throughput (KTPS)" << endl;
	cerr << props["dbname"] << '\t' << file_name << '\t' << num_threads << '\t';
	cerr << total_ops / duration / 1000 << endl;
}

string ParseCommandLine(int argc, const char *argv[],
		utils::Properties &props) {
	int argindex = 1;
	string filename;
	while (argindex < argc && StrStartWith(argv[argindex], "-")) {
		if (strcmp(argv[argindex], "-threads") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("threadcount", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-db") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("dbname", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-C") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("clusterfilename", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-P") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			filename.assign(argv[argindex]);
			ifstream input(argv[argindex]);
			try {
				props.Load(input);
			} catch (const string &message) {
				cout << message << endl;
				exit(0);
			}
			input.close();
			argindex++;
		} else {
			cout << "Unknown option " << argv[argindex] << endl;
			exit(0);
		}
	}

	if (argindex == 1 || argindex != argc) {
		UsageMessage(argv[0]);
		exit(0);
	}

	return filename;
}

void UsageMessage(const char *command) {
	cout << "Usage: " << command << " [options]" << endl;
	cout << "Options:" << endl;
	cout << "  -threads n: execute using n threads (default: 1)" << endl;
	cout << "  -db dbname: specify the name of the DB to use (default: basic)"
			<< endl;
	cout
			<< "  -P propertyfile: load properties from the given file. Multiple files can"
			<< endl;
	cout
			<< "  -C clusterfile: only used for dtranx db, load ip addresses from the given file."
			<< endl;
	cout
			<< "                   be specified, and will be processed in the order specified"
			<< endl;
}

inline bool StrStartWith(const char *str, const char *pre) {
	return strncmp(str, pre, strlen(pre)) == 0;
}

