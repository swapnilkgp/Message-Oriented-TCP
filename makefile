all:
	make -f libmake all
	make -f initmake all
	make -f usermake all

clean:
	make -f libmake clean
	make -f initmake clean
	make -f usermake clean
	ipcrm -a