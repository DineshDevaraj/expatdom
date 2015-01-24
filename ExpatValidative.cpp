
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

#include <sstream>
#include <errno.h>
#include <expat.h>

#include "ExpatValidative.h"

static Node_t oInvalidNode;

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

Environ_t::Environ_t(Doc_t *doc)
{
   level = 0;
   this->doc = doc;
   root = cur = new Node_t;
}

Environ_t::~Environ_t()
{
   Node_t *first = NULL;
   first = root->first;
   *root = *first;
   delete first;
}

static void start(void *arg, const char *name, const char *attrs[])
{
   Environ_t *env = (Environ_t *)arg;
   env->level++;

   Node_t *parent = env->cur;
   parent->childCount++;

   Node_t *cur = new Node_t;

   if(parent->last)
   {
      parent->last->next = cur;
      cur->prev = parent->last;
   }
   else /* first child */
   {
      parent->first = cur;
      parent->last = cur;
      cur->prev = 0;
   }
  
   int I = 0;
   for( ; attrs[I]; I+=2)
   {
      Attr_t *attr = new Attr_t;

       /* first attr */
      if(0 == cur->attr)
         cur->attr = attr;
      else
         cur->attr->next = attr;

      attr->key = attrs[I];
      attr->val = attrs[I + 1];
   }
   cur->attrCount = I / 2;
  
   cur->depth = parent->childCount;
   cur->level = env->level;

   cur->root = env->root;
   cur->parent = parent;
   cur->doc = env->doc;
   cur->key = name;
   
   parent->last = cur;
   env->cur = cur;

   return;
}

static void value(void *arg, const char *val, int len)
{
   Environ_t *env = (Environ_t *)arg;

   int I;
   for(I = 0; I < len; I++)
      if(not spcch(val[I]))
         break;
   if(I == len) return;
   
   len -= I;
   val += I;

   if(env->cur->len)
      env->cur->val.append(" ");

   env->cur->val.append(val, len);
   env->cur->len += len;

   return;
}

static void end(void *arg, const char *name)
{
   Environ_t *env = (Environ_t *)arg;
   env->cur = env->cur->parent;
   env->level--;
   return;
}

static void SetError(XML_Parser parser, Error_t &err)
{
   err.code = XML_GetErrorCode(parser);
   err.line = XML_GetCurrentLineNumber(parser);
   err.column = XML_GetCurrentColumnNumber(parser);
   err.offset = XML_GetCurrentByteIndex(parser);
   err.msg = (char *)XML_ErrorString((XML_Error)err.code);
   return;
}

Node_t * ExpatParseFile(Doc_t *doc, const char *path)
{
   Environ_t env(doc);

   Error_t &err = doc->error;

   FILE *fh = fopen(path, "r");
   if(0 == fh)
   {
      err.code = 1;
      err.msg = strerror(errno);
      return NULL;
   }

   XML_Parser parser = XML_ParserCreate(NULL);
   if(0 == parser)
   {
      fclose(fh);
      err.code = 2;
      err.msg = "Unable to create parser";
      return NULL;
   }

   XML_SetUserData(parser, (void *)&env);
   XML_SetElementHandler(parser, start, end);
   XML_SetCharacterDataHandler(parser, value);

   int len = 0;
   char buf[1024] = {};

   while(len = fread(buf, sizeof(char), sizeof buf, fh))
   {
      if(0 == XML_Parse(parser, buf, len, len < 1024))
      {
         SetError(parser, err);
         return NULL;
      }
   }

   XML_ParserFree(parser);
   fclose(fh);

   return env.root;
}

Node_t * ExpatParseString(Doc_t *doc, const char *xmlStr)
{
   Environ_t env(doc);

   Error_t &err = doc->error;

   XML_Parser parser = XML_ParserCreate(NULL);
   if(0 == parser)
   {
      err.code = 2;
      err.msg = "Unable to create parser";
      return NULL;
   }

   XML_SetUserData(parser, (void *)&env);
   XML_SetElementHandler(parser, start, end);
   XML_SetCharacterDataHandler(parser, value);

   if(0 == XML_Parse(parser, xmlStr, strlen(xmlStr), 1))
   {
      SetError(parser, err);
      return NULL;
   }

   XML_ParserFree(parser);
   return env.root;
}

Doc_t::Doc_t()
{
   root = NULL;
   path.clear();
}

Attr_t::Attr_t()
{
   next = NULL;
   prev = NULL;
   parent = NULL;
}

Error_t::Error_t()
{
   code = 0;
   line = 0;
   column = 0;
   offset = 0;
}

Node_t::Node_t()
{
   len = 0;
   level = 0;
   depth = 0;
   attrCount = 0;
   childCount = 0;

   doc = NULL;
   attr = NULL;
   root = NULL;

   next = NULL;
   prev = NULL;
   first = NULL;
   last = NULL;
   parent = NULL;
}

struct ErrorType
{
   enum Value
   {
      NONE = 1,
      EXPECTED_NUMBER,
      MISSING_CLOSE_BRACKET
   };
};

typedef ErrorType::Value ErrorType_t;

