
#
# Author : D.Dinesh
# Website : www.techybook.com
# Email Id : dinesh@techybook.com
#
# Created : 17 Dec 2014 - Wed
# Updated : 24 Jan 2015 - Sat
#
# Licence : Refer the license file
#

FLAG=-g
GPP=/usr/bin/g++
LIBS=-lexpat

SRC=demo.cpp \
	 ExpatValidative.cpp

demo.x : $(SRC)
	$(GPP) $(FLAG) $(INC) $(SRC) $(LIBS) -o $@ 
