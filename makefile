# Authors: Ty-Rack-US Park
# Date: April 13, 2015
#
# Description: Make for spreadsheet server

all: server.o spreadsheet.o dependency.o
	g++ SpreadsheetServer.o Spreadsheet.o DependencyGraph.o /usr/local/lib/libboost_system.a -lpthread

server.o: SpreadsheetServer.cpp SpreadsheetServer.h 
	g++ -c SpreadsheetServer.cpp -std=gnu++0x

spreadsheet.o: Spreadsheet.cpp Spreadsheet.h 
	g++ -c Spreadsheet.cpp -std=gnu++0x

dependency.o: DependencyGraph.cpp DependencyGraph.h 
	g++ -c DependencyGraph.cpp

run:
	make all
	./a.out 2118

clean:
	rm -f ./a.out *.o