Node_t & Doc_t::ParseFile(const char *path)
{
   this->path = path;
   Node_t *root = NULL;
   root = ExpatParseFile(this, path);
   if(root) return *root;
   return oInvalidNode;
}

Node_t & Doc_t::ParseString(const char *xmlStr)
{
   Node_t *root = NULL;
   root = ExpatParseString(this, xmlStr);
   if(root) return *root;
   return oInvalidNode;
}

static void PrintNode(Node_t *node, int lev, FILE *fh)
{
   if(1 == lev)
      fprintf(fh, "%s ", node->key.data());
   else
      fprintf(fh, "%*c%s ", 3 * (lev - 1), ' ', node->key.data());
   for(Attr_t *attr = node->attr; attr; attr = attr->next)
      fprintf(fh, "%s='%s' ", attr->key.data(), attr->val.data());
   if(node->len)
      fprintf(fh, "%s ", node->val.data());
   fprintf(fh, "[%d, %d, %d, %d]", node->level, node->depth, node->attrCount, node->childCount);
   fprintf(fh, "\n");
}

static void XmlPrint(Node_t *node, int lev, FILE *fh)
{
   while(node)
   {
      PrintNode(node, lev, fh);
      XmlPrint(node->first, lev + 1, fh);
      node = node->next;
   }
   return;
}

void Node_t::Print(FILE *fh)
{
   PrintNode(this, 1, fh);
   XmlPrint(first, 2, fh);
}

/**
 *
 * get entinties 
 *
 * @args xpath : 
 * @args tl    : tag length
 * @args in    : index if exist of 0
 * @args dl    : delimeter location
 * @args el    : error location
 *
 **/
static ErrorType_t getent(const char *xpath, int &tl, int &in, const char * &dl, const char * &el)
{
   char ch;
   const char *itr = 0;
   tl = 0; in = 0; dl = 0;
   
   for(itr = xpath; ch = *itr; itr++)
   {
      if('[' == ch)
      {
         if(not isdigit(itr[1]))
         {
            el = itr + 1;
            return ErrorType::EXPECTED_NUMBER;
         }

         const char *idx = NULL;
         for(idx = itr + 1; *idx != ']'; idx++)
         {
            if(not isdigit(*idx))
            {
               el = idx;
               return ErrorType::MISSING_CLOSE_BRACKET;
            }
         }

         tl = itr - xpath;
         in = atoi(itr + 1);
         itr = idx;
      }
      else if('/' == ch)
      {
         dl = itr;
         if(0 == tl) 
            tl = itr - xpath;
         return ErrorType::NONE;
      }
   }
   
   if(0 == tl) 
      tl = strlen(xpath);

   return ErrorType::NONE;
}

static bool cmplen(const char *lval, const char *rval, int len)
{
   while(*lval && * rval)
   {
      if(*lval++ != *rval++)
         return false;
      len--;
   }
   return !len;
}

static Node_t * XPath(Node_t *node, const char *base, const char *xpath)
{
   int tl = 0;
   int in = 0;
   int nl = 0; /* name length */

   const char *dl = NULL;
   const char *el = NULL;

   ErrorType_t ret;
   std::stringstream str;
   Error_t &err = node->doc->error;

   ret = getent(xpath, tl, in, dl, el);
   
   if(ErrorType::EXPECTED_NUMBER == ret)
   {
      str << "Expected number at location "  << 
              el - base + 1 << " in given XPATH";
      err.msg = str.str();
      return NULL;
   }
   else if(ErrorType::MISSING_CLOSE_BRACKET == ret)
   {
      str << "Close bracket missing at location " << 
              el - base + 1 << " in given XPATH";
      err.msg = str.str();
      return NULL;
   }

   for( ; node; node = node->next)
   {
      if(node->key.length() == tl and
            cmplen(node->key.data(), xpath, tl))
      {
         if(0 == in--)
         {
            if(dl) node = XPath(node->first, base, dl + 1);
            return node;
         }
      }
   }

   if(NULL == node) err.msg = "Given XPATH does not exist";

   return node;
}

Node_t & Node_t::XPath(const char *xpath)
{
   if(0 == xpath && 0 == *xpath)
   {
      doc->error.msg = "Empty XPATH received";
      return oInvalidNode;
   }
   else if(0 == first)
   {
      doc->error.msg = "Node does not has any child";
      return oInvalidNode;
   }

   Node_t *node;
   if('/' != *xpath)
      node = ::XPath(first, xpath, xpath);      
   else 
      node = ::XPath(first->root, xpath + 1, xpath + 1);
      
   if(node) return *node;
   return oInvalidNode;
}

Node_t & Node_t::operator [] (const char *xpath)
{
   return XPath(xpath);
}

bool Node_t::valid()
{
   return (this != &oInvalidNode);
}

static void XmlFree(Node_t *node)
{
   if(0 == node) 
      return;
   while(node)
   {
      Attr_t *attr = node->attr;
      Attr_t *next = NULL;
      while(attr)
      {
         next = attr->next;
         delete attr;
         attr = next;
      }
      XmlFree(node->first);
      Node_t *cur = node;
      node = cur->next;
      delete cur;
   }
   return;
}

Doc_t::~Doc_t()
{
   XmlFree(root);
}
