#include <iostream>
#include <fstream>
#include <libgen.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
#include <boost/asio.hpp>
#include <time.h>
#include <vector>
#define BUF_SIZE 8192

using namespace std;
using namespace boost::asio;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

duration<double> latency;

class DHTClient 
{
public:
  // DHTClient():socket_(create_socket("localhost", "40300")){socket_.close();}
  DHTClient(string port, string server):port_(port),server_(server),socket_(create_socket(server, port)){}

  boost::asio::ip::tcp::socket create_socket(std::string hostname, std::string port) {
    using namespace boost::asio;

    io_service io_service;
    ip::tcp::resolver resolver(io_service);
    ip::tcp::resolver::query query(hostname, port);
    ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    ip::tcp::socket socket(io_service);
    try{
      connect(socket, endpoint_iterator);
    }catch (std::exception &e){
      // std::cerr << e.what() << std::endl;
      cout << hostname << " is offline" << endl;
      exit(1);
    }
    return socket;
  }

  void write_read_buffer(boost::asio::ip::tcp::socket &socket, string data){
    using namespace std;
    using namespace boost::asio;

    // track connection duration, bytes transmitted
    size_t xmitBytes = 0;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, nullptr);

    //ready to recieve data from server
    size_t recd = 0, position;
    unsigned char buf[BUF_SIZE];
    // vector<unsigned char> buf;
    size_t len;
    stringstream stream;
    string str_buf;
    boost::system::error_code ignored_error;
    write(socket, buffer(data), ignored_error);
    xmitBytes += data.length();

    str_buf = "";
    while (true)
    {
      len = socket.read_some(boost::asio::buffer(buf), ignored_error);

      for (int i = 0; i < len; ++i)
      {
        stream << buf[i];
      }

      str_buf = stream.str();
      position = str_buf.find("\n");
      if ( position < str_buf.length() )
      {
        recd = stoi(str_buf.substr(0, position));
        break;
      }
    }

    stream.str("");

    while(str_buf.length()<recd+1+to_string(recd).length())
    {
      len = socket.read_some(boost::asio::buffer(buf), ignored_error);
      for (int i = 0; i < len; ++i)
      {
        stream << buf[i];
      }
      str_buf = str_buf + stream.str();
      stream.str("");
    }

    position = str_buf.find("\n");
    str_buf = str_buf.substr(position+1);

    cout << str_buf << endl;
    gettimeofday(&end_time, nullptr);
    // cout << endl
    // << "Transmitted " << xmitBytes << " bytes in "
    // << (end_time.tv_sec - start_time.tv_sec) << " seconds" << endl;
  }

  bool send_message(string data){
    auto starttime = high_resolution_clock::now();

    try{
      // auto socket = create_socket(server_name, port);
      write_read_buffer(socket_, data);
    }catch (std::exception &e){
      std::cerr << e.what() << std::endl;
      return false;
    }

    auto endtime = high_resolution_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(endtime - starttime);
    // if (!bool_specific_server)
    // {
    //   latency = latency + time_span;
    // }
    return true;
    // cout << "Time span: " << time_span.count() << endl;
  }

  void close_socket(){
    socket_.close();
  }

private: 
  ip::tcp::socket socket_;
  string port_;
  string server_;
};

class dataGenerator
{
public:
  dataGenerator(int num_server):key_(""),num_server_(num_server){}

  string command_test(){
    string data, key, value;
    cout<<"Client: ";
    getline(cin, data);
    if (cin.eof()){
      return "EXIT";
    } 

    // ******* PUT *******
    if (data == "PUT")
    {
      cout << "KEY:";
      getline(cin, key);
      cout << "VAL:";
      getline(cin, value);
      // PUT\nkey\nvalue
      data = "PUT\n" + key + "\n" + value;
    }
    // ******* GET *******
    else if (data == "GET")
    {
      cout << "KEY:";
      getline(cin, key);
      // GET\nlength_key\nkey
      data = "GET\n" + key;
    }
    // ******* DEL *******
    else if (data == "DEL")
    {
      cout << "KEY:";
      getline(cin, key);
      // DEL\nlength_key\nkey
      data = "DEL\n" + key;
    }
    // ----------------------- JUST FOR TEST --------------
    // ******* SHOW *******
    else if (data == "SHOW")
    {
      data = "SHOW\n";
    }
    // ******* INIT *******
    else if (data == "INIT")
    {
      data = "INIT\n";
    }
    // ----------------------- JUST FOR TEST -------------
    return data;
  }

