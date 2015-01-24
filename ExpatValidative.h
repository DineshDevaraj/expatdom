
/**
*
* Author : D.Dinesh
* Website : www.techybook.com
* Email Id : dinesh@techybook.com
*
* Created : 17 Dec 2014 - Wed
* Updated : 24 Jan 2015 - Sat
*
* Licence : Refer the license file
*
**/

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

typedef std::string Str;

/* forward declaration */
struct Doc_t;
struct Node_t;
struct Attr_t;

struct Error_t
{
   Error_t();

   Str msg;
   int code;
   int line;
   int column;
   int offset;
};

struct Doc_t
{
   Doc_t();

   Str path;

   Node_t *root;
   Error_t error;

   Node_t & ParseFile(const char *path);
   Node_t & ParseString(const char *xml);

   ~Doc_t();

   /* to ensure doc is always static */
   private : void * operator new(size_t size);
};

struct Node_t
{
   Node_t();

   Str key;
   Str val;
   int len;

   int level;
   int depth;

   int attrCount;
   int childCount;

   Attr_t *attr;

   Doc_t *doc;
   Node_t *root;

   Node_t *next;
   Node_t *prev;
   Node_t *parent;
   Node_t *first;    /* first child */
   Node_t *last;     /* last child */
   
   bool valid();

   void Print(FILE *fh = stdout);
   Node_t & XPath(const char *xpath);
   Node_t & operator [] (const char *xpath);
};

struct Attr_t
{
   Attr_t();

   Str key;
   Str val;

   Attr_t *next;
   Attr_t *prev;

   Node_t *parent;
};

#define spcch(ch) (' ' == ch || '\t' == ch || '\r' == ch || '\n' == ch)

struct Environ_t
{
   Environ_t(Doc_t *doc);

   int level;
   Doc_t *doc;
   Node_t *cur;
   Node_t *root;

   ~Environ_t();
};

struct Content
{
   enum Value
   {
      XmlStr,
      FilePath
   };
};

typedef Content::Value Content_t;
