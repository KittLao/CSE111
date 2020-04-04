// $Id: commands.cpp,v 1.17 2018-01-25 14:02:55-08 - - $

#include "commands.h"
#include "debug.h"
#include "file_sys.h"
#include <iomanip>

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
   {"#"     , fn_ignore}
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

/* My code begins */
void print_wordvec (const wordvec& words, const string& delimeter) {
   for (string w : words)
      cout << w << delimeter;
   cout << endl;
}

wordvec get_path (const wordvec& command) {
   if (command.size() == 0) throw command_error ("No command found");
   wordvec path;
   for (int i = 1; i < command.size(); i++)
      path.push_back(command[i]);
   return path;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path = get_path(words);
   if (path.size() == 0) throw command_error ("No files selected.");
   wordvec contents;
   for (string file : path) {
      if (!state.cwd->contents->find(file))
         //throw command_error (file + " not found");
         cout << file << " not found" << endl;
      else if (state.cwd->contents->search_dir(file)->contents->get_type() == file_type::DIRECTORY_TYPE)
         throw command_error("Cannot cat a directory");
      else {
         contents = state.cwd->contents->search_dir(file)->contents->readfile();
         cout << word_range (contents.cbegin(), contents.cend());
      }
   }
   cout << endl;
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path = get_path(words);
   if (path.size() == 0 or path[0] == "/") {
      if (state.cwd != nullptr)
         state.cwd.reset();
         state.cwd = state.root;
   } else {
      for (string dir : path) {
         if (state.cwd->contents->search_dir(dir)->contents->get_type() == file_type::PLAIN_TYPE)
            throw command_error ("Cannot cd into a file");
         inode_ptr temp = state.cwd->contents->search_dir(dir);
         state.cwd.reset();
         state.cwd = temp;
      }
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void print_path (const wordvec& path) {
   cout << path[0];
   for (int i = 1; i < path.size() - 1; i++)
      cout << path[i] << "/";
   if (path.size() > 1)
      cout << path[path.size() - 1];
   cout  << ":" << endl;
}

void ls_helper(const base_file_ptr& base) {
   dirents_itr itr = base->get_itr();
   auto it_b = itr.itr_b, it_e = itr.itr_e;
   wordvec path = base->get_path();
   print_path(path);
   while (it_b != it_e) {
      cout << "     ";
      cout << it_b->second->get_inode_nr();
      cout << "      ";
      //cout << setw(6);
      //cout << left;
      cout << it_b->second->contents->size();
      //cout << setw(6);
      //cout << left;
      cout << "   ";
      cout << it_b->first << endl;
      it_b++;
   }
}


void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path = get_path(words);
   inode_ptr temp = state.cwd;   
   if (path.size() > 0) fn_cd (state, words);
   ls_helper(state.cwd->contents);
   state.cwd = temp;
}

void lsr_helper(const base_file_ptr& base) {
   if (base->get_type() != file_type::DIRECTORY_TYPE)
      return; 
   else {
      ls_helper(base);
      dirents_itr itr = base->get_itr();
      auto it_b = itr.itr_b, it_e = itr.itr_e;
      while (it_b != it_e) {
         if(it_b->first != "." && it_b->first != ".." &&
            it_b->second->contents->get_type() != file_type::PLAIN_TYPE)
            ls_helper(it_b->second->contents);
         it_b++;
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_state state_ = state;
   wordvec path	= get_path(words);
   if (path.size() > 0) fn_cd (state_, words);
   lsr_helper(state_.cwd->contents);
}

void fn_make (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path = get_path(words);
   if (path.size() == 0) throw command_error ("No file specified.");
   if (state.cwd->contents->find(path[0]))
      cout << path[0] << "already exists" << endl;
   else {
      inode_ptr new_inode =  state.cwd->contents->mkfile(path[0]);
      wordvec data;
      for (int i = 1; i < path.size(); i++)
         data.push_back(path[i]);
      state.cwd->contents->write_to_file(path[0], data);
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path = get_path(words);
   if (path.size() == 0) throw command_error ("No name specified.");
   if (state.cwd->contents->find(path[0]))
      cout << path[0] << "already exists" << endl;
   else {
      inode_ptr new_inode = state.cwd->contents->mkdir(path[0]);
      state.cwd->contents->init_dir(path[0], new_inode, state.cwd);
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path	= get_path(words);
   if (path.size() == 0) throw command_error ("No prompt specified.");
   state.set_prompt(path[0]);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << state.cwd->contents->get_name() << endl;
   //print_wordvec(state.cwd->contents->get_path(), " ");
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path	= get_path(words);
   if (path.size() == 0) throw command_error ("No path specified");
   state.cwd->contents->remove(path[0]);
}

// Recursively removes the files or directory.
void rmr_helper (base_file_ptr& base, const string& n) {
   if(base->search_dir(n)->contents->get_type() == file_type::PLAIN_TYPE) {
      base->remove(n);
   } else {
      dirents_itr itr = base->search_dir(n)->contents->get_itr();
      auto it_b = itr.itr_b, it_e = itr.itr_e;
      while (it_b != it_e) {
         if (it_b->second->contents->get_type() == file_type::PLAIN_TYPE)
            it_b->second->contents->remove(it_b->first);
         else if (it_b->first != "." && it_b->first != "..")
            rmr_helper(it_b->second->contents, it_b->first);
         it_b++;
      }
      base->remove(n);
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   wordvec path = get_path(words);
   if (path.size() == 0) throw command_error ("No path specified");
   rmr_helper(state.cwd->contents, path[0]);
}
/* My code ends */

void fn_ignore(inode_state& state, const wordvec& words) {}
