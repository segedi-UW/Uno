CFLAGS= -Wall -g -pthread
LIBS=cards.c agLinkedList.c agUtil.c agLLIterator.c agLLNode.c agString.c online.c

uno : uno.c $(LIBS)
	clear
	@gcc -o uno uno.c $(CFLAGS) $(LIBS) 2>&1 >/dev/null | more -30

host :
	./uno -m -b 0 AJ

join :
	./uno -i 0.0.0.0 -j 3543 Chase

dj :
	gdb --args ./uno -j 3543 -i 0.0.0.0 Fabs

dh :
	gdb --args ./uno AJ -m -b 0
