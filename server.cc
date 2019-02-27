#include <iostream>
#include <boost/asio.hpp>
#include <array>
#include <fstream>
#include <string.h>
#include "thread_pool.hpp"
#include "hashlist.h"

using boost::asio::ip::tcp;
using namespace std;

//server run hashtable; thread work on this hashtable
pHash_List plist = init_hash_list();  
int get_succ=0, get_fail=0, put_succ=0, put_fail=0;

class TCPConnection : public std::enable_shared_from_this<TCPConnection> 
{
public:
	TCPConnection(boost::asio::io_service &io_service)
	: socket_(io_service),
	strand_(io_service)
	{        
	}

	tcp::socket &socket()
	{
		return socket_;
	}

	void start()
	{
		doRead();
	}

private:
	void doRead()
	{
		auto self = shared_from_this();
		socket_.async_read_some(boost::asio::buffer(buffer_, buffer_.size()),
			strand_.wrap([this, self](boost::system::error_code ec,
				std::size_t bytes_transferred)
			{
				if (!ec)
				{
					bytes_transferred = handle_data(bytes_transferred);
					doWrite(bytes_transferred);
				}
			}));        
	}

	void doWrite(size_t length)
	{
		auto self = shared_from_this();        
		boost::asio::async_write(socket_, boost::asio::buffer(buffer_, length),
			strand_.wrap([this, self](boost::system::error_code ec,
                                                           std::size_t /* bytes_transferred */)
			{

				if (!ec)
				{

					doRead();                                         
				}
			}));
	}

	size_t handle_data(size_t length){
		string command, response, data;
		u32 position;
		stringstream stream;
		for (int i = 0; i < length; ++i)
		{
			stream << buffer_[i];
		}
		data = stream.str();
    // ********** de-protocol ***********
		position = data.find("\n");
		if (position <= data.length())
		{
			command = data.substr(0, position);
			data = data.substr(position+1);
			if (command == "PUT")
			{

				response = insert_node_to_hash(plist, data);
				// cout << response << endl;
				if (response == "OK")
				{
					put_succ++;
				}
				else if (response == "ERROR")
				{
					put_fail++;
				}

			}
			else if (command == "GET")
			{
				// sleep(10);
				response = get_node_to_hash(plist, data);
				if (response == "ERROR")
				{
					get_fail++;
				}
				else
				{
					get_succ++;
				}
			}
			else if (command == "DEL")
			{
				response = delete_node_to_hash(plist, data);
			}

      // ----------------------- JUST FOR TEST -------------
			else if (command == "SHOW")
			{
				print_hash(plist);
				response = "SHOWN IN SERVER";
			}
			else if (command == "INIT")
			{
				init_hash(plist);
				response = "DONE"; 
			}
      // ----------------------- JUST FOR TEST -------------

			data = response;  
		}
		data = std::to_string(data.length()) + "\n" + data;

		for (int i = 0; i < data.length(); ++i)
		{
			buffer_[i] = data[i];
		}
		length = data.length();


		cout << "PUT_SUCC = " << put_succ << "\tPUT_FAIL = " << put_fail << "\tGET_SUCC = " << get_succ << "\tGET_FAIL = " << get_fail << endl;

		return length;
	}

private:
	tcp::socket socket_;
	boost::asio::io_service::strand strand_;
	std::array<char, 8192> buffer_;
};

class DHTServer
{
public:
	DHTServer(boost::asio::io_service &io_service, unsigned short port)
	: io_service_(io_service),
	acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		doAccept();
	}

	void doAccept()
	{
		auto conn = std::make_shared<TCPConnection>(io_service_);
		acceptor_.async_accept(conn->socket(),
			[this, conn](boost::system::error_code ec)
			{
				if (!ec)
				{
					conn->start();
				}                                   
				this->doAccept();
			});    
	}

private: 
	boost::asio::io_service &io_service_;
	tcp::acceptor acceptor_;
};

int main(int argc, char *argv[])
{
	using namespace std;

	//Get port as size_t from argv
	size_t port = 40300, num_thread = 3;

	ifstream in("DHTConfig");
	if (in.is_open())
	{
		for (string str; getline(in, str) ;)
		{	
			if (str.find("PORT")!=str.npos)
			{
				port = stoi(str.substr(str.find("=")+1));
			}
			if (str.find("THREAD")!=str.npos)
			{
				num_thread = stoi(str.substr(str.find("=")+1));
			}
			
		}
		in.close();
	}

	AsioThreadPool pool(num_thread);

	DHTServer server(pool.getIOService(), port);

	pool.stop();

	return 0;
}