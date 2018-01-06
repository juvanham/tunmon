all: tun_mon

CXX=g++ --std=c++17 -Wall $(CXXFLAGS) -pedantic -g
LIBS=-lboost_program_options -lboost_system -lboost_filesystem -lpthread

net_dev.o: net_dev.cpp net_dev.hpp
	$(CXX) -c net_dev.cpp

config.o: config.cpp config.hpp
	$(CXX) -c config.cpp

driver.o: driver.cpp driver.hpp
	$(CXX) -c driver.cpp

pidfile.o: pidfile.cpp pidfile.hpp
	$(CXX) -c pidfile.cpp

main.o: main.cpp net_dev.hpp config.hpp driver.hpp pidfile.hpp
	$(CXX) -c main.cpp

tun_mon: main.o net_dev.o config.o driver.o pidfile.o
	g++ $(LIBS) -o tun_mon main.o net_dev.o config.o driver.o pidfile.o

.PHONY: clean

clean:
	rm -f tun_mon main.o net_dev.o config.o driver.o pidfile.o
	rm -f tunmon.pid *~
