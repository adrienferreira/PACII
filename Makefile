#pop : main-pop.o utils-pop.o textuel-pop.o mimes-pop.o 
#	gcc obj/main-pop.o obj/utils-pop.o obj/textuel-pop.o obj/mimes-pop.o -o bin/pop -g

pop: main-pop.o utils-pop.o mimes-pop.o clicable-pop.o graphique-pop.o
	gcc obj/clicable-pop.o obj/main-pop.o obj/utils-pop.o obj/mimes-pop.o obj/graphique-pop.o  -o bin/pop -L/usr/X11R6/lib -lX11 -Wall -g

main-pop.o: src/main-pop.c include/main-pop.h include/common-pop.h
	gcc -c src/main-pop.c -o obj/main-pop.o -Iinclude -Wall -g

utils-pop.o: src/utils-pop.c include/utils-pop.h include/common-pop.h
	gcc -c src/utils-pop.c -o obj/utils-pop.o -Iinclude -Wall -g

textuel-pop.o: src/textuel-pop.c include/textuel-pop.h include/common-pop.h
	gcc -c src/textuel-pop.c -o obj/textuel-pop.o -Iinclude -Wall -g

mimes-pop.o: src/mimes-pop.c include/mimes-pop.h include/common-pop.h
	gcc -c src/mimes-pop.c -o obj/mimes-pop.o -Iinclude -Wall -g

clicable-pop.o: src/clicable-pop.c include/clicable-pop.h include/common-pop.h
	gcc -c src/clicable-pop.c -o obj/clicable-pop.o -Iinclude -I/usr/X11R6/include -Wall -g

graphique-pop.o: src/graphique-pop.c include/graphique-pop.h include/common-pop.h
	gcc -c src/graphique-pop.c -o obj/graphique-pop.o -Iinclude -I/usr/X11R6/include -Wall -g

rmobj:
	rm obj/*;

clean:
	rm pop;	rm *~;	rm src/*~;	rm include/*~;	rm bin/*;rm scenarios/*~;rm obj/*;
