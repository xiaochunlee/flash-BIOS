INC=  
LIB= 

CC = cc
##CC_FLAG=-Wall 
CC_FLAG=-c -Wall 
#PRG=authenticate.o
OBJ=secureflash.o rsa.o sha1.o string.o
all:$(OBJ)
#	$(CC) -c   -o $@ $(OBJ)  $(LIB) 
#	$(CC) $(CC_FLAG) $(INC) $^ -o $@ 
#	mv *.o ../
debug:$(OBJ)
	$(CC)  -Wall -o  $@ $(OBJ)  $(LIB) 
	
#.SUFFIXES: .c .o .c 
#.c.o:  
#	$(CC) $(CC_FLAG) $(INC) -c $*.c -o $*.o 

.PRONY:clean
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f $(OBJ) $(PRG) 
