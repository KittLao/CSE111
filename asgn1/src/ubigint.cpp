// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <string>
#include <iostream>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that) {
   DEBUGF ('~', this << " -> " << *this);
   if (that == 0) ubig_value.push_back(that);
   while (that > 0) {
      ubig_value.push_back(that%10);
      that /= 10;
   }
}

ubigint::ubigint (const string& that): ubig_value(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (int i = that.length() - 1; i >= 0; i--) {
      if (not isdigit (that[i])) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      } 
     ubig_value.push_back(static_cast<udigit_t>(that[i] - '0'));
   }
   this->removeZeros();
}

ubigint ubigint::operator+ (const ubigint& that) const {
   int m = min (this->ubig_value.size(), that.ubig_value.size()), i = 0;
   udigit_t n = 0, carry = 0;
   ubigvalue_t new_value;
   // Iterate through the minimum size of each digit.
   // Add the two digits and if they are greater than
   // 10, turn carry to 1, else 0.
   while (i < m) {
      n = this->ubig_value[i] + that.ubig_value[i] + carry;
      new_value.push_back (n%10);
      carry = n/10;
      i++;
   }
   // The two while loops will handle different
   // size digits.
   while (i < this->ubig_value.size()) {
      n = this->ubig_value[i] + carry;
      new_value.push_back (n%10);
      carry = n/10;
      i++;
   }
   while (i < that.ubig_value.size()) {
      n = that.ubig_value[i] + carry;
      new_value.push_back (n%10);
      carry = n/10;
      i++;
   }
   new_value.push_back (carry);
   ubigint new_bigint;
   new_bigint.ubig_value = new_value;
   new_bigint.removeZeros();
   return new_bigint;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   int m = min (this->ubig_value.size(), that.ubig_value.size()), i = 0;
   int n = 0, carry = 0;
   ubigvalue_t new_value;
   // Iterate through the minimum size of each digit.
   // Subtract the digit from "this" to "that", and
   // if there is a negative value, then make carry
   // negative, and add 10 to the value.
   while (i < m) {
      n = this->ubig_value[i] - that.ubig_value[i] + carry;
      carry = n >= 0 ? 0 : -1;
      n = n >= 0 ? n : 10 + n;
      new_value.push_back(static_cast<udigit_t>(n));
      i++;
   }
   // The two while loops will take care of handling
   // different size digits. The algorithm is the same.
   while (i < this->ubig_value.size()) {
      n = this->ubig_value[i] + carry;
      carry = n	>= 0 ? 0 : -1;
      n	= n >= 0 ? n : 10 + n;
      new_value.push_back(static_cast<udigit_t>(n));
      i++;
   }
   while (i < that.ubig_value.size()) {
      n = that.ubig_value[i] + carry;
      carry = n >= 0 ? 0 : -1;
      n = n >= 0 ? n : 10 + n;
      new_value.push_back(static_cast<udigit_t>(n));
      i++;
   }
   carry = carry < 0 ? -1 * carry : carry;
   new_value.push_back(static_cast<udigit_t>(carry));
   ubigint new_bigint;
   new_bigint.ubig_value = new_value;
   new_bigint.removeZeros();
   return new_bigint;
}

ubigint ubigint::operator* (const ubigint& that) const {
   int zeros = 0, s1 = that.ubig_value.size(), s2 = this->ubig_value.size();
   udigit_t carry = 0, n = 0;
   ubigvalue_t temp_value;
   ubigint new_bigint (0), temp_bigint;
   // Computes n number of single digit multiplication 
   // on the "that", where n is the number of digits
   // on "this"
   for (int i = 0; i < s1; i++) {
      // This for loop does the actual single digit 
      // multiplcation on the ubig.
      for(int j = 0; j < s2; j++) {
         n = this->ubig_value[j] * that.ubig_value[i] + carry;
         temp_value.push_back(n%10);
         carry = n/10;
      }
      temp_value.push_back(carry);
      carry = 0;
      // This for loop adds the necessary zero's when
      // generating the list of single digit
      // multiplication on list. Increment the amount
      // of zeros each per single digit multiplication.
      for (int i = 0; i < zeros; i++) temp_value.insert(temp_value.begin(), 0);
      zeros++;
      temp_bigint.ubig_value = temp_value;
      // After single digit multiplication is done, store
      // it into a ubig, and add it into the final result
      new_bigint = new_bigint + temp_bigint;
      temp_value.clear();
   }
   new_bigint.removeZeros();
   return new_bigint;
}

void ubigint::multiply_by_2() {
   ubigint bigint_two {2}, n {0};
   n = (*this) * bigint_two;
   ubig_value = n.ubig_value;
}

/*
void ubigint::divide_by_2() {
   ubigint bigint_two {2}, incr {1}, quotient {0}, divisor {*this};
   while ((bigint_two < divisor) || (bigint_two == divisor)) {
      quotient = quotient + incr;
      divisor = divisor - bigint_two;
   }
   ubig_value = quotient.ubig_value;
}
*/

void ubigint::divide_by_2() {
   int i = ubig_value.size() - 1;
   int quotient_ = 0;
   ubigint divisor, quotient {0}, TWO {2};
   // Iterate while the divisor still have values.
   while (i >= 0) {
      // Get the divisor
      while (divisor < TWO && i >= 0) {
         //divisor.ubig_value.push_back(ubig_value[i]);
         divisor.ubig_value.insert(divisor.ubig_value.cbegin(), ubig_value[i]);
         i--;
      }
      // Get quotient
      while (divisor > TWO || divisor == TWO) {
         divisor = divisor - TWO;
         quotient_++;
      }
      quotient.ubig_value.insert(quotient.ubig_value.cbegin(), quotient_);
      quotient_ = 0;
   }
   quotient.removeZeros();
   ubig_value = quotient.ubig_value;
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   quotient.removeZeros();
   remainder.removeZeros();
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   int x = this->ubig_value.size(), y = that.ubig_value.size();
   if (x != y) return false;
   for (int i = 0; i < x; i++)
      if (this->ubig_value[i] != that.ubig_value[i]) return false;
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   int x = this->ubig_value.size(), y = that.ubig_value.size();
   if (x < y) return true;
   if (x > y) return false; 
   for (int i = x - 1; i >= 0; i--) {
      if (this->ubig_value[i] < that.ubig_value[i]) return true;
      if (this->ubig_value[i] > that.ubig_value[i]) return false;
   }
   return false;   
}

bool ubigint::operator> (const ubigint& that) const {
   int x = this->ubig_value.size(), y = that.ubig_value.size();
   if (x < y) return false;
   if (x > y) return true;
   for (int i = x - 1; i >= 0; i--) {
      if (this->ubig_value[i] < that.ubig_value[i]) return false;
      if (this->ubig_value[i] > that.ubig_value[i]) return true;
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   string expr = "";
   for (int i = that.ubig_value.size() - 1; i >= 0; i--)
      expr += to_string(that.ubig_value[i]);
   return out << expr;
}

void ubigint::operator= (const ubigint& that) {
   ubig_value = that.ubig_value;
}

void ubigint::removeZeros () {
   while (ubig_value.size() > 1 and ubig_value.back() == 0)
      ubig_value.pop_back();
}

