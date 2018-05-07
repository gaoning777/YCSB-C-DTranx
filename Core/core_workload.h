//
//  core_workload.h
//  YCSB-C
//
//  Created by Jinglei Ren on 12/9/14.
//  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
//

/*
 * Create workloads for each client thread because the
 * UniformGenerator is not thread safe.
 */

#ifndef YCSB_C_CORE_WORKLOAD_H_
#define YCSB_C_CORE_WORKLOAD_H_

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <mutex>
#include "DB/tabledb/table_db.h"
#include "properties.h"
#include "Generator/generator.h"
#include "Generator/discrete_generator.h"
#include "Generator/counter_generator.h"
#include "Util/funcs.h"

namespace Ycsb {
namespace Core {

enum Operation {
	INSERT, READ, UPDATE, SCAN, READMODIFYWRITE
};

class CoreWorkload {
public:
	///
	/// The name of the database table to run queries against.
	///
	static const std::string TABLENAME_PROPERTY;
	static const std::string TABLENAME_DEFAULT;

	///
	/// The name of the property for the number of fields in a record.
	///
	static const std::string FIELD_COUNT_PROPERTY;
	static const std::string FIELD_COUNT_DEFAULT;

	///
	/// The name of the property for the field length distribution.
	/// Options are "uniform", "zipfian" (favoring short records), and "constant".
	///
	static const std::string FIELD_LENGTH_DISTRIBUTION_PROPERTY;
	static const std::string FIELD_LENGTH_DISTRIBUTION_DEFAULT;

	///
	/// The name of the property for the length of a field in bytes.
	///
	static const std::string FIELD_LENGTH_PROPERTY;
	static const std::string FIELD_LENGTH_DEFAULT;

	///
	/// The name of the property for deciding whether to read one field (false)
	/// or all fields (true) of a record.
	///
	static const std::string READ_ALL_FIELDS_PROPERTY;
	static const std::string READ_ALL_FIELDS_DEFAULT;

	///
	/// The name of the property for deciding whether to write one field (false)
	/// or all fields (true) of a record.
	///
	static const std::string WRITE_ALL_FIELDS_PROPERTY;
	static const std::string WRITE_ALL_FIELDS_DEFAULT;

	///
	/// The name of the property for the proportion of read transactions.
	///
	static const std::string READ_PROPORTION_PROPERTY;
	static const std::string READ_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of update transactions.
	///
	static const std::string UPDATE_PROPORTION_PROPERTY;
	static const std::string UPDATE_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of insert transactions.
	///
	static const std::string INSERT_PROPORTION_PROPERTY;
	static const std::string INSERT_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of scan transactions.
	///
	static const std::string SCAN_PROPORTION_PROPERTY;
	static const std::string SCAN_PROPORTION_DEFAULT;

	///
	/// The name of the property for the proportion of
	/// read-modify-write transactions.
	///
	static const std::string READMODIFYWRITE_PROPORTION_PROPERTY;
	static const std::string READMODIFYWRITE_PROPORTION_DEFAULT;

	///
	/// The name of the property for the the distribution of request keys.
	/// Options are "uniform", "zipfian" and "latest".
	///
	static const std::string REQUEST_DISTRIBUTION_PROPERTY;
	static const std::string REQUEST_DISTRIBUTION_DEFAULT;

	///
	/// The name of the property for the max scan length (number of records).
	///
	static const std::string MAX_SCAN_LENGTH_PROPERTY;
	static const std::string MAX_SCAN_LENGTH_DEFAULT;

	///
	/// The name of the property for the scan length distribution.
	/// Options are "uniform" and "zipfian" (favoring short scans).
	///
	static const std::string SCAN_LENGTH_DISTRIBUTION_PROPERTY;
	static const std::string SCAN_LENGTH_DISTRIBUTION_DEFAULT;

