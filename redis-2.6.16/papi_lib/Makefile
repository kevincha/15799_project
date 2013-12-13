CC = gcc
PAPI_LIB=/usr/local/lib/libpapi.a

papi_test: papi_test.c papiwrap.c
	$(CC) -I/usr/local/include $^ $(PAPI_LIB) -o $@

clean:
	rm -f papi_test
