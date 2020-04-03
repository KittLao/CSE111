// $Id: listmap.tcc,v 1.12 2019-02-21 17:28:55-08 - - $
#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next, node* prev,
                                     const value_type& value_):
            link (next, prev), value (value_) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));
   while(begin() != end()) erase(begin());
}

template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::listmap(const listmap& that) {
   *this = that;
}


template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::listmap&
listmap<Key,Value,Less>::operator= (const listmap& that) {
   iterator itr = that.begin();
   while(itr != that.end()) {
      insert(*itr);
      ++itr;
   }
   return this;
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
   DEBUGF ('l', &pair << "->" << pair);
   node* new_node;
   if(find(pair.first) != end()) erase(find(pair.first));
   if(begin() == end()) { // Empty case
      new_node = new node(anchor()->next, anchor(), pair);
      anchor()->next = new_node;
      anchor()->prev = new_node;
   } else if(less(pair.first, anchor()->next->value.first)) { // Insert to front
      new_node = new node(anchor()->next, anchor(), pair);
      anchor()->next->prev = new_node;
      anchor()->next = new_node;
   } else if(less(anchor()->prev->value.first, pair.first)) { // Insert to back
      new_node = new node(anchor(), anchor()->prev, pair);
      anchor()->prev->next = new_node;
      anchor()->prev = new_node;
   } else { // Insert in middle
      node* temp = anchor()->next;
      while(less(temp->value.first, pair.first) && temp != anchor()) // Find insert position
         temp = temp->next;
      new_node = new node(temp, temp->prev, pair);
      temp->prev->next = new_node;
      temp->prev = new_node;
      temp = nullptr;
   }
   return iterator(new_node);
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) {
   DEBUGF ('l', that);
   iterator itr = begin();
   while(itr != end()) {
      if(!less(itr->first, that) && !less(that, itr->first)) return itr;
      ++itr;
   }
   return itr;
}

//
// iterator listmap::erase (iterator position)
//

template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
   DEBUGF ('l', &*position);
   if(position == end()) return position; 
   Key k = position->first;
   node* temp;
   node* next_node;
   if(!less(k, anchor()->next->value.first) && !less(anchor()->next->value.first, k)) { // erase front
      temp = anchor()->next;
      next_node = temp->next;
      anchor()->next = anchor()->next->next;
      anchor()->next->prev = anchor();
      temp = nullptr;
   } else if(!less(k, anchor()->prev->value.first) && !less(anchor()->prev->value.first, k)) { // erase back
      temp = anchor()->prev;
      next_node = temp->next;
      anchor()->prev = anchor()->prev->prev;
      anchor()->prev->next = anchor();
      temp = nullptr;
   } else {
      temp = anchor()->next;
      while(!less(k, temp->value.first) && !less(temp->value.first, k) && temp != anchor()) // Find insert position
         temp = temp->next;
      next_node = temp->next;
      temp->prev->next = temp->next;
      temp->next->prev = temp->prev;
      temp = nullptr;
   }
   return iterator(next_node);
}

template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::displayAll() {
   iterator itr =  begin();
   while(itr != end()) {
      cout << itr->first << " = " << itr->second << endl;
      ++itr;
   }
}

template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::displayKeyFromValue(const Value& that) {
   iterator itr = begin();
   while(itr != end()) {
      if(itr->second == that)
         cout << itr->first << endl;
      ++itr;
   }
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
   DEBUGF ('l', where);
   return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
   DEBUGF ('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
   DEBUGF ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
   DEBUGF ('l', where);
   where = where->prev;
   return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
            (const iterator& that) const {
   return this->where != that.where;
}

