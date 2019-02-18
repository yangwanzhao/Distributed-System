g++ -g server.cc hashlist.cc\
	-std=c++17 \
	-lboost_system \
	-lboost_filesystem \
	-lboost_regex \
	-lboost_serialization \
	-lpthread \
	-lstdc++ \
	-o server
	


g++ -g client.cc \
	-std=c++17 \
	-lboost_system \
	-lboost_filesystem \
	-lboost_regex \
	-lboost_serialization \
	-lpthread \
	-lstdc++ \
	-o client