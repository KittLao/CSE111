// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

str_str_map m;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

/*
Case 1: key = 
   removes "key" fromt he listmap
Case 2: = value
   prints out all the keys that maps to "value"
Case 3: =
   prints out all the key and value in the listmap
Case 4: key = value
   inserts the pair (key, value) into the listmap
*/
void key_value_cmd(const smatch& command) {
   if(command[1] != "" && command [2] != "") { // key and value both present
      m.erase(m.find(command[1]));
      m.insert({command[1], command[2]});
   } else if (command[1] != "" && command[2] == "") { // only key present
      m.erase(m.find(command[1]));
   } else if (command[1] == "" && command [2] != "") { // only value present
      m.displayKeyFromValue(command[2]);
   } else  { //	neither	key or value present
      m.displayAll();
   }
}

/*
This function is called if there is no equal sign. 
The line that is read acts as a key. It then looks
it up in the listmap.
*/
void query_cmd(const smatch& command) {
   auto value = m.find(command[1]);
   if(value == m.end()) cout << command[1] << " : key not found" << endl;
   else cout << value->first << " = " << value->second << endl;
}

void read_line(const string& line) {
   regex comment {R"(^\s*(#.*)?$)"};
   regex key_value {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed {R"(^\s*([^=]+?)\s*$)"};
   smatch command;
   if(regex_search(line, command, comment)) { // comments are ignored
      //cout << endl;
   } else if(regex_search(line, command, key_value)) { // 3 cases here
      key_value_cmd(command);
   } else if(regex_search(line, command, trimmed)) { // 1 case
      query_cmd(command);
   } else {
      cout << "this is an error" << endl;
   }
}

void read_file (istream& infile, const string& file_name) {
   static string colons(32, ':');
   cout << colons << endl << file_name << endl << colons << endl;
   while(true) {
      string line;
      getline(infile, line);
      if(infile.eof()) break;
      read_line(line);
   }
}

int main (int argc, char** argv) {
   string cin_name = "-";
   string prog_name {argv[0]}; // the program name is just keyvalue
   vector<string> file_names (&argv[1], &argv[argc]); // a list of files
   if(file_names.size() == 0) // if there is no files, read from std input
       file_names.push_back(cin_name);
   for(const auto& file_name : file_names) {
      if(file_name == cin_name) // if a dash is inputed instead of a file, read from std input
         read_file(cin, file_name);
      else {
         ifstream file(file_name);
         if(file.fail()) { // throw error if file cannot open
            cout << prog_name << " : " << file_name << " does not exist."  << endl;
            return -1;
         } else {
            read_file(file, file_name);
            file.close();
         }
      }
   }


   return 0;

   /*
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   str_str_map test;
   for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      str_str_pair pair (*argp, to_string<int> (argp - argv));
      cout << "Before insert: " << pair << endl;
      test.insert (pair);
   }

   for (str_str_map::iterator itor = test.begin();
        itor != test.end(); ++itor) {
      cout << "During iteration: " << *itor << endl;
   }

   str_str_map::iterator itor = test.begin();
   test.erase (itor);

   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
   */
}
