/*
 * Author: Ning Gao(nigo9731@colorado.edu)
 *         Zhang Liu(zhli5318@colorado.edu)
 *
 * Cockroach DB client Handler using libpqxx
 *
 *
 * NOTE: server IP and port will be loaded from bangdb.config during runtime
 *
 * NOTE:
 *
 * 
 */

#ifndef DTRANX_COCKROACH_DB_H_
#define DTRANX_COCKROACH_DB_H_

#include <pqxx/pqxx>
//#include "kvdb.h"
#include "DB/commons.h"
namespace Ycsb {
namespace DB {


class CockroachDB: public KVDB {
public:
    CockroachDB(){
        shareDB = false;
        keyType = KeyType::STRING;
    }

    CockroachDB(const CockroachDB& other) {
        std::cout << "CockroachDB copy contructor is called" << std::endl;
        shareDB = other.shareDB;
        keyType = other.keyType;
    }

    ~CockroachDB() {
        Close();
    }

    KVDB* Clone(int index) {
        CockroachDB *instance = new CockroachDB(*this);
        std::cout << "Cloning CockroachDB called" << std::endl;
	//current connectiion is hard coded
	instance->client_ = new pqxx::connection("postgresql://ycsb@192.168.0.1:26257/ycsb");

        return instance;
    }

    void Init(std::vector<std::string> ips, std::string selfAddress, int localStartPort,
            bool fristTime) {
	//current connectiion is hard coded
	client_ = new pqxx::connection("postgresql://ycsb@192.168.0.1:26257/ycsb");
    }

    void Close() {
	if (client_) {
            client_->disconnect();
            delete client_;
        }
    }


    int Read(std::vector<std::string> keys) {
		pqxx::work tx(*client_);
		pqxx::result r;
        for (auto it = keys.begin(); it != keys.end(); ++it) {
			r = tx.exec("SELECT value FROM kv WHERE key = b'" + const_cast<std::string&>(*it) + "'");
     			
			if(r[0][0].is_null()) {
        	    tx.abort();
        		printf("individual get transaction failed ");
        	    return kErrorNoData;
        	}
        }

		try{
			tx.commit();
		} catch (const std::exception &e){
			return kErrorConflict;
		}

		return kOK;
    }

    int ReadSnapshot(std::vector<std::string> keys) {
        /*
         */
        return kOK;
    }

    int Update(std::vector<KVPair> writes) {
		pqxx::work tx(*client_);
		pqxx::result r;

		for (auto it = writes.begin(); it != writes.end(); ++it) {
    		tx.exec("UPDATE kv SET value = b'" + it->second + "' WHERE key = b'" + it->first + "'");
//            cout << "update write key: " << it->first << " and the value is "
//                    << it->second << endl;
		}

        try{
            tx.commit();
        } catch (const std::exception &e){
            return kErrorConflict;
        }

        return kOK;

    }

    int ReadWrite(std::vector<std::string> reads, std::vector<KVPair> writes) {
        pqxx::work tx(*client_);
        pqxx::result r;


    	for (auto it = reads.begin(); it != reads.end(); ++it) {
            r = tx.exec("SELECT value FROM kv WHERE key = b'" + const_cast<std::string&>(*it) + "'");

            if(r[0][0].is_null()) {
                tx.abort();
                printf("individual get transaction failed ");
                return kErrorNoData;
            }
		}
        for (auto it = writes.begin(); it != writes.end(); ++it) {
            tx.exec("UPDATE kv SET value = b'" + it->second + "' WHERE key = b'" + it->first + "'");
//            cout << "update write key: " << it->first << " and the value is "
//                    << it->second << endl;
        }
        
		try{
            tx.commit();
        } catch (const std::exception &e){
            return kErrorConflict;
        }

        return kOK;

    }

    int Insert(std::vector<KVPair> writes) {
        pqxx::work tx(*client_);
        pqxx::result r;
        for (auto it = writes.begin(); it != writes.end(); ++it) {
			tx.exec("INSERT INTO kv VALUES ( b'" + it->first + "', b'" + it->second + "')");    
        }

        try{
            tx.commit();
        } catch (const std::exception &e){
				std::cout<<"insert commit error"<<std::endl;
			return kErrorConflict;
        }

        return kOK;
    }

private:
    pqxx::connection* client_;
};
} // DB
} // Ycsb

#endif /* DTRANX_COCKROACH_DB_H_ */

