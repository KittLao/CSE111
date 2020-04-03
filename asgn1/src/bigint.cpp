// $Id: bigint.cpp,v 1.78 2019-04-03 16:44:33-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <iostream>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}
/* My code begins */
bigint bigint::operator+ (const bigint& that) const {
   bigint x {*this}, y {that}, result;
   if (!x.is_negative && !y.is_negative) { // (+, +)
      result.uvalue = x.uvalue + y.uvalue;
      result.is_negative = false;
   } else if (!x.is_negative && y.is_negative) { // (+, -)
      y.is_negative = false;
      result = x - y;
   } else if (x.is_negative && !y.is_negative) { // (-, +)
      x.is_negative = false;
      result = y - x;
   } else { // (-, -)
      x.is_negative = false;
      y.is_negative = false;
      result = x + y;
      result.is_negative = true;
   }
   return result;
}

bigint bigint::operator- (const bigint& that) const {
   bigint x {*this}, y {that}, result;
   if (!x.is_negative && !y.is_negative) { // (+, +)
      if (x.uvalue >= y.uvalue) {
         result.uvalue = x.uvalue - y.uvalue;
         result.is_negative = false;
      } else {
         result.uvalue = y.uvalue - x.uvalue;
         result.is_negative = true;
      }
   } else if (!x.is_negative && y.is_negative) { // (+, -)
      y.is_negative = false;
      result = x + y;
      result.is_negative = false;
   } else if (x.is_negative && !y.is_negative) { // (-, +)
      x.is_negative = false;
      result = x + y;
      result.is_negative = true;
   } else { // (-, -)
      x.is_negative = false;
      y.is_negative = false;
      result = x - y;
      result.is_negative = !result.is_negative;
     
   }
   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint x {*this}, y {that}, result;
   result.uvalue = x.uvalue * y.uvalue;
   result.is_negative = (x.is_negative && y.is_negative) || (!x.is_negative && !y.is_negative) ? false : true;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint x {*this}, y {that}, result;
   result.uvalue = x.uvalue / y.uvalue;
   result.is_negative =	(x.is_negative && y.is_negative) || (!x.is_negative && !y.is_negative) ? false : true;
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint x {*this}, y {that}, result;
   result.uvalue = x.uvalue % y.uvalue;
   result.is_negative = (x.is_negative && y.is_negative) || (!x.is_negative && !y.is_negative) ? false : true;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

void bigint::operator= (const bigint& that) {
   uvalue = that.uvalue;
   is_negative = that.is_negative;
}
/* My code ends */

ostream& operator<< (ostream& out, const bigint& that) {
   return out <<  (that.is_negative ? "-" : "")
              <<  that.uvalue;
}
