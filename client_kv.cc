#include <iostream>
#include <libgen.h>
#include <boost/filesystem.hpp>
#include <unistd.h>
#include <boost/asio.hpp>
#include <time.h>

using namespace std;

#define BUF_SIZE 4096

/**
 * Connect to a server so that we can have bidirectional communication on the 
 * socket (represented by a file descriptor) that this function returns
 *
 * @param hostname The name of the server (ip or DNS) to connect to 
 * @param port the server's port that we should use
 */

boost::asio::ip::tcp::socket connect_to_server(std::string hostname,
 std::string port) {
  using namespace boost::asio;
  io_service io_service;
  ip::tcp::resolver resolver(io_service);
  ip::tcp::resolver::query query(hostname, port);
  ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  ip::tcp::socket socket(io_service);
  connect(socket, endpoint_iterator);
  return socket;
}

/**
 * Receive text from stdin, send it to the server, and then print whatever the
 * server sends back.
 * 
 * @param socket The socket file descriptor to use for the echo operation
 */

string command(){
  string data, key, value, str_length_key, str_length_value;
  int length_key,length_value;
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

    str_length_key = to_string(key.length());
    str_length_value = to_string(value.length());
      // protocol
      // PUT\nlength_key\nkey\nlength_value\nvalue
    data = "PUT\n" + str_length_key + "\n" + key + "\n" + str_length_value + "\n" + value;
  }

    // ******* GET *******
  else if (data == "GET")
  {

    cout << "KEY:";
    getline(cin, key);

    str_length_key = to_string(key.length());
      // protocol
      // GET\nlength_key\nkey
    data = "GET\n" + str_length_key + "\n" + key;
  }

    // ******* DEL *******
  else if (data == "DEL")
  {

    cout << "KEY:";
    getline(cin, key);

    str_length_key = to_string(key.length());
      // protocol
      // DEL\nlength_key\nkey
    data = "DEL\n" + str_length_key + "\n" + key;
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

string random_str(int length){
  stringstream ss;
  string random_str;
  int flag;
  for (int i = 0; i < length; i++)
  {
    flag = rand() % 3;
    switch (flag)
    {
      case 0:
      ss << char('A' + rand() % 26);
      break;
      case 1:
      ss << char('a' + rand() % 26);
      break;
      case 2:
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

string command_test(int put_or_get){
  int length_key, length_value;
  string data, key, value;

  // srand((unsigned)time(NULL)); 
  length_key = rand()%5 + 1;
  length_value = rand()%5 + 1;
  
  key = random_str(length_key);
  value = random_str(length_value);


  if (put_or_get <= 40)
  { //put
    data = "PUT\n" + to_string(length_key) + "\n" + key + "\n" + to_string(length_value) + "\n" + value;
  }
  else
  { //get
    data = "GET\n" + to_string(length_key) + "\n" + key;
  }

  return data;
}

string pickServer(string data){
  int position, len_key, num_server, id_server;
  string key, command;

  num_server = 3;

  position = data.find("\n");
  if (position <= data.length()){
    command = data.substr(0, position);
    data = data.substr(position+1);
    if (command == "PUT" || command == "GET" || command == "DEL"){
      position = data.find("\n");
      len_key = stoi(data.substr(0, position));
      data = data.substr(position+1);
      position = data.find("\n");
      key = data.substr(0, position);
    }
  }

  if (key == "")
  {
    return "localhost";
  }


  // hash(key)
  hash<string> h;
  size_t n = h(key);
  id_server = (n % num_server)+1;
  if (id_server == 1)
  {
    // return "18.222.251.249";
    return "localhost";
  }
  else if (id_server == 2)
  {
    // return "18.222.141.133";
    return "localhost";
  }
  else if (id_server == 3)
  {
    // return "18.217.26.7";
    return "localhost";
  }
  return NULL;
}


void echo_client(boost::asio::ip::tcp::socket &socket, string data){
  using namespace std;
  using namespace boost::asio;

  // track connection duration, bytes transmitted
  size_t xmitBytes = 0;
  struct timeval start_time, end_time;
  gettimeofday(&start_time, nullptr);

    // Get the data from stdin, This assumes that we haven't redirected stdin
    // to a socket

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

  cout << "Server:" << endl;
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


void send_message(string port, string server_name, string data){

  server_name = pickServer(data);
  cout << "send to " << server_name << endl;
  try{
      // Set up the client socket

    auto socket = connect_to_server(server_name, port);
      //Start the 'echo' interaction
    echo_client(socket, data);

      // clean up when done
    socket.close();
  }catch (std::exception &e){
    std::cerr << e.what() << std::endl;
  }
}

/** Print some helpful usage information */
void usage(const char *progname) {
  using std::cout;
  cout << "  Usage: " << progname << " [options]\n";
  cout << "    -p <int> : Port on which to listen (default 41100)\n";
  cout << "    -h       : print this message\n";
}

int main(int argc, char *argv[]) {
  // Config vars that we get via getopt
  string port = "41100";       // random seed
  bool show_help = false; // show usage?
  bool test_mode = false;
  string server_name = "localhost";
  // Parse the command line options:
  int o;
  while ((o = getopt(argc, argv, "p:d:ht")) != -1) {
    switch (o) {
      case 'h':
      show_help = true;
      break;
      case 'p':
      port = string(optarg);
      break;
      case 'd':
      server_name = string(optarg);
      break;
      case 't':
      test_mode = true;
      break;
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

  string data;
  int put_or_get;

  if (test_mode)       // test mode
  {  
    srand((unsigned)time(NULL)); 
    for (int i = 0; i < 10; ++i)
    {
      put_or_get = (rand()%100)+1;
      data = command_test(put_or_get);
      send_message(port, server_name, data);
    }
  }
  else              // user mode
  {  
    while(true){
      data = command(); 
      if (data == "EXIT")
      {
        break;
      } 
      send_message(port, server_name, data);
    }


  }
  return 0;


}


