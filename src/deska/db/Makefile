CPPFLAGS += -I/opt/PostgreSQL/8.4/include
CPP = g++
LINKER = g++
LINKERFLAGS = -L/opt/PostgreSQL/8.4/lib -lpq

all:	run

run:	main.o Database.o
	$(LINKER) $(LINKERFLAGS) main.o Database.o
 	
main.o:	main.cpp
	$(CPP) $(CPPFLAGS) -c main.cpp

Database.o:	Database.cpp Database.h
	$(CPP) $(CPPFLAGS) -c Database.cpp