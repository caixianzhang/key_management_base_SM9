#注意此处千万不要连接标准openssl库，这样会导致椭圆曲线参数错误

CXX = g++
CXXFLAGS = -O2 -g -Wall
LDFLAGS = -lpthread -ldl -lmysqlclient

GMSSL = libcrypto.a

OBJS = 	main.o \
		sql.o \
		file.o clear.o directory.o \
		net.o \
		threadpool.o \
		rand.o \
		two-way-auth.o agreement.o distribute.o \
		sm4-cbc.o \
		sm9-KGC.o \
		sm9-keygen.o sm9-setup.o \
		sm9-decrypt.o sm9-encrypt.o \
		sm9-exchange.o \
		sm9-sign.o sm9-verify.o 

TARGET = KGC

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(GMSSL) -o $@ $(LDFLAGS)
	@ rm -rf *.o *.h *.cpp *.a
	
.PHONY:clean
clean:
	rm -rf *.o $(TARGET) 