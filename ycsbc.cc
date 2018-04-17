/*
 * ycsbc.cc
 * YCSB-C
 *
 * 	Created by Jinglei Ren on 12/19/14.
 * 	Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
 */

#include <cstring>
#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include <future>
#include <chrono>
#include "Util/funcs.h"
#include "Util/Timer.h"
#include "client.h"
#include "Core/core_workload.h"
#include "DB/db_factory.h"
#include "DTranx/Util/Log.h"
#include "DB/commons.h"
#include <gperftools/profiler.h>

using namespace std;
//aggregate throughput during the previous 20 seconds
int ONLINE_THROUGHPUT_TIME = 1000000;

/*
 * after STABLE_AFTER_NUM_TRANX and before STABLE_BEFORE_NUM_TRANX tranx for each thread, ycsb starts
 * to calculate the statistics for the latency
 *
 * STABLE_AFTER_NUM_TRANX is decided by how much time to populate the cache and the difference among clients
 * STABLE_BEFORE_NUM_TRANX is decided by the difference among clients
 *
 * This is implemented for cache effect, it should be dynamically adjusted based on the database
 */
int STABLE_AFTER_NUM_TRANX;
int STABLE_BEFORE_NUM_TRANX;

void UsageMessage(const char *command);
bool StrStartWith(const char *str, const char *pre);
string ParseCommandLine(int argc, const char *argv[], Ycsb::Core::Properties &props);

struct DBData {
	bool isKV;
	std::vector<std::string> ips;
	Ycsb::DB::TableDB *tabledb;
	Ycsb::DB::KVDB *kvdb;
	std::string dbName;
	DBData()
			: isKV(false), ips(), tabledb(NULL), kvdb(NULL) {
	}
};

/*
 * Thread function
 * for DTranx(key value store), DB is independently instantiated among threads while DTranx Clients are shared.
 */
void DelegateClient(DBData *dbData, Ycsb::Core::Properties *props, const int num_ops,
		bool is_loading, std::atomic<int> *sum, std::atomic<int> *sum_succ, double* avgLatency,
		int index) {
	/*
	 * when testing hyperdexDB, create a kvdb instance for each thread
	 * because each thread needs one socket connection to reach the max throughput
	 */
	//ycsbc::KVDB* kvdb = dynamic_cast<ycsbc::KVDB*>(ycsbc::DBFactory::CreateDB("hyperdex"));
	//kvdb->Init(dbData->ips);
	Ycsb::Core::CoreWorkload wl;
	wl.Init(*props);
	if (dbData->isKV) {
		assert(dbData->kvdb != NULL);
	} else {
		assert(dbData->tabledb != NULL);
	}
	if (dbData->isKV) {
		Ycsb::DB::KVDB *kvdb = dbData->kvdb->GetDBInstance(index);
		Ycsb::Client client(dbData->tabledb, kvdb, wl);
		boost::chrono::steady_clock::time_point start = boost::chrono::steady_clock::now();
		boost::chrono::steady_clock::time_point end;
		for (int i = 0; i < num_ops; ++i) {
			if (i == STABLE_AFTER_NUM_TRANX) {
				start = boost::chrono::steady_clock::now();
			}
			if (i == STABLE_BEFORE_NUM_TRANX) {
				end = boost::chrono::steady_clock::now();
			}
			if (is_loading) {
				if (client.DoInsert()) {
					sum_succ->operator ++();
				}
			} else {
				if (client.DoTransaction()) {
					sum_succ->operator ++();
				}
			}
			sum->operator ++();
		}
		uint64_t timeElapsed = boost::chrono::duration_cast<boost::chrono::microseconds>(
				end - start).count();
		*avgLatency = timeElapsed * 1.0 / (STABLE_BEFORE_NUM_TRANX - STABLE_AFTER_NUM_TRANX);
		dbData->kvdb->DestroyDBInstance(kvdb);
		return;
	}
	//kvdb->Close();
	//delete kvdb;
}

