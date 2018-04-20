/*
 * Author: Ning Gao(nigo9731@colorado.edu)
 *
 * KVDB is key value databases, it can be shared among YCSB client threads or Cloned.
 *
 * 1. Sharing is handled by this class and users only call GetDBInstance/DestroyDBInstance to get an instance
 * 2. KVDB is able to feed different key types, e.g. btree db asks for plain integer values
 *
 * Before GetDBInstance, call Init
 * After DestroyDBInstance, call Close
 */

#ifndef YCSB_C_KVDB_H_
#define YCSB_C_KVDB_H_

#include <vector>
#include <string>
#include "DB/db_base.h"

namespace Ycsb {
namespace DB {

class KVDB: public DB_BASE {
public:
	enum KeyType {
		STRING, INTEGER, CUSTOMIZE
	};
	/*
	 * Init function initializes client connections
	 * 	@ips: a list of remote server ips
	 * 	@selfAddress: self ip address to bind to
	 * 	@remotePorts: port number the remote server is using
	 * 		assuming all remote server share the same configuration.
	 * 	@localStartPort: start port number that ycsb can bind to, it
	 * 		might use several ports
	 * 	@firstTime means if the database is empty or not.
	 */
	virtual void Init(std::vector<std::string> ips, std::string selfAddress, int localStartPort,
			bool fristTime = false) = 0;

	void InitCockroach(std::string db_user, std::string db_name){
		db_user_ = db_user;
		db_name_ = db_name;
	}
	virtual void Close() = 0;
	/*
	 * Reads a record from the database.
	 *
	 * @param keys are a list of read keys
	 * @return Zero on success, or a non-zero error code on error/record-miss.
	 */
	virtual int Read(std::vector<std::string> keys) {
		return kOK;
	}
	virtual int Read(std::vector<uint64_t> keys) {
		return kOK;
	}
	virtual int Read() {
		return kOK;
	}

	virtual int ReadSnapshot(std::vector<std::string> keys) {
		return kOK;
	}

	virtual int ReadSnapshot(std::vector<uint64_t> keys) {
		return kOK;
	}

	virtual int ReadSnapshot() {
		return kOK;
	}

	/*
	 * 	ReadWrite a record in the database.
	 * Field/value pairs in the specified vector are written to the record,
	 *
	 * @param reads, readset
	 * @param writes, writeset
	 * @return Zero on success, a non-zero error code on error.
	 */
	virtual int ReadWrite(std::vector<std::string> reads, std::vector<KVPair> writes) {
		return kOK;
	}

	virtual int ReadWrite(std::vector<uint64_t> reads, std::vector<KVPairInt> writes) {
		return kOK;
	}

	virtual int ReadWrite() {
		return kOK;
	}

	/*
	 *  Inserts a record into the database.
	 *  Field/value pairs in the specified vector are written into the record.
	 *
	 *  @param writes, a list of key value pair to insert.
	 *  @return Zero on success, a non-zero error code on error.
	 */
	virtual int Insert(std::vector<KVPair> writes) {
		return kOK;
	}

	virtual int Insert(std::vector<KVPairInt> writes) {
		return kOK;
	}

	virtual int Insert() {
		return kOK;
	}

	/*
	 * Update is used as Remove a record for BTree and RTree
	 */

	virtual int Update(std::vector<KVPair> writes) {
		return kOK;
	}

	virtual int Update(std::vector<KVPairInt> writes) {
		return kOK;
	}

	virtual int Update() {
		return kOK;
	}

	KVDB() {

	}

	KVDB(const KVDB &other) {
		keyType = other.keyType;
		shareDB = other.shareDB;
	}

	virtual ~KVDB() {
	}

	bool isDBShared() {
		return shareDB;
	}

	virtual KVDB *Clone(int index) = 0;

	virtual KVDB *GetDBInstance(int index) {
		if (shareDB) {
			return this;
		} else {
			return Clone(index);
		}
	}
	virtual void DestroyDBInstance(KVDB* kvdb) {
		if (shareDB) {
			return;
		} else {
			delete kvdb;
		}
	}

	KeyType GetKeyType() const {
		return keyType;
	}

	void SetKeyType(KeyType keyType) {
		this->keyType = keyType;
	}

protected:
	/*
	 * shareDB means if we want to share DB instance among threads
	 * isKeyTypeString means whether the key is string type or integer or customized like RTree
	 */
	bool shareDB;
	KeyType keyType;


	std::string db_user_;
	std::string db_name_;

};
} // DB
} // Ycsb

#endif // YCSB_C_KVDB_H_