  string command(int put_or_get, int length_key){
    int length_value;
    string data, key, value;
    length_value = rand()%5 + 1;

    key = random_str(length_key);
    value = random_str(length_value);
    key_ = key;

    if (put_or_get <= 40)
    { //put
      data = "PUT\n" + key + "\n" + value;
      cout << "PUT  " << key << ": ";
    }
    else
    { //get
      data = "GET\n" + key;
      cout << "GET  " << key << ": ";
    }
    return data;
  }

  string random_str(int length){
    stringstream ss;
    string random_str;
    int flag;
    for (int i = 0; i < length; i++)
    {
      flag = rand() % 2;
      switch (flag)
      {
      // case 0:
      // ss << char('A' + rand() % 26);
      // break;
        case 0:
        ss << char('a' + rand() % 26);
        break;
        case 1:
        ss << char('0' + rand() % 10);
        break;
        default:
        ss << 'x';
        break;
      }
    }

    ss >> random_str;
    return random_str;
  }

  int pickServer(){
    int id_server;
    hash<string> h;

    size_t n = h(key_);
    id_server = n % num_server_;
    return id_server;
  }

private:
  string key_;
  int num_server_;
};

/** Print some helpful usage information */
void usage(const char *progname) {
  using std::cout;
  cout << "  Usage: " << progname << " [options]\n";
  cout << "    -p       : Port on which to listen (default 41100)\n";
  cout << "    -h       : print this message\n";
  //cout << "    -d       : destination address\n";
  cout << "    -c       : number of commands\n";
  cout << "    -l       : length of key\n";
  //cout << "    -s       : specify dst addr(works with -d)\n";
  cout << "    -t       : test mode\n";
  
}

int main(int argc, char *argv[]) {
  string port = "40300", data;
  bool show_help = false; 
  bool test_mode = false;
  // bool bool_specific_server = false;
  //string server_name = "localhost";
  int num_command = 10, key_len = 3;

  // Parse the command line options:
  int o;
  while ((o = getopt(argc, argv, "p:d:c:l:hts")) != -1) {
    switch (o) {
      case 'h':
      show_help = true;
      break;
      case 'p':
      port = string(optarg);
      break;
      // case 'd':
      // server_name = string(optarg);
      // break;
      case 'c':
      num_command = atoi(optarg);
      break;
      case 'l':
      key_len = atoi(optarg);
      break;
      case 't':
      test_mode = true;
      break;
      // case 's':
      // bool_specific_server = true;
      // break;
      default:
      show_help = true;
      break;
    }
  }

  // Print help and exit
  if (show_help) {
    usage(basename(argv[0]));
    exit(0);
  }

  // load info of Server IP address from 'DHTConfig'
  vector<string> server_list;
  int num_server;
  ifstream in("DHTConfig");
  if (in.is_open())
  {
    for (string str; getline(in, str) ;)
    { 
      if (str.find("IP")!=str.npos)
      {
        str = str.substr(str.find("=")+1);
        boost::split(server_list, str, boost::is_any_of(","));
      }
    }
    in.close();
  }
  num_server = server_list.size();
  server_list.push_back("localhost");

  dataGenerator dataGen(num_server);
  /**********************************************/
  /* test mode: only communicate with localhost */
  /**********************************************/
  if (test_mode)      
  {  
    DHTClient clientTest(port, server_list[num_server]); // localhost
    
    while(true){
      data = dataGen.command_test(); 
      if (data == "EXIT")
      {
        break;
      } 
      clientTest.send_message(data);
    }
    clientTest.close_socket();
  }
  /**********************************************/
  /********** communicate with the DHT **********/
  /**********************************************/
  else      
  {  
    int put_or_get, serverID;

    vector<DHTClient> clientNode;
    for (int i = 0; i < num_server; ++i)
    {
      clientNode.push_back(DHTClient(port, server_list[i]));
    }
    
    srand((unsigned)time(NULL)); 
    
    for (int i = 0; i < num_command; ++i)
    {
      put_or_get = (rand()%100)+1;
      data = dataGen.command(put_or_get, key_len);
      serverID = dataGen.pickServer();
      clientNode[serverID].send_message(data);
    }

    for (int i = 0; i < num_server; ++i)
    {
      clientNode[i].close_socket();
    }
    // cout << "Total Time: " << latency.count() << endl;
  }

  return 0;


}


