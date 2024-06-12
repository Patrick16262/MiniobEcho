
#include <arpa/inet.h>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>
#include "debug.h"

using namespace std;

namespace DebugUtils {

void write_startup_debug_external(int argc, char **argv)
{
  static const char *header = R"(
  ===========================
  === Observer Debug Info ===
  ===========================
  )";
  write_debug_external(header);
  write_debug_external("arguments:");
  stringstream ss;

  for (int i = 0; i < argc; i++) {
    ss << argv[i] << " ";
  }

  write_debug_external(ss.str().c_str());
}

void write_debug_external(const char *msg)
{
  std::ofstream out_file("../debug.txt", std::ios_base::app);
  out_file << msg << std::endl;
}

long read_debug_external(char *buf, long max_size, long offset = 0)
{
  std::ifstream in_file("../debug.txt");
  in_file.seekg(offset, std::ios_base::beg);
  in_file.read(buf, max_size);
  return in_file.gcount();
}

void debug_singal_handler(int sig)
{
  string msg = "Received signal: " + to_string(sig);
  write_debug_external(msg.c_str());
}

void exec_debug_thread()
{
  static volatile bool have_debug_thread = false;
  if (have_debug_thread) {
    return;
  }
  have_debug_thread = true;

  string host = "172.18.16.1";
  int    port = 18465;

  // 1. 创建socket
  int client_fd = socket(AF_INET, SOCK_STREAM, 0);

  // 2. 连接服务器
  struct sockaddr_in server_addr;

  server_addr.sin_family      = AF_INET;
  server_addr.sin_port        = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(host.c_str());

  int ret = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  if (ret == -1) {
    cout << "connect failed: " << strerror(errno) << endl;
    return;
  }

  // 3. 发送数据
  new thread([client_fd]() {
    const int MAX_SIZE = 1024 * 64;
    char      buf[MAX_SIZE];
    long      offset = 0;
    while (true) {
      long len = read_debug_external(buf, MAX_SIZE, offset);
      offset += len;
      if (len > 0) {
        send(client_fd, buf, len, 0);
      }
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  });
}

}  // namespace DebugUtils