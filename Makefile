all: tun_mon

CXX=g++ --std=c++17 -Wall $(CXXFLAGS) -pedantic -g
LIBS=-lboost_program_options -lboost_system -lboost_filesystem -lpthread
TESTLIBS=-lboost_unit_test_framework

net_dev.o: net_dev.cpp net_dev.hpp
	$(CXX) -c net_dev.cpp

config.o: config.cpp config.hpp
	$(CXX) -c config.cpp

driver.o: driver.cpp driver.hpp
	$(CXX) -c driver.cpp

pidfile.o: pidfile.cpp pidfile.hpp
	$(CXX) -c pidfile.cpp

actor.o: actor.cpp actor.hpp
	$(CXX) -c actor.cpp

main.o: main.cpp net_dev.hpp config.hpp driver.hpp pidfile.hpp actor.hpp
	$(CXX) -c main.cpp


test_net_dev.o: test_net_dev.cpp net_dev.o
	$(CXX) -c test_net_dev.cpp

test_config.o: test_config.cpp config.o
	$(CXX) -c test_config.cpp


test_net_dev: test_net_dev.o net_dev.o
	g++ $(LIBS) $(TESTLIBS) -o test_net_dev test_net_dev.o net_dev.o 	

test_config: test_config.o config.o
	g++ $(LIBS) $(TESTLIBS) -o test_config test_config.o config.o 	


tun_mon: main.o net_dev.o config.o driver.o pidfile.o actor.o
	g++ $(LIBS) -o tun_mon main.o net_dev.o config.o driver.o pidfile.o actor.o

unittests: test_net_dev test_config
	./test_net_dev && ./test_config


.PHONY: clean

clean:
	rm -f tun_mon main.o net_dev.o config.o driver.o pidfile.o test_*.o
	rm -f tunmon.pid *~
