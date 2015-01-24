
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

#include <stdio.h>
#include <sys/stat.h>

#include "ExpatValidative.h"

#define OK   0
#define ERR -1

int main(int argc, char *argv[])
{
   Doc_t xml;

   if(argc < 3)
   {
      fprintf(stderr, "usage : "
            "%s xml_file_path "
            "file_or_str (f/s) "
            "[xpath]\n", argv[0]);
      return ERR;
   }

   Node_t *pRoot = NULL;
   bool file = ('f' == argv[2][0]);

   if(file)
   {
      pRoot = &(xml.ParseFile(argv[1]));
   }
   else
   {
      struct stat st;
      char *xmlStr = NULL;
      FILE *fh = fopen(argv[1], "r");

      stat(argv[1], &st);
      xmlStr = new char[st.st_size];
      fread(xmlStr, st.st_size, sizeof(char), fh);

      pRoot = &(xml.ParseString(xmlStr));

      delete xmlStr;
      fclose(fh);
   }
   
   Node_t &root = *pRoot;

   if(not root.valid())
   {
      Error_t &err = xml.error;
      fprintf(stderr, "msg %s line %d column %d offset %d\n", \
            err.msg.data(), err.line, err.column, err.offset);
      return ERR;
   }

   if(argc < 4)
   {
      root.Print(stderr);
   }
   else
   {
      Node_t & node = root[argv[3]];
      if(node.valid()) node.Print(); /* default stdout */
      else printf("%s\n", xml.error.msg.data());
   }

   return OK;
}
