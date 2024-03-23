CC              = clang
CFLAGS          = -Wall -Wextra -Werror -Wstrict-prototypes -pedantic
LFLAGS          = 
EXEC            = huff dehuff
HUFF_OBJS      = bitwriter.o node.o pq.o huff.o
DEHUFF_OBJS	=bitwriter.o node.o dehuff.o bitreader.o
PQTEST_OBJS = pq.o node.o pqtest.o 
NODETEST_OBJS = node.o nodetest.o 
BWTEST_OBJS = bwtest.o bitwriter.o
BRTEST_OBJS = brtest.o bitreader.o
#LIBS1= io-$(shell uname -m).a


all: huff dehuff pqtest nodetest bwtest brtest

huff: $(HUFF_OBJS) #$(LIBS1)
	$(CC) $(LFLAGS) -o huff $(HUFF_OBJS) #$(LIBS1)

dehuff: $(DEHUFF_OBJS) #$(LIBS1)
	$(CC) $(LFLAGS) -o dehuff $(DEHUFF_OBJS) #$(LIBS1)
nodetest: $(NODETEST_OBJS)
	$(CC) $(LFLAGS) -o nodetest $(NODETEST_OBJS)

pqtest: $(PQTEST_OBJS)
	$(CC) $(LFLAGS) -o pqtest $(PQTEST_OBJS)

bwtest: $(BWTEST_OBJS) # $(LIBS1)
	$(CC) $(LFLAGS) -o bwtest $(BWTEST_OBJS) #$(LIBS1)

brtest: $(BRTEST_OBJS)
	$(CC) $(LFLAGS) -o brtest $(BRTEST_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f huff dehuff bwtest pqtest nodetest brtest $(HUFF_OBJS) $(DEHUFF_OBJS) $(PQTEST_OBJS) $(NODETEST_OBJS) $(BWTEST_OBJS) $(BRTEST_OBJS)

format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build --use-cc=$(CC) make



