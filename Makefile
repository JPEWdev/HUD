#########  AVR Project Makefile Template   #########
######                                        ######
######    Copyright (C) 2003, Pat Deegan,     ######
######            Psychogenic Inc             ######
######          All Rights Reserved           ######
######                                        ######
###### You are free to use this code as part  ######
###### of your own applications provided      ######
###### you keep this copyright notice intact  ######
###### and acknowledge its authorship with    ######
###### the words:                             ######
######                                        ######
###### "Contains software by Pat Deegan of    ######
###### Psychogenic Inc (www.psychogenic.com)" ######
######                                        ######
###### If you use it as part of a web site    ######
###### please include a link to our site,     ######
###### http://electrons.psychogenic.com  or   ######
###### http://www.psychogenic.com             ######
######                                        ######
####################################################


##### This Makefile will make compiling Atmel AVR 
##### micro controller projects simple with Linux 
##### or other Unix workstations and the AVR-GCC 
##### tools.
#####
##### It supports C, C++ and Assembly source files.
#####
##### Customize the values as indicated below and :
##### make
##### make disasm 
##### make stats 
##### make hex
##### make writeflash
##### make gdbinit
##### or make clean
#####
##### See the http://electrons.psychogenic.com/ 
##### website for detailed instructions


####################################################
#####                                          #####
#####              Configuration               #####
#####                                          #####
##### Customize the values in this section for #####
##### your project. MCU, PROJECTNAME and       #####
##### PRJSRC must be setup for all projects,   #####
##### the remaining variables are only         #####
##### relevant to those needing additional     #####
##### include dirs or libraries and those      #####
##### who wish to use the avrdude programmer   #####
#####                                          #####
##### See http://electrons.psychogenic.com/    #####
##### for further details.                     #####
#####                                          #####
####################################################


#####         Target Specific Details          #####
#####     Customize these for your project     #####

# Name of target controller 
# (e.g. 'at90s8515', see the available avr-gcc mmcu 
# options for possible values)
MCU=atmega644p

# Name of our project
# (use a single word, e.g. 'myproject')
PROJECTNAME=hud

CLOCK_SPEED=10000000

AVRDUDE_SPEED=1.1

# Source files
# List C/C++/Assembly source files:
# (list all files to compile, e.g. 'a.c b.cpp as.S'):
# Use .cc, .cpp or .C suffix for C++ files, use .S 
# (NOT .s !!!) for assembly source code files.
PRJSRC= \
	src/uart.c \
	src/vfd.c \
	src/elm327.c \
	src/obd_data.c \
	src/hud_data.c \
	src/timer.c \
	src/led.c \
	src/btn.c \
	src/menu.c \
	src/queue.c \
	src/pin.c \
	src/main.c \
	src/diagnostics.c \
	src/layout.c 

#AVR_INCLUDES=/usr/local/AVR/avr/include

# additional includes (e.g. -I/path/to/mydir)
INC=-Isrc

OUTPUT_DIR=output

# libraries to link in (e.g. -lmylib)
LIBS= -Wl,-u,vfprintf -lprintf_flt

# Optimization level, 
# use s (size opt), 1, 2, 3 or 0 (off)
OPTLEVEL=s

#####      AVR Dude 'writeflash' options       #####
#####  If you are using the avrdude program
#####  (http://www.bsdhome.com/avrdude/) to write
#####  to the MCU, you can set the following config
#####  options and use 'make writeflash' to program
#####  the device.

# programmer id--check the avrdude for complete list
# of available opts.  These should include stk500,
# avr910, avrisp, bsd, pony and more.  Set this to
# one of the valid "-c PROGRAMMER-ID" values 
# described in the avrdude info page.
# 
AVRDUDE_PROGRAMMERID=avrispmkII

# port--serial or parallel port to which your 
# hardware programmer is attached
# 
AVRDUDE_PORT=usb


####################################################
#####                Config Done               #####
#####                                          #####
##### You shouldn't need to edit anything      #####
##### below to use the makefile but may wish   #####
##### to override a few of the flags           #####
##### nonetheless                              #####
#####                                          #####
####################################################


##### Flags ####

# HEXFORMAT -- format for .hex file output
HEXFORMAT=ihex

# compiler
CFLAGS=-I. $(INC) -g -mmcu=$(MCU) -O$(OPTLEVEL) \
    -DF_CPU=$(CLOCK_SPEED)UL				\
	-std=c99								\
    -fpack-struct -fshort-enums             \
	-funsigned-bitfields -funsigned-char    \
	-Wall -Werror -Wstrict-prototypes       \
	-Wa,-ahlms=$(OUTPUT_DIR)/$(firstword    \
	$(filter %.lst, $(<:.c=.lst)))