int main(const int argc, const char *argv[]) {
	//::ProfilerStart("ycsbc.prof");
	DTranx::Util::Log::setLogPolicy( { { "Client", "PROFILE" }, { "Server", "PROFILE" }, { "Tranx",
			"PROFILE" }, { "Storage", "PROFILE" }, { "RPC", "PROFILE" }, { "Util", "PROFILE" }, {
			"Log", "PROFILE" } });

	Ycsb::Core::Properties props;
	string file_name = ParseCommandLine(argc, argv, props);

	/*
	 * prepare clients, ips etc. for dtranx
	 */
	DBData dbData;
	dbData.dbName = props["dbname"];
	if (props["dbname"] == "dtranx" || props["dbname"] == "hyperdex" || props["dbname"] == "btree"
			|| props["dbname"] == "rtree" || props["dbname"] == "bangdb" || props["dbname"] == "cockroach") {
		dbData.isKV = true;
		std::string clusterFileName = props.GetProperty("clusterfilename", "");
		if (clusterFileName.empty()) {
			cout << "please use -C for cluster filename" << endl;
			return 0;
		}
		std::ifstream ipFile(clusterFileName);
		if (!ipFile.is_open()) {
			cout << "cannot open " << clusterFileName << endl;
			exit(0);
		}
		std::string ip;
		while (std::getline(ipFile, ip)) {
			dbData.ips.push_back(ip);
		}
	}
	Ycsb::Core::CoreWorkload wl;
	wl.Init(props);
	STABLE_AFTER_NUM_TRANX = stoi(props.GetProperty("stablestart", "1"));
	STABLE_BEFORE_NUM_TRANX = stoi(props.GetProperty("stableend", "2"));
	if (dbData.isKV) {
		dbData.kvdb = dynamic_cast<Ycsb::DB::KVDB*>(Ycsb::DB::DBFactory::CreateDB(
				props.GetProperty("dbname"), wl));
		/*
		 * port usage is somewhat static and no need to add configuration file
		 * for ycsb program
		 */
		dbData.kvdb->Init(dbData.ips, props["selfAddress"], LOCAL_USABLE_PORT_START,
				std::stoi(props["firsttime"]) != 0);
	} else {
		dbData.tabledb = dynamic_cast<Ycsb::DB::TableDB*>(Ycsb::DB::DBFactory::CreateDB(
				props.GetProperty("dbname"), wl));
	}

	/*
	 * generate a keys file if needed
	 */
	if (props.GetProperty("genwork", "0") == "1") {
		std::ofstream workFile("genwork");
		if (!workFile.is_open()) {
			cout << "cannot open " << "genwork" << endl;
			exit(0);
		}
		int total_ops = stoi(props[Ycsb::Core::CoreWorkload::RECORD_COUNT_PROPERTY]);
		for (int i = 0; i < total_ops; i++) {
			std::string key = wl.NextTransactionKey();
			workFile << key << endl;
		}
		exit(0);
	}

	const int num_threads = stoi(props.GetProperty("threadcount", "1"));
	/*
	 *  Loads data
	 */
	vector<std::thread> threads;
	int total_ops = stoi(props[Ycsb::Core::CoreWorkload::RECORD_COUNT_PROPERTY]);
	std::atomic<int> sums[num_threads];
	std::atomic<int> sums_succ[num_threads];
	double latencies[num_threads];
	for (int i = 0; i < num_threads; ++i) {
		sums[i].store(0);
	}
	for (int i = 0; i < num_threads; ++i) {
		sums_succ[i].store(0);
	}
	int sum = 0;
	if (total_ops >= num_threads) {
		for (int i = 0; i < num_threads; ++i) {
			threads.push_back(
					std::thread(DelegateClient, &dbData, &props, total_ops / num_threads, true,
							&sums[i], &sums_succ[i], &latencies[i], i));
		}
		sum = 0;
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point end;
		while (sum < total_ops * 0.8) {
			sum = 0;
			for (int i = 0; i < num_threads; ++i) {
				sum += sums[i].load();
			}
			time_t t = time(0);   // get time now
			struct tm * now = localtime(&t);
			cout << now->tm_sec << ": " << sum << endl;
			sleep(1);
			end = std::chrono::system_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
					> ONLINE_THROUGHPUT_TIME) {
				break;
			}

		}
		sum = 0;

		for (int i = 0; i < num_threads; ++i) {
			threads[i].join();
		}
		threads.clear();
	}
	sum = 0;
	for (int i = 0; i < num_threads; ++i) {
		sum += sums_succ[i].load();
	}
	cerr << "# Loading records:\t" << sum << endl;

	for (int i = 0; i < num_threads; ++i) {
		sums[i].store(0);
	}
	for (int i = 0; i < num_threads; ++i) {
		sums_succ[i].store(0);
	}
	for (int i = 0; i < num_threads; ++i) {
		latencies[i] = 0.0;
	}

	/*
	 * Performs transactions
	 */
	total_ops = stoi(props[Ycsb::Core::CoreWorkload::OPERATION_COUNT_PROPERTY]);
	Util::Timer<double> timer;
	timer.Start();
	for (int i = 0; i < num_threads; ++i) {
		threads.push_back(
				std::thread(DelegateClient, &dbData, &props, total_ops / num_threads, false,
						&sums[i], &sums_succ[i], &latencies[i], i));
	}
	sum = 0;
	int prev = 0;
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point end;
	double duration;
	bool iSTimerRestartedAfterStable = false;
	bool iSTimeEndedBeforeInStable = false;
	int numOfTranxBeforeStable;
	int numOfTranxBeforeInstable;
	while (sum < total_ops * 0.98) {
		sum = 0;
		for (int i = 0; i < num_threads; ++i) {
			sum += sums[i].load();
		}
		if (!iSTimerRestartedAfterStable && sum > num_threads * STABLE_AFTER_NUM_TRANX) {
			iSTimerRestartedAfterStable = true;
			numOfTranxBeforeStable = sum;
			timer.Restart();
		}
		if (iSTimerRestartedAfterStable && !iSTimeEndedBeforeInStable
				&& sum > num_threads * STABLE_BEFORE_NUM_TRANX) {
			iSTimeEndedBeforeInStable = true;
			duration = timer.End();
			numOfTranxBeforeInstable = sum;
		}
		time_t t = time(0);   // get time now
		struct tm * now = localtime(&t);
		cout << now->tm_sec << ": " << sum << "---diff: " << (sum - prev) << endl;
		prev = sum;
		sleep(1);
		end = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
				> ONLINE_THROUGHPUT_TIME) {
			break;
		}
	}
	sum = 0;
	for (int i = 0; i < num_threads; ++i) {
		threads[i].join();
	}
	threads.clear();
	total_ops = total_ops / num_threads * num_threads;

	for (int i = 0; i < num_threads; ++i) {
		sum += sums[i].load();
	}

	int sum_succ = 0;
	for (int i = 0; i < num_threads; ++i) {
		sum_succ += sums_succ[i].load();
	}

	double avg_latency = 0.0;
	uint64_t numOfLatencies = 0;
	for (int i = 0; i < num_threads; ++i) {
		avg_latency += latencies[i];
		numOfLatencies++;
	}
	avg_latency = avg_latency / numOfLatencies;
	double succ_rate = 1.0 * sum_succ / total_ops;
	cerr << "# Transaction throughput (KTPS)" << endl;
	cerr << props["dbname"] << '\t' << file_name << '\t' << num_threads << '\t';
	cerr << (numOfTranxBeforeInstable - numOfTranxBeforeStable) * succ_rate / duration / 1000
			<< endl;
	cerr << "total_ops: " << total_ops << ", success: " << sum_succ << ", percentage: "
			<< 1.0 * sum_succ / total_ops << endl;
	cerr << "avg_latency: " << avg_latency << " microseconds" << endl;
	if (dbData.isKV) {
		dbData.kvdb->Close();
	}
	//::ProfilerStop();
}

