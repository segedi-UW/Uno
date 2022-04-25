CFLAGS= -Wall -g
LIBS=cards.c agLinkedList.c agUtil.c agLLIterator.c agLLNode.c agString.c online.c

uno : uno.c $(LIBS)
	clear
	@gcc -o uno uno.c $(CFLAGS) $(LIBS) 2>&1 >/dev/null | more -30

serve:
	./uno -s -b $(bots)

host :
	./uno -m -b 0 $(name)

join :
	./uno -j $(ip) $(name)

dj :
	gdb --args ./uno -j 0.0.0.0 Fabs

dh :
	gdb --args ./uno AJ -m -b 0

log : 
	cat .uno.log | more

clean :
	rm -f ./uno
	rm -f *.o