	///
	/// The name of the property for the order to insert records.
	/// Options are "ordered" or "hashed".
	///
	static const std::string INSERT_ORDER_PROPERTY;
	static const std::string INSERT_ORDER_DEFAULT;

	static const std::string INSERT_START_PROPERTY;
	static const std::string INSERT_START_DEFAULT;

	static const std::string RECORD_COUNT_PROPERTY;
	static const std::string OPERATION_COUNT_PROPERTY;

	///
	/// The name of the property for max number of keys in one transaction
	/// and max key value
	///
	static const std::string MAX_KEY_NUMBER_PROPERTY;
	static const std::string MAX_KEY_NUMBER_DEFAULT;
	static const std::string MAX_KEY_VALUE_PROPERTY;
	static const std::string MAX_KEY_VALUE_DEFAULT;

	///
	/// the name of the property whether transactions are snapshot or normal ones
	///
	static const std::string IS_SNAPSHOT_PROPERTY;
	static const std::string IS_SNAPSHOT_DEFAULT;

	///
	/// the name of the property whether key is from a random generator("none") or a file
	///
	static const std::string KEY_GENERATOR_PROPERTY;
	static const std::string KEY_GENERATOR_DEFAULT;

	///
	/// the name of the property whether mempool is enabled
	///
	static const std::string MEMPOOL_PROPERTY;
	static const std::string MEMPOOL_DEFAULT;

	///
	/// Initialize the scenario.
	/// Called once, in the main client thread, before any operations are started.
	///
	virtual void Init(const Properties &p);

	virtual void BuildValues(std::vector<DB::DB_BASE::KVPair> &values);
	virtual void BuildUpdate(std::vector<DB::DB_BASE::KVPair> &update);

	virtual std::string NextTable() {
		return table_name_;
	}
	virtual std::string NextSequenceKey();
	virtual std::string NextTransactionKey();
	virtual uint64_t NextSequenceKeyInt();
	virtual uint64_t NextTransactionKeyInt();
	virtual std::vector<std::string> NextTransactionKeys();
	virtual std::vector<uint64_t> NextTransactionKeysInt();
	virtual std::vector<DB::DB_BASE::KVPair> NextTransactionKVs();
	virtual std::vector<DB::DB_BASE::KVPairInt> NextTransactionKVsInt();
	virtual Operation NextOperation() {
		return op_chooser_.Next();
	}
	virtual std::string NextFieldName();
	virtual size_t NextScanLength() {
		return scan_len_chooser_->Next();
	}
	size_t GetMaxKeyValue();
	virtual size_t GetMaxKeyCount();
	virtual bool IsSnapshot() {
		return isSnapshot;
	}

	bool read_all_fields() const {
		return read_all_fields_;
	}
	bool write_all_fields() const {
		return write_all_fields_;
	}

	bool UseMempoolCache() {
		return useMempoolCache;
	}

	CoreWorkload()
			: field_count_(0), read_all_fields_(false), write_all_fields_(false), field_len_generator_(
			NULL), key_generator_(NULL), key_chooser_(NULL), keynum_chooser_(
			NULL), field_chooser_(NULL), scan_len_chooser_(NULL), insert_key_sequence_(3), ordered_inserts_(
					true), record_count_(0), max_key_count_(3), max_key_value_(100) {
	}

	virtual ~CoreWorkload() {
		if (field_len_generator_)
			delete field_len_generator_;
		if (key_generator_)
			delete key_generator_;
		if (key_chooser_)
			delete key_chooser_;
		if (field_chooser_)
			delete field_chooser_;
		if (scan_len_chooser_)
			delete scan_len_chooser_;
		if (keynum_chooser_)
			delete keynum_chooser_;
		if (key_generator != KEY_GENERATOR_DEFAULT) {
			keyFile.close();
		}
	}

protected:
	static Generator<uint64_t> *GetFieldLenGenerator(const Properties &p);
	std::string BuildKeyName(uint64_t key_num);