# c++ specific flags
CPPFLAGS=-fno-exceptions               		\
	-Wa,-ahlms=$(OUTPUT_DIR)/$(firstword    \
	$(filter %.lst, $(<:.cpp=.lst))		\
	$(filter %.lst, $(<:.cc=.lst)) 		\
	$(filter %.lst, $(<:.C=.lst)))

# assembler
ASMFLAGS =-I. $(INC) -mmcu=$(MCU)        		\
	-x assembler-with-cpp            		\
	-Wa,-gstabs,-ahlms=$(OUTPUT_DIR)/$(firstword	\
		$(<:.S=.lst) $(<.s=.lst))


# linker
LDFLAGS=-Wl,-Map,$(TRG).map -mmcu=$(MCU) \
	-lm $(LIBS) -O$(OPTLEVEL)

CTAGFLAGS= --c++-kinds=+p --fields=+iaS --extra=+q

##### executables ####
CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
AVRDUDE=avrdude
REMOVE=rm -f
CTAGS=ctags
SED=sed

##### automatic target names ####
TRG=$(OUTPUT_DIR)/$(PROJECTNAME).out
DUMPTRG=$(OUTPUT_DIR)/$(PROJECTNAME).s

HEXROMTRG=$(OUTPUT_DIR)/$(PROJECTNAME).hex 
HEXEETRG=$(OUTPUT_DIR)/$(PROJECTNAME).ee.hex
HEXTRG=$(HEXROMTRG) $(HEXEETRG)
GDBINITFILE=gdbinit-$(PROJECTNAME)

TAGS=tags
AVR_TAGS=avr_tags

# Define all object files.

# Start by splitting source files by type
#  C++
CPPFILES=$(filter %.cpp, $(PRJSRC))
CCFILES=$(filter %.cc, $(PRJSRC))
BIGCFILES=$(filter %.C, $(PRJSRC))
#  C
CFILES=$(filter %.c, $(PRJSRC))
#  Assembly
ASMFILES=$(filter %.S, $(PRJSRC))


# List all object files we need to create
OBJDEPS=$(addprefix $(OUTPUT_DIR)/,	\
	$(CFILES:.c=.o)    \
	$(CPPFILES:.cpp=.o)\
	$(BIGCFILES:.C=.o) \
	$(CCFILES:.cc=.o)  \
	$(ASMFILES:.S=.o) )

# Define all lst files.
LST=$(filter %.lst, $(OBJDEPS:.o=.lst))

# All the possible generated assembly 
# files (.s files)
GENASMFILES=$(filter %.s, $(OBJDEPS:.o=.s)) 


.SUFFIXES : .c .cc .cpp .C .o .out .s .S \
	.hex .ee.hex .h .hh .hpp


.PHONY: writeflash writefuses clean stats gdbinit stats $(TAGS)

# Make targets:
# all, disasm, stats, hex, writeflash/install, clean
all: $(TRG)

disasm: $(DUMPTRG) stats

stats: $(TRG)
	$(OBJDUMP) -h $(TRG)
	$(SIZE) $(TRG) 

hex: $(HEXTRG) 


writeflash: hex
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(MCU) -P $(AVRDUDE_PORT) -e        \
	 -U flash:w:$(HEXROMTRG)		\
	 -U eeprom:w:$(HEXEETRG)		\
	 -B $(AVRDUDE_SPEED)

# FUSES:
# Full Swing Oscillator; Start-up time 1K CK + 65 ms; Ceramic resonator; slowly rising power; [CKSEL=0111 SUT=00]
# No Clock output on PORTB1 [CKOUT=1]
# No Divide clock by 8 internally [CKDIV=1]
# No Boot Reset Vector Enabled [BOOTRST=1 BOOTSZ=00]
# No Preserve EEPROM through the Chip Erase cycle [EESAVE=1]
# No watchdog timer always on [WDTON=1]
# Serial program downloading enabled [SPIEN=0]
# JTAG Interface Enabled [JTAGEN=1]
# On Chip Debug Enabled [OCDEN=1]
# Brown-out Detection level at VCC=4.3 V [BODLEVEL=100]
#
#	N = Unprogrammed(1)
#	Y = Programmed(0)
#
# BIT	LOW			HIGH		EXTENDED
# 7		CKDIV=N		OCDEN=N
# 6		CKOUT=N		JTAGEN=N
# 5		SUT1=N		SPIEN=Y
# 4		SUT0=N		WDTON=N
# 3		CKSEL3=N	EESAVE=N
# 2		CKSEL2=N	BOOTSZ1=Y	BODLEVEL2=N
# 1		CKSEL1=N	BOOTSZ0=Y	BODLEVEL1=Y
# 0		CKSEL0=N	BOOTRST=N	BODLEVEL0=Y
#
# http://www.engbedded.com/fusecalc/

