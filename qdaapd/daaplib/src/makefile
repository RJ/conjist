CC = c++
AR = ar
CP = cp
RM = rm

TARGET = libdaaplib.a
OBJS = taginput.o tagoutput.o registry.o

LIBS = 
LIBPATH = 
LIBDEPLOY = /usr/local/lib
INCDEPLOY = /usr/local/include
INCPATH = ../include
CFLAGS = -Wall -Wno-multichar

.cpp.o:
	$(CC) $(CFLAGS) -I$(INCPATH) -c $<

$(TARGET): $(OBJS)
	$(AR) rs $(TARGET) $(OBJS)

$(OBJS): $(<:.o=.h) $(INCPATH)/daap/basic.h

clean:
	rm $(OBJS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) $(LIBDEPLOY)
	chmod a+r $(LIBDEPLOY)/$(TARGET)
	ranlib $(LIBDEPLOY)/$(TARGET)
	cp -r $(INCPATH)/daap $(INCDEPLOY)
	chmod -R a+r $(INCDEPLOY)/daap
