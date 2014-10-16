#/*
# *                   ##
# *                  ##             cafe (CaFE) : Calcurator, Function Expandable
# *                  ##             
# *   ###   ####   ######  ###       an RPN calculator for command line interface
# * ##     ##  ##    ##   ## ##
# * ##     ##  ##    ##   ###             (c) Tsukimidai Communications Syndicate
# *   ###   ### ##   ##    ####               1991 - 2008
# */
#
#/*
#***   version 2.0 release 0.5
#**/


##########
###  settings for install
##########

# set install target "PUBLIC" or "PRIVATE"
INSTALL_TARGET			= PUBLIC

# FOR PUBLIC (SYSTEM) INSTALL
PUBLIC_INSTALL_DIR		= /usr/local/bin
PUBLIC_STDLIB_DIR		= /etc

# FOR PRIVATE (USER-ONLY) INSTALL
PRIVATE_INSTALL_DIR		= ~/bin
PRIVATE_STDLIB_DIR		= ~/etc

INSTALL_DIR				= $($(INSTALL_TARGET)_INSTALL_DIR)
STDLIB_DIR				= $($(INSTALL_TARGET)_STDLIB_DIR)


##########
###  build settings
##########

STDLIB_FILE				= cafe.stdlib
STDLIB_LOCATION			= $(STDLIB_DIR)/$(STDLIB_FILE)

CAFELIB					= library
STDLIB_SRC				= $(CAFELIB)/$(STDLIB_FILE)

SHELL		= /bin/sh

CC			= gcc
CFLAGS		= -O3 -Wall
DPENDFLAGS	= -Y 


DATETIME	= $(shell date +%Y%m%d-%H%M.%S)
BUILDFOR	= $(MACHTYPE)-$(VENDOR)-$(OSTYPE)

BUILD_INFO	= ui_buildinfo


SRCS	=						\
			fundamentals.c		\
			command_table.c		\
			stack.c				\
			ui.c				\
			functions.c			\
			controls.c			\
			fileop.c			\
			key.c				\
			string.c			\
			debugger.c			\
#			ui_buildinfo.c		\

BUILD_INFO_SRC	= $(BUILD_INFO).c

			
OBJS	=						\
			fundamentals.o		\
			stack.o				\
			command_table.o		\
			ui.o				\
			functions.o			\
			controls.o			\
			fileop.o			\
			key.o				\
			string.o			\
			debugger.o			\
#			ui_buildinfo.o		\

BUILD_INFO_OBJ	= $(BUILD_INFO).o

all : packinfo cafe $(STDLIB_SRC)

install : install_executable install_stdlib 

install_stdlib : $(STDLIB_SRC)
	@if [ ! -e $(STDLIB_DIR) ] ; then mkdir -p $(STDLIB_DIR) ; fi;
	@[ -d $(STDLIB_DIR) ]  # catch an error if the target is not a directory
	@if [ $(STDLIB_SRC) -nt $(STDLIB_LOCATION) ] ; then cp $(STDLIB_SRC) $(STDLIB_LOCATION) ; echo "executable copied to $(STDLIB_DIR)"; fi
	
install_executable : cafe packinfo
	@if [ ! -e $(INSTALL_DIR) ] ; then mkdir -p $(INSTALL_DIR) ; fi;
	@[ -d $(INSTALL_DIR) ]  # catch an error if the target is not a directory
	@if [ cafe -nt $(INSTALL_DIR)/cafe ] ; then cp cafe $(INSTALL_DIR)/; echo "executable copied to $(INSTALL_DIR)" ; fi;
	
uninstall : 
	sudo rm $(INSTALL_DIR)/cafe $(STDLIB_LOCATION)

$(STDLIB_SRC) : $(STDLIB_SRC).base
	sed s#FILENAME_TO_REPLACE#$(STDLIB_LOCATION)# $(STDLIB_SRC).base > $(STDLIB_SRC)

cafe : $(OBJS) $(BUILD_INFO_OBJ)
	$(CC) -o $@ $(OBJS) $(BUILD_INFO_OBJ) -lncurses -lm;

fundamentals.o : fundamentals.c
	$(CC) $(CFLAGS) -DSTDLIB_LOCATION=\"$(STDLIB_LOCATION)\" -o $@ -c $<

$(BUILD_INFO_OBJ) : $(SRCS) $(BUILD_INFO_SRC) ui.h
	$(CC) $(CFLAGS) -DBUILDNUM=\"$(shell cat packinfo)\" -DBUILDFOR=\"$(BUILDFOR)\" -DSTDLIB_LOCATION=\"$(STDLIB_LOCATION)\" -o $@ -c $(BUILD_INFO_SRC)
	
.c.o : 
	$(CC) $(CFLAGS) -o $@ -c $<

packinfo : $(SRCS) $(BUILD_INFO_SRC)
	@if ! grep modified ./packinfo > /dev/null ; then echo '$(shell cat packinfo)\(modified\)' > packinfo ; echo "packinfo updated"; fi ;

pack : 
	- make clean ; \ 
	cp ~/.cafe samples/dot_cafe.$(USER)-$(HOST) ; \
	echo $(DATETIME) > packinfo ; \
	cd .. ; \
	tar jcvf cafe\($(DATETIME)\).tar.bz2 CaFE2/ ; \
	cd CaFE2/ ;
	
clean : 
	- rm -rf *.o *.bak $(STDLIB_SRC) build cafe
	
depend : 
	makedepend $(DPENDFLAGS) -- $(CFLAGS) -- $(SRCS) 2> /dev/null ;


# DO NOT DELETE

fundamentals.o: cafe2.h command_table.h functions.h stack.h key.h ui.h
fundamentals.o: fileop.h debugger.h string.h
command_table.o: cafe2.h string.h command_table.h functions.h stack.h
command_table.o: controls.h fileop.h ui.h key.h debugger.h
stack.o: cafe2.h string.h stack.h ui.h
ui.o: cafe2.h string.h ui.h stack.h
functions.o: cafe2.h string.h functions.h key.h command_table.h fileop.h
functions.o: stack.h debugger.h ui.h
controls.o: cafe2.h string.h stack.h functions.h controls.h fileop.h
controls.o: debugger.h ui.h command_table.h
fileop.o: cafe2.h string.h fileop.h functions.h key.h ui.h stack.h controls.h
key.o: cafe2.h string.h key.h ui.h stack.h fileop.h command_table.h
key.o: functions.h
string.o: cafe2.h string.h stack.h ui.h
debugger.o: cafe2.h debugger.h ui.h stack.h key.h