# NOTE: We write the fuses at a slower speed because on
# new devices, the external clock is not configured yet so
# the high speed transfer fails
writefuses:
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(MCU) -P $(AVRDUDE_PORT) \
	 -B 100 \
	 -U lfuse:w:0xff:m \
	 -U hfuse:w:0xd9:m \
	 -U efuse:w:0xfc:m

readfuses:
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(MCU) -P $(AVRDUDE_PORT) \
	 -B $(AVRDUDE_SPEED) \
	 -U lfuse:r:-:h \
	 -U hfuse:r:-:h \
	 -U efuse:r:-:h

readeeprom:
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMERID)   \
	 -p $(MCU) -P $(AVRDUDE_PORT) \
	 -B $(AVRDUDE_SPEED) \
	 -U eeprom:r:eeprom.hex:i

install: writeflash

$(DUMPTRG): $(TRG) 
	$(OBJDUMP) -S  $< > $@


$(TRG): $(OBJDEPS) 
	$(CC) $(LDFLAGS) -o $(TRG) $(OBJDEPS)

$(TAGS) :
	touch tags_c_temp.c
	$(CC) $(CFLAGS) -dM -E tags_c_temp.c > tags.def
	$(SED) -i -e"s/^#define\s*\(\w\+\)\s*\(.*\)/\1=\2/" tags.def
	$(CTAGS) -f $(TAGS) $(CTAGFLAGS) -I @tags.def -R .
	rm tags_c_temp.c
	rm tags.def

$(AVR_TAGS) :
	touch tags_c_temp.c
	$(CC) $(CFLAGS) -dM -E tags_c_temp.c > tags.def
	$(SED) -i -e"s/^#define\s*\(\w\+\)\s*\(.*\)/\1=\2/" tags.def
	$(CTAGS) -f $(AVR_TAGS) $(CTAGFLAGS) -I @tags.def -R $(AVR_INCLUDES)
	rm tags_c_temp.c
	rm tags.def

#### Generating assembly ####
# asm from C
%.s: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) -S $(CFLAGS) $< -o $@

# asm from (hand coded) asm
%.s: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) -S $(ASMFLAGS) $< > $@


# asm from C++
.cpp.s .cc.s .C.s :
	@mkdir -p $(shell dirname $@)
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $< -o $@


#### Generating object files ####
# object from C
$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@


# object from C++ (.cc, .cpp, .C files)
#.cc.o .cpp.o .C.o :
#	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# object from asm
$(OUTPUT_DIR)/%.o : %.S
	@mkdir -p $(shell dirname $@)
	$(CC) $(ASMFLAGS) -c $< -o $@


#### Generating hex files ####
# hex files from elf
#####  Generating a gdb initialisation file    #####
.out.hex:
	$(OBJCOPY) -j .text                    \
		-j .data                       \
		-O $(HEXFORMAT) $< $@

.out.ee.hex:
	$(OBJCOPY) -j .eeprom                  \
		--change-section-lma .eeprom=0 \
		-O $(HEXFORMAT) $< $@


#####  Generating a gdb initialisation file    #####
##### Use by launching simulavr and avr-gdb:   #####
#####   avr-gdb -x gdbinit-myproject           #####
gdbinit: $(GDBINITFILE)

$(GDBINITFILE): $(TRG)
	@echo "file $(TRG)" > $(GDBINITFILE)
	
	@echo "target remote localhost:1212" \
		                >> $(GDBINITFILE)
	
	@echo "load"        >> $(GDBINITFILE) 
	@echo "break main"  >> $(GDBINITFILE)
	@echo "continue"    >> $(GDBINITFILE)
	@echo
	@echo "Use 'avr-gdb -x $(GDBINITFILE)'"


#### Cleanup ####
clean:
	$(REMOVE) $(TRG) $(TRG).map $(DUMPTRG)
	$(REMOVE) $(OBJDEPS)
	$(REMOVE) $(LST) $(GDBINITFILE)
	$(REMOVE) $(GENASMFILES)
	$(REMOVE) $(HEXTRG)
	
#####                    EOF                   #####
