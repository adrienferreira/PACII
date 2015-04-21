pop : main-pop.o utils-pop.o textuel-pop.o mimes-pop.o 
	gcc obj/main-pop.o obj/utils-pop.o obj/textuel-pop.o obj/mimes-pop.o -o bin/pop

main-pop.o : src/main-pop.c include/main-pop.h include/common-pop.h
	gcc -c src/main-pop.c -o obj/main-pop.o -Iinclude

utils-pop.o : src/utils-pop.c include/utils-pop.h include/common-pop.h
	gcc -c src/utils-pop.c -o obj/utils-pop.o -Iinclude

textuel-pop.o : src/textuel-pop.c include/textuel-pop.h include/common-pop.h
	gcc -c src/textuel-pop.c -o obj/textuel-pop.o -Iinclude

mimes-pop.o : src/mimes-pop.c include/mimes-pop.h include/common-pop.h
	gcc -c src/mimes-pop.c -o obj/mimes-pop.o -Iinclude

clean :
	rm pop;	rm *~;	rm src/*~;	rm include/*~;	rm bin/*;	rm obj/*;
