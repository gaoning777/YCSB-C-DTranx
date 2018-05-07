YCSB-C
======

Yahoo! Cloud Serving Benchmark in C++, a C++ version of YCSB (https://github.com/brianfrankcooper/YCSB/wiki)


pre-requisite: libtbb-dev
Dynamically linked libtbb-dev

For hyperdex support, check out the website for installation http://hyperdex.org
	Clients: install libhyperdex-client-dev-warp, python-hyperdex-client-warp
	Server: install hyperdex-warp
	
	To start hyperdex servers, follow the steps here
	1. start coordinator at server1
		hyperdex coordinator -f -l 192.168.0.1 -p 7777 &> hyperdex.coord &
	2. start all daemons in all servers
		hyperdex daemon -f --listen=192.168.0.1 --listen-port=7776 --coordinator=192.168.0.1 --coordinator-port=7777 --data=./hyperdex.db &> hyperdex.daemon &
	3. initialize database
		3.1 Add space "ning" using python in coordinator
				import hyperdex.admin
				a = hyperdex.admin.Admin('192.168.0.1', 7777)
				a.add_space('''
  					space ning
  					key keystr
  					attributes value
  					''')
		3.2 create hashkey file by running HashGenerator.cc program
				./a.out > hashkeys
		3.3 Run InitializeHyperdex.py and feed it with hashkey file
				python InitializeHyperdex.py hashkeys


Build  
1. Build the project  
	make  
2. Build unit tests  
	make unit  
3. Run unit tests  
	./Test/UnitTest/test  

Maintained by Ning Gao
University of Colorado Boulder
