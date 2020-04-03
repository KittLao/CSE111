// $Id: file_sys.cpp,v 1.6 2018-06-27 14:44:57-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

// When the inode_state is created, the root's
// parent's are itself. Cwd needs to point to
// the root.
inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   root->contents->insert_dir_("..", root);
   root->contents->insert_dir_(".", root);
   root->contents->update_path(root->contents->get_path(), "/");
   cwd = root;
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}
/*
inode_state::inode_state (const inode_ptr& that) {
   *this = that;
}
*/
void inode_state::operator= (const inode_state& that) {
   cwd.reset();
   cwd = that.cwd;
   root.reset();
   root = that.root;
   //prompt_ = that.prompt_();
}

void inode_state::set_prompt (const string& prompt) { prompt_ = prompt + " "; }

// Make the contents point to the type passed in
inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

// Number of characters in the file. Length each word
// plus the amount of spaces. Amount of spaces is just
// size of vector.
size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   for (string word : data)
      size += word.length();
   size += data.size();
   return size - 1;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::search_dir (const string& key) {
   throw file_error ("is a plain file");
}

bool plain_file::find(const string& key) {
   throw file_error ("is a plain file");
}

dirents_itr plain_file::get_itr () {
   throw file_error ("is a plain file");
}

file_type plain_file::get_type () {
   return file_type::PLAIN_TYPE;
}

void plain_file::write_to_file (const string& file, const wordvec& data) { 
   throw file_error ("is a plain file");
}

void plain_file::insert_dir (const string& dir, const string& key, const inode_ptr& value) {
   throw file_error ("is a plain file");
}

void plain_file::insert_dir_ (const string& key, const inode_ptr& value)	{ 
   throw file_error ("is a plain file");
}

void plain_file::init_dir (const string& dir, const inode_ptr& current, const inode_ptr& parent) {
   throw file_error ("is a plain file");
}

void plain_file::update_path (const wordvec& old, const string& new_) {
   throw file_error ("is a plain file");
}

wordvec plain_file::get_path () {
   throw file_error ("is a plain file");
}

string plain_file::get_name () {
   return name;
}

void plain_file::set_name (const string& n) {
   name = n;
}


size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   size = dirents.size();
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

// If file or directory doens't exist, then throw
// and error. If it exist and is a directory, see
// if the directory is empty or not. If not, then
// cannot delete.
void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   if (dirents.find(filename) == dirents.end()) throw file_error (filename + " not found");
   if (dirents.at(filename)->contents->get_type() == file_type::DIRECTORY_TYPE) {
      if (dirents.at(filename)->contents->size() > 2)
         throw file_error (filename + " must be empty");
      else {
         dirents.at(filename)->contents->search_dir("..").reset();
         dirents.at(filename)->contents->search_dir(".").reset();
         dirents.erase(filename);
      }
   }
   else {
      dirents.at(filename).reset();
      dirents.erase(filename);
   }
}

// If directory already exists as a directory or file,
// throw and error. Otherwise, create a new inode of
// type directory. Create a new inode_ptr that points
// to the new inode. Insert the new inode_ptr to the
// dirents.
inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   if (dirents.find(dirname) != dirents.end()) throw file_error (dirname + " already exists");
   inode new_inode(file_type::DIRECTORY_TYPE);
   new_inode.contents->update_path(path, dirname);
   inode_ptr new_inode_ptr = make_shared<inode>(new_inode);
   dirents.insert({dirname, new_inode_ptr});
   return new_inode_ptr;
}

// If the file already exists as file or directory, throw an
// error. Otherwise create a new inode that is a file type.
// Create a new inode_ptr that points to the new inode.
// Insert it into the dirents.
inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   if (dirents.find(filename) != dirents.end()) throw file_error (filename + " already exists");
   inode new_inode(file_type::PLAIN_TYPE);
   new_inode.contents->set_name(filename);
   inode_ptr new_inode_ptr = make_shared<inode>(new_inode);
   dirents.insert({filename, new_inode_ptr});
   return new_inode_ptr;
}

inode_ptr directory::search_dir (const string& key) {
   if (dirents.find(key) == dirents.end()) throw file_error (key + "2 not found");
   return dirents.at(key);
}

bool directory::find(const string& key) {
   if (dirents.find(key) == dirents.end())
      return false;
   return true;
}

dirents_itr directory::get_itr () {
   dirents_itr itr;
   itr.itr_b = dirents.begin();
   itr.itr_e = dirents.end();
   return itr;
}

file_type directory::get_type () {
   return file_type::DIRECTORY_TYPE;
}

// Write to a file while in cwd. While in the cwd, find the file
// that needs to be updated, and write the data to the file.
// Update the cwd by inserting the new file into the dirents.
void directory::write_to_file (const string& file, const wordvec& data) {
   inode_ptr data_ = dirents.at(file);
   data_->contents->writefile(data);
   dirents.at(file).reset();
   dirents.erase(file);
   dirents.insert({file, data_});
}

// Insert something into the directory dir, while in cwd. This is
// used when adding "." and ".." when making a new directory. The
// cwd simply find the new directory created, and calls insert_dir_.
// After updating the new directory, the cwd needs to update it's
// own directory by inserting the newly updated directory into its
// own.
void directory::insert_dir (const string& dir, const string& key, const inode_ptr& value) {
   inode_ptr value_ = dirents.at(dir);
   value_->contents->insert_dir_(key, value);
   dirents.at(dir).reset();
   dirents.erase(dir);
   dirents.insert({dir, value_});
}

// Simply insert something into the map. This is usefull for the 
// inode_state constructor because it can directly create the "."
// and ".." when initiated.
void directory::insert_dir_ (const string& key, const inode_ptr& value) {
   inode_ptr value_ = value;
   if (dirents.find(key) != dirents.end()) {
      dirents.at(key).reset();
      dirents.erase(key);
   }
   dirents.insert({key, value_});
}

void directory::init_dir (const string& dir, const inode_ptr& current, const inode_ptr& parent) {
   insert_dir(dir, ".", current);
   insert_dir(dir, "..", parent);
}

// Always update the path each time a new directory is made.
void directory::update_path (const wordvec& old, const string& new_) {
   path = old;
   path.push_back(new_);
}

string directory::get_name () {
   return path.back();
}

wordvec directory::get_path () {
   return path;
}

void directory::set_name (const string& n) {
   path.pop_back();
   path.push_back(n);
}
