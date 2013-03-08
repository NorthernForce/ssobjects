#********************************************************************
#        Copyright (c) 2006, Lee Patterson & Ant Works Software
#        http://ssobjects.sourceforge.net
#
#        created  :    July 4, 2001
#        filename :    Makefile
#        author   :    Lee Patterson (workerant@users.sourceforge.net)
#        
#        purpose  :    Make file for Linux operating system.
#*********************************************************************

#when adding a new source file, you have to rebuild the dependancy
#easiest way is to 'make clean all' to rebuild everything,
#or 'rm makefile.dep'

TERM    := dumb

SERVER  := server
SERVER_DBG  := server_dbg

#
# source files used for project(s)
#
SERVER_SRCS := \
	server2.cpp \
	timed.cpp

#
#-----------------------------------------------------
#

ALL_SRCS := $(SERVER_SRCS)

#
# Convert the source files into object files
#
SERVER_OBJS 	:= $(SERVER_SRCS:.cpp=.o)
SERVER_OBJS_DBG := $(SERVER_SRCS:.cpp=_dbg.o)

#
# Include and library paths used in source
#
INCLUDES := -I../.. -I. -I/home/pi/OpenCV-2.4.3/release//include/opencv -I/home/pi/OpenCV-2.4.3/release//include  /home/pi/OpenCV-2.4.3/release//lib/libopencv_calib3d.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_contrib.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_core.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_features2d.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_flann.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_gpu.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_highgui.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_imgproc.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_legacy.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_ml.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_nonfree.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_objdetect.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_photo.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_stitching.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_ts.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_video.so /home/pi/OpenCV-2.4.3/release//lib/libopencv_videostab.so  

LIBS_DBG := -L../.. -lssobjects_dbg -lpthread $(INCLUDES)
LIBS     := -L../.. -lssobjects     -lpthread $(INCLUDES)

#
# Build options
#
#WARNINGS = -W -Wall -Woverloaded-virtual -Weffc++
CCFLAGS_DBG := $(INCLUDES) $(WARNINGS) -g -fPIC  -D_REENTRANT -DDEBUG  -DUSE_NCURSES
CCFLAGS     := $(INCLUDES) $(WARNINGS) -fPIC -O2 -D_REENTRANT -DNDEBUG -DUSE_NCURSES

#
# Compiler used
#
CC := g++

#
# Targets
#
none         : debug
all2         : debug release
release      : tags $(SERVER)      
debug        : tags $(SERVER_DBG) 
makefile.dep : depend

#
# Build commands
#
%.o : %.cpp
	@echo "Compiling release $<"
	@$(CC) -c $(CCFLAGS) $< -o $@

%_dbg.o : %.cpp
	@echo "Compiling debug $<"
	@$(CC) -c $(CCFLAGS_DBG) $< -o $@

$(SERVER): $(SERVER_OBJS)
	@echo "*** Building release $(SERVER)"
	@$(CC) -o $(SERVER) $(SERVER_OBJS) $(LIBS) -lpthread

$(SERVER_DBG): $(SERVER_OBJS_DBG)
	@echo "*** Building debug $(SERVER_DBG)"
	@$(CC) -o $(SERVER_DBG) $(SERVER_OBJS_DBG) $(LIBS_DBG) -lpthread

tidy :
	@echo "Removing object files"
	@rm -f *.o

clean : 
	@echo "Removing all intermediate files"
	@rm -f  $(SERVER) $(SERVER_OBJS) $(SERVER_DBG) $(SERVER_OBJS_DBG) \
		core makefile.dep TAGS *.log *~

../../ssobjects.a:
	cd ../..; $(MAKE) release

../../ssobjects_dbg.a:
	cd ../..; $(MAKE) debug

tags : 
	@echo Started build at:
	@date
	@echo "Building tags file"
	@etags *.cpp

depend: 
	@if [ ! -f makefile.dep ]; then \
	echo "Building dependancies"; \
	rm -f makefile.dep; \
	for i in $(ALL_SRCS) ; \
	do \
	$(CC) -DUSE_NCURSES $(INCLUDES) -MM -c $$i >> makefile.dep ; \
	$(CC) -DUSE_NCURSES $(INCLUDES) -MM -c $$i | sed 's/\.o/_dbg\.o/' >> makefile.dep ; \
	done; \
	echo "$(SERVER): ../../ssobjects.a" >> makefile.dep; \
	echo "$(SERVER_DBG): ../../ssobjects_dbg.a" >> makefile.dep; \
	fi

-include makefile.dep