string ParseCommandLine(int argc, const char *argv[], Ycsb::Core::Properties &props) {
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
		} else if (strcmp(argv[argindex], "-genwork") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("genwork", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-s") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("selfAddress", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-db") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("dbname", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-m") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("mempool", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-C") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("clusterfilename", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-i") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("firsttime", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-s1") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("stablestart", argv[argindex]);
			argindex++;
		} else if (strcmp(argv[argindex], "-s2") == 0) {
			argindex++;
			if (argindex >= argc) {
				UsageMessage(argv[0]);
				exit(0);
			}
			props.SetProperty("stableend", argv[argindex]);
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
	cout << "  -genwork n: if n equals 1, generate keys to a file named genwork.data (default: 0)"
			<< endl;
	cout << "  -threads n: execute using n threads" << endl;
	cout << "  -db dbname: specify the name of the DB to use" << endl;
	cout
			<< "  -m poolcached: whether to enable mempool for ddsbrick data structures (options:0,1, default: 1 meaning enable)"
			<< endl;
	cout
			<< "  -i firsttime: specify if this is the first time to run against the database, for btree/rtree usage"
			<< "0 means no, anything else is yes" << endl;
	cout << "  -P propertyfile: load properties from the given file. Multiple files can" << endl;
	cout << "  -C clusterfile: only used for dtranx db, load ip addresses from the given file."
			<< endl;
	cout << "  -s selfAddress: self address that clients bind to." << endl;
	cout << "                   be specified, and will be processed in the order specified" << endl;
	cout << "  -s1 stablestart" << endl;
	cout << "  -s2 stableend" << endl;
}

inline bool StrStartWith(const char *str, const char *pre) {
	return strncmp(str, pre, strlen(pre)) == 0;
}

