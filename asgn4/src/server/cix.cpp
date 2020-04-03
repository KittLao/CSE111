// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <string>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"put" , cix_command::PUT },
   {"get" , cix_command::GET },
   {"rm"  , cix_command::RM  }
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      outlog << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_put (client_socket& server, string filename) {
   /* Attempt to open file and read from it */
   int fd = open(filename.c_str(), O_RDONLY);
   if(fd < 0) {
      outlog << "Error opening " << filename << endl;
      return;
   }

   char buffer[0x1000];
   size_t bytes_rd = read(fd, buffer, sizeof buffer);
   close(fd);
   if(bytes_rd < 0) {
      cout << "Error reading " << filename << endl;
      return;
   }
   buffer[bytes_rd] = '\0';
   outlog << "contents in file:" << endl << buffer << endl;

   /* Set up header and send it to server */
   cix_header header;
   header.command = cix_command::PUT;
   header.nbytes = bytes_rd;
   strcpy(header.filename, filename.c_str());
   outlog << "sending header " << header << endl;
   send_packet(server, &header, sizeof header);

   /* Send contents from file to server */
   send_packet(server, buffer, bytes_rd);
   outlog << "sent " << bytes_rd << " bytes" << endl;

   /* Server receive response from server whether request failed or not */
   recv_packet(server, &header, sizeof header);
   if(header.command == cix_command::NAK)
      outlog << "Error on PUT" << endl;
   else
      outlog << "Success on PUT" << endl;
}

void cix_get (client_socket& server, string filename) {
   /* Set up header and send it to server */
   cix_header header;
   header.command = cix_command::GET;
   strcpy(header.filename, filename.c_str());
   send_packet(server, &header, sizeof header);

   /* Receive response from server on whether request failed or not */
   recv_packet(server, &header, sizeof header);
   if(header.command == cix_command::NAK) {
      outlog << "Error on GET" << endl;
      return;
   }

   /* Attempt to open file and store buffer into file */
   int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
   if(fd < 0) {
      outlog << "Error opening " << filename << endl;
      return;
   }
   auto buffer = make_unique<char[]> (header.nbytes + 1);
   recv_packet(server, buffer.get(), header.nbytes);
   outlog << "received " << header.nbytes << " bytes" << endl;
   buffer[header.nbytes] = '\0';
   int bytes_wr = write(fd, buffer.get(), header.nbytes);
   close(fd);
   if(bytes_wr < 0) {
      outlog << "Error writing to " << filename << endl;
      return;
   }
   outlog << "bytes wirtten " << bytes_wr << endl;
   outlog << "contents written " << buffer.get() << endl;
   outlog << "Success on GET" << endl;
}

void cix_rm (client_socket& server, string filename) {
   cix_header header;
   header.command = cix_command::RM;
   strcpy(header.filename, filename.c_str());
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   if(header.command == cix_command::NAK)
      outlog << "Error on RM" << endl;
   else
      outlog << "Success on RM" << endl;
}


void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         outlog << "input " << line << endl;
         char* line_ = const_cast<char*>(line.c_str());
         char* cmd_ = strtok_r(line_, " ", &line_);
         const auto& itor = command_map.find ((string)cmd_);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::PUT:
               cix_put(server, strtok_r(line_, " ", &line_));
               break;
            case cix_command::GET:
               cix_get(server, strtok_r(line_, " ", &line_));
               break;
            case cix_command::RM:
               cix_rm(server, strtok_r(line_, " ", &line_));
               break;
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}