	std::string table_name_;
	int field_count_;

	bool read_all_fields_;

	bool write_all_fields_;
	Generator<uint64_t> *field_len_generator_;
	Generator<uint64_t> *key_generator_;
	DiscreteGenerator<Operation> op_chooser_;
	Generator<uint64_t> *key_chooser_;
	Generator<uint64_t> *keynum_chooser_;
	Generator<uint64_t> *field_chooser_;
	Generator<uint64_t> *scan_len_chooser_;
	CounterGenerator insert_key_sequence_;

	bool ordered_inserts_;
	size_t record_count_;
	size_t max_key_count_;
	size_t max_key_value_;

	bool isSnapshot;
	std::string key_generator;
	std::ifstream keyFile;

	bool useMempoolCache;
};

inline size_t CoreWorkload::GetMaxKeyValue() {
	return max_key_value_;
}

inline size_t CoreWorkload::GetMaxKeyCount() {
	return max_key_count_;
}

inline std::string CoreWorkload::NextSequenceKey() {
	uint64_t key_num = key_generator_->Next();
	return BuildKeyName(key_num);
}

inline uint64_t CoreWorkload::NextSequenceKeyInt() {
	return key_generator_->Next();
}

inline std::string CoreWorkload::NextTransactionKey() {
	if (key_generator != KEY_GENERATOR_DEFAULT) {
		std::string key;
		std::getline(keyFile, key);
		return key;
	}
	uint64_t key_num;
	do {
		key_num = key_chooser_->Next();
	} while (key_num > insert_key_sequence_.Last());
	return BuildKeyName(key_num);
}

inline uint64_t CoreWorkload::NextTransactionKeyInt() {
	assert(key_generator == KEY_GENERATOR_DEFAULT);
	uint64_t key_num;
	do {
		key_num = key_chooser_->Next();
	} while (key_num > insert_key_sequence_.Last());
	return key_num;
}

inline std::string CoreWorkload::BuildKeyName(uint64_t key_num) {
	if (!ordered_inserts_) {
		key_num = Util::Hash(key_num);
	}
	return std::string("user").append(std::to_string(key_num));
}

inline std::string CoreWorkload::NextFieldName() {
	return std::string("field").append(std::to_string(field_chooser_->Next()));
}

inline std::vector<std::string> CoreWorkload::NextTransactionKeys() {
	std::vector<std::string> keys;
	size_t num = keynum_chooser_->Next();
	for (size_t i = 1; i <= num; ++i) {
		keys.push_back(NextTransactionKey());
	}
	return keys;
}

inline std::vector<uint64_t> CoreWorkload::NextTransactionKeysInt() {
	std::vector<uint64_t> keys;
	size_t num = keynum_chooser_->Next();
	for (size_t i = 1; i <= num; ++i) {
		keys.push_back(NextTransactionKeyInt());
	}
	return keys;
}

inline std::vector<DB::DB_BASE::KVPair> CoreWorkload::NextTransactionKVs() {
	std::vector<DB::DB_BASE::KVPair> result;
	size_t num = keynum_chooser_->Next();
	for (size_t i = 1; i <= num; ++i) {
		result.push_back(
				DB::DB_BASE::KVPair(NextTransactionKey(),
						std::string(field_len_generator_->Next(), Util::RandomPrintChar())));
	}
	return result;
}

inline std::vector<DB::DB_BASE::KVPairInt> CoreWorkload::NextTransactionKVsInt() {
	std::vector<DB::DB_BASE::KVPairInt> result;
	size_t num = keynum_chooser_->Next();
	for (size_t i = 1; i <= num; ++i) {
		result.push_back(
				DB::DB_BASE::KVPairInt(NextTransactionKeyInt(),
						std::string(field_len_generator_->Next(), Util::RandomPrintChar())));
	}
	return result;
}

} // Core
} // Ycsb

#endif // YCSB_C_CORE_WORKLOAD_H_
