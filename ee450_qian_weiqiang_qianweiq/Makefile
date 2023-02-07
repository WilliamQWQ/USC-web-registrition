all: serverM serverC serverEE serverCS client

serverM: serverM.cpp
	g++ serverM.cpp -std=c++11 -o serverM

serverC: serverC.cpp
	g++ serverC.cpp -std=c++11 -o serverC

serverEE: serverEE.cpp
	g++ serverEE.cpp -std=c++11 -o serverEE

serverCS: serverCS.cpp
	g++ serverCS.cpp -std=c++11 -o serverCS

client: client.cpp
	g++ client.cpp -std=c++11 -o client

clean:
	rm -f serverM serverC serverEE serverCS client