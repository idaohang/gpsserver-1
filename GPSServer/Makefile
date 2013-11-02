CC = g++
INCLUDE =
INCDIR = 
APILIB	=	
CCFLAGS	= -g -c -I. -I$(INCDIR) -I/usr/local/include `mysql_config --cflags`
LDFLAGS = -liconv   -lpthread -ldl -L$(OBJDIR) -L/usr/lib -L/usr/local/lib `mysql_config --libs`
OBJS = IniFile.o ffcs_logger.o ffcs_logger_queue.o LogAdmin.o Epoll.o API-Socket.o API-Tools.o ProcGPSMsg.o SendToGPS.o soapC.o soapServer.o soapClient.o stdsoap2.o WebService.o RFID.o 3des.o

all:gps_server
gps_server:$(OBJS)
	$(CC)  -o gps_server $(OBJS)  $(LDFLAGS)
IniFile.o:IniFile.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  IniFile.cpp -o IniFile.o 
ffcs_logger.o:ffcs_logger.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  ffcs_logger.cpp -o ffcs_logger.o 
ffcs_logger_queue.o:ffcs_logger_queue.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  ffcs_logger_queue.cpp -o ffcs_logger_queue.o 
LogAdmin.o:LogAdmin.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  LogAdmin.cpp -o LogAdmin.o 
Epoll.o:Epoll.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  Epoll.cpp -o Epoll.o 
API-Socket.o:API-Socket.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  API-Socket.cpp -o API-Socket.o 
API-Tools.o:API-Tools.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  API-Tools.cpp -o API-Tools.o 
ProcGPSMsg.o:ProcGPSMsg.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  ProcGPSMsg.cpp -o ProcGPSMsg.o 
SendToGPS.o:SendToGPS.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  SendToGPS.cpp -o SendToGPS.o 
soapC.o:soapC.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  soapC.cpp -o soapC.o 
soapServer.o:soapServer.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  soapServer.cpp -o soapServer.o 
soapClient.o:soapClient.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  soapClient.cpp -o soapClient.o 
stdsoap2.o:stdsoap2.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  stdsoap2.cpp -o stdsoap2.o 
WebService.o:WebService.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  WebService.cpp -o WebService.o 
RFID.o:RFID.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  RFID.cpp -o RFID.o 
3des.o:3des.cpp
	$(CC) $(CCFLAGS) $(INCLUDE)  3des.cpp -o 3des.o 

clean:
	rm -rf *.o gps_server
