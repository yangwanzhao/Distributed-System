#include<iostream>
// #include<stdlib.h>
#include<boost/asio.hpp>
#include<boost/thread.hpp>

using namespace std;
using namespace boost::asio;

void doing(boost::shared_ptr<ip::tcp::socket> sock);
int main(){
 
	typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
	io_service service;
	ip::tcp::endpoint ep(ip::tcp::v4(), 2001); // listen on 2001
	ip::tcp::acceptor acc(service, ep);
	while (true) 
	{
		socket_ptr sock(new ip::tcp::socket(service));
		acc.accept(*sock);
		boost::function0<void> f = boost::bind(doing,sock);
		boost::thread t(f);
		
	}
 
	system("pause");
	return 1;
}
void doing(boost::shared_ptr<ip::tcp::socket> sock)
{
	while (true) {
		try{
			char data[512];
			
			size_t len = sock->read_some(buffer(data));
			if (len > 0)
				write(*sock, buffer(data, len));
		}
		catch (boost::system::system_error e)
		{
			cout << e.code() << endl;
			return;
		}
		
	}
}

