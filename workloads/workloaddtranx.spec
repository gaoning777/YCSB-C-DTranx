# Yahoo! Cloud System Benchmark
# Workload A: Update heavy workload
#   Application example: Session store recording recent actions
#                        
#   Read/update ratio: 50/50
#   Default data size: 1 KB records (10 fields, 100 bytes each, plus key)
#   Request distribution: zipfian
#
#	adapted by Ning Gao to work on DTranx project
#
#

# Transaction Count
# 	recordcount: number of insert transactions to execute before tests
# 	operationcount: number of testing transactions to run
# 	workload: not used
# 	readallfields: whether to read all fields for tabledb
recordcount=0
operationcount=800000
workload=com.yahoo.ycsb.workloads.CoreWorkload
readallfields=true

#
# Transaction types
#	readproportion: pure reads
#		number of keys are decided by a uniform distribution generator(1, keymaxnumber)
#		configured by keymaxnumber, default 3
#	updateproportion: blind update one item
#	scanproportion: not used for kvdb
#	insertproportion: insert items
#		number of keys are decided by a uniform distribution generator(1, keymaxnumber)
#	readmodifywriteproportion: read and then update
#		number of keys are decided by a uniform distribution generator(1, keymaxnumber)
#		number of write items is always 1, the rest is the read items
#	snapshot: snapshot reads
#		when snapshot is enabled, every transaction is snapshot read, all the above transaction
#		types is not applying
#
readproportion=1
updateproportion=0
scanproportion=0
insertproportion=0
readmodifywriteproportion=0
snapshot=1

# Key chooser
#	keymaxvalue: max key value
#		if the requestdistribution is uniform, key values are chosen between (0, keymaxvalue-1)
# 	requestdistribution: decides how keys are generated, options are uniform, zipfian, normal, and counter
#		default: uniform distribution
#	insertorder: hashed/nohash
#		it's used in key build, after key is choosen from requestdistribution, the uint64_t
#		value will be mapped to a string.
#		if hashed, the mapping is key->"user"+hash(key)
#		else, the mapping is key->"user"+key
#
keymaxvalue=10000000
requestdistribution=uniform
insertorder=hashed