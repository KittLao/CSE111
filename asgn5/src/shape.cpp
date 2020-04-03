// $Id: shape.cpp,v 1.2 2019-02-28 15:24:20-08 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (const string& glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(fontcode.at(glut_bitmap_font_)), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
            polygon({{-width/2, -height/2}, {width/2, -height/2}, {width/2, height/2}, {-width/2, height/2}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (GLfloat width, GLfloat height):
            polygon({{0, height/2}, {-width/2, 0}, {0, -height/2}, {width/2, 0}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

triangle::triangle (const vertex v1, const vertex v2, const vertex v3):
            polygon({v1, v2, v3}) {
}

equilateral::equilateral (GLfloat width):
            triangle({-width/2, -width*sqrt(3)/6}, {width/2, -width*sqrt(3)/6}, {0, width*sqrt(3)/3}) {
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   const GLubyte* text = reinterpret_cast<const GLubyte*> (textdata.c_str());
   void* font = fontcode.at("Times-Roman-10");
   int width = glutBitmapLength(font, text);
   int height = glutBitmapHeight(font);
   glColor3ubv(color.ubvec);
   glRasterPos2f(center.xpos, center.ypos);
   glutBitmapString(font, text);
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glLineWidth(4);
   /* Draw ellipse */
   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);
   const GLfloat delta = 2 * M_PI / 64;
   GLfloat w = dimension.xpos / 2.5;
   GLfloat h = dimension.ypos / 2.5;
   for(GLfloat theta = 0; theta < 2 * M_PI; theta += delta) {
      GLfloat xpos = w * cos (theta) + center.xpos;
      GLfloat ypos = h * sin (theta) + center.ypos;
      glVertex2f (xpos, ypos);
   }
   glEnd();

   /* Draw ellipse border */
   glBegin(GL_LINE_LOOP);
   GLubyte red[] = {255, 0, 0};
   glColor3ubv(red);
   for(GLfloat theta = 0; theta < 2 * M_PI; theta += delta) {
      GLfloat xpos = w * cos (theta) + center.xpos;
      GLfloat ypos = h * sin (theta) + center.ypos;
      glVertex2f (xpos, ypos);
   }
   glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glLineWidth(4);
   /* Draw polygon */
   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);
   for(vertex vertice : vertices)
      glVertex2f(vertice.xpos + center.xpos, vertice.ypos + center.ypos);
   glEnd();

   /* Draw polygon border */
   glBegin(GL_LINE_LOOP);
   GLubyte red[] = {255, 0, 0};
   glColor3ubv(red);
   for(vertex vertice : vertices)
      glVertex2f(vertice.xpos + center.xpos, vertice.ypos + center.ypos);
   glEnd();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}
