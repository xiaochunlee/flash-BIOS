DEBUG=

ifeq ($(DEBUG),y)
    DEBUG_FLAGS= -O -g -D__DEBUG__
else
    DEBUG_FLAGS= 
endif

CC = cc
CC_FLAG=-Wall -lpthread 
LIB_FLAG= -fPIC -shared -lpthread

CC_FLAG+=$(DEBUG_FLAGS)

PRG= flashbios
PRG_LIB= libflash.so

OBJ=main.o basefunc.o data.o flashopt.o smbios.o
SUBDIR=authenticate
SUBOBJ=$(SUBDIR)/*.o 

all:$(PRG) $(PRG_LIB)

$(PRG):$(SUBOBJ) $(OBJ) 
	$(CC) $(CC_FLAG)  -o $@ $(OBJ) $(SUBOBJ) 

$(PRG_LIB):$(SUBOBJ) $(OBJ)
	$(CC) $(LIB_FLAG)  -o $@ $(OBJ) $(SUBOBJ) 
	#cp $(PRG_LIB) /usr/lib/ -rf
	
$(SUBOBJ):
	$(MAKE) -C $(SUBDIR) 
	
      
.SUFFIXES: .c .o .c 
.c.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.c -o $*.o 

.PRONY:clean
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f $(OBJ) $(SUBOBJ) $(PRG) $(PRG_LIB) 
