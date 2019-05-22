test: Nain.o Filesystem.o File.o INode.o Superblock.o Address.o
	g++ Nain.o Filesystem.o File.o INode.o Superblock.o Address.o -o test

File.o: File.cpp
	g++ -c File.cpp

INode.o: INode.cpp
	g++ -c INode.cpp

Nain.o: main.cpp
	g++ -c main.cpp -o Nain.o

Filesystem.o: Filesystem.cpp
	g++ -c Filesystem.cpp

Superblock.o: Superblock.cpp
	g++ -c Superblock.cpp

Address.o: Address.cpp
	g++ -c Address.cpp

clean:
	-rm -f *.o