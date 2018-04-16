CC=g++-4.9
CFLAGS=-std=c++11 -rdynamic -Wl,--no-as-needed -pthread -g -Wall -pthread -I./
LDFLAGS= -ltbb -ldtranx -llogcabin -lpthread -lprotobuf -lzmq -lhyperdex-client\
 -lboost_thread -lboost_system -lboost_chrono -lbtree -lrtree -lprofiler -lpmemlog -lleveldb -lcryptopp -lmetis\
 -lbangdb-client -lpq -lpqxx
SUBDIRS=Core DB Scripts Util
SUBSRCS=$(wildcard Core/*.cc) $(wildcard DB/*.cc) $(wildcard DB/tabledb/*.cc) $(wildcard Util/*.cc)
SUBHEADERS=$(wildcard Core/*.h) $(wildcard DB/*.h) $(wildcard Util/*.h)
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	$(MAKE) -C $@

$(EXEC): $(EXEC).cc $(OBJECTS) $(SUBHEADERS)
	$(CC) $(CFLAGS) $(EXEC).cc $(OBJECTS) $(LDFLAGS) -o $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)

.PHONY: $(SUBDIRS)
