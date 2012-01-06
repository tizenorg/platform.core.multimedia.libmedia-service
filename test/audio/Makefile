CC =gcc
LIBS = libmedia-info

OBJS = audio_svc_test.o
SRCS = audio_svc_test.c
TARGET = audio-svc-test

TARGET : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) `pkg-config $(LIBS) --cflags --libs` -g
$(OBJS) : $(SRCS)
	$(CC) -c $(SRCS) `pkg-config $(LIBS) --cflags --libs` -g
clean :
	rm $(TARGET)
	rm $(OBJS)
