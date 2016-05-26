.PHONY: all
all : libcrawler.so file_tester web_tester

file_tester : file_tester.c libcrawler.so
	gcc -g -L. file_tester.c -Wall -o file_tester -lcrawler -lpthread

web_tester : web_tester.c cs537.c libcrawler.so
	gcc -g -L. -lcrawler -lpthread web_tester.c cs537.c -Wall -o web_tester

libcrawler.so : crawler.c
	#gcc -g -shared -fPIC -o libcrawler.so crawler.c 
	gcc -g -fPIC -c -Wall -pthread crawler.c -o crawler.o
	gcc -g -fPIC -c -Wall -pthread hashTable.c -o hashTable.o
	gcc -g -shared -o libcrawler.so crawler.o hashTable.o

.PHONY: clean
clean :
	rm -f file_tester web_tester libcrawler.so *.o *~


        #gcc -g -fPIC -c crawler.c -Wall -Werror -o crawler.o
        #gcc -g -shared -o libcrawler.so crawler.o

