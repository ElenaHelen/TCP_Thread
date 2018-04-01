#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

//Константи, які визначають операційну систему
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define OS_WINDOWS
#ifdef OS_WINDOWS
#include <windows.h>
#include <winsock2.h>
//Символ розділення папок у шляху до файлу
#define PATH_DELIM '\\'
#else
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
//Дескриптор сокету
#define SOCKET unsigned int
//Функція порівняння рядків без врахування регістру
#define stricmp strcasecmp
#define PATH_DELIM '/'
#endif

#define BUFFER_SIZE 256 //Розмір звичайного буферу
#define MAX_BUFFER_SIZE 4096 //Максимальний розмір буферу
#define MAX_FILE_SIZE 65535*16 //Максимальний розмір файлу

//Довжина черги серверу
#define LINEQ 5

char disconnetClientCmd[10]="bye";
char shutdownServerCmd[20]="serverShutdown";

//Шаблон HTTP- запиту
#define SUCCESS_RESPONSE_SERVER_CMD "Server success \ received %d bytes"

//Функція ініціалізації підсистеми сокетів
int initSocketAPI() {
#ifdef OS_WINDOWS
	WSAData ws;
	return WSAStartup(MAKEWORD(2,2),&ws);
#else
	return 0;
#endif
}

//Функція деініціалізації підсистеми сокетів
int deinitSocketAPI() {
#ifdef OS_WINDOWS
	return WSACleanup();
#else
	return 0;
#endif
}

//Універсальна функція закриття сокету
void closeSocket(SOCKET sc) {
#ifdef OS_WINDOWS
	closesocket(sc);
#else
	close(sc);
#endif
}


//Виведення повідомлення про помилку
int socketError(bool isError, char* msg, bool deinitSock = false) {
	int ret = -1;
	if (isError) {
		std::cout<<"Error socket ";
#ifdef OS_WINDOWS
		ret = WSAGetLastError();
		if (deinitSock) {
			WSACleanup();
		}
#else
		perror(msg);
		std::cout<<msg<<endl;
#endif
	}
	std::cout<<msg<<" (code: "<<ret<<")";
	return ret;
}

//Зчитування рядку символів з сокету
int readLine(SOCKET fd, char *buf) {
	int rc=0, n =0;
	while(1) {
		rc = recv( fd, buf, 1, 0 );
		if (rc <=0) return (n) ? n : rc;
		if (*buf == '\0') break;
		++n;
		++buf;
	}
	return n;
}

//Функція повертає значення параметру по імені
// з командного рядка

char* getParameter(char** argv,int argc, const char* paramName, char* paramValue, char delim = ' ') {
	if (!argv || argc<=1 || !paramName) return NULL;
	for (int i=1; i<argc; ++i) {

		if (!strncasecmp(argv[i],paramName,strlen(paramName))) {
			if (delim==' ') {
				if (i==argc-1)
					return NULL;
				else {

					strcpy(paramValue,argv[i+1]);
					return paramValue;
				}

			} else {

				int nLen = strlen(argv[i])-strlen(paramName);
				if (nLen<=0)
					return NULL;
				strcpy(paramValue,argv[i]+strlen(paramName)+1);
				return paramValue;

			}

		}

	}

	return NULL;


}

//Функція розбиває рядок імені серверу на ім'я, порт та
//шлях до файлу

char* getServerPortPath( const char* server_port_path
                        ,char*server
                        ,int& port
                        ,char*path
                        ,char portDelim=':'
                        ,char pathDelim='/') {

	char* pportDelim = strchr(server_port_path,':');
	char* ppathDelim = strchr(server_port_path,'/');
	char* pservEnd = (pportDelim)?pportDelim:(ppathDelim)?ppathDelim:NULL;

	if (pservEnd) {
		strncpy(server,server_port_path,pservEnd-server_port_path);
		server[pservEnd-server_port_path]='\0';

		if (pportDelim && port) port = atoi(pportDelim+1);

		if (ppathDelim) strcpy(path,ppathDelim);
	}

	else strcpy(server,server_port_path);
	return server;

}

//Функція розбиває рядок імені серверу на ім'я та порт
char* getServerPort(char* server_port, char*server,
                    int& port,char delim=':') {
	char* serverEnd = strchr(server_port,delim);
	if (!serverEnd) {

		strcpy(server,server_port);
		return server_port;

	}

	strncpy(server,server_port,serverEnd-server_port);
	server[serverEnd-server_port]='\0';
	int ntmpPort = atoi(serverEnd+1);

	if (ntmpPort>0)


		port = ntmpPort;
	return server;
}

//Функція отримує ім'я серверу та порт з командного рядку
char* getServerPort(char** argv,int argc,
                    const char* paramName, char*server,int& port,char delim=':') {

	if (!argv || argc<=1 || !server) return NULL;
	for (int i=1; i<argc; ++i) {

		if (stricmp(argv[i],paramName) || i==argc-1) continue;
		return getServerPort(argv[i+1],server,port,delim);

	}

	return server;

}

//Функція отримує ім'я серверу, можливо порт та шлях до файлу

//з командного рядку

char* getServerPortPath(char** argv,int argc,
                        const char* paramName, char*server, int& port, char*path, char portDelim=':',char pathDelim='/') {

	if (!argv || argc<=1 || !paramName || !server) return NULL;
	for (int i=1; i<argc; ++i) {

		if (stricmp(argv[i],paramName) || i==argc-1) continue;
		return getServerPortPath(argv[i+1],server,port, path,portDelim,pathDelim);

	}

	return server;

}

//Функція повертає повний шлях файлу по його імені

char* extractFilePath(char* filePath, const char* fileName, char delim=PATH_DELIM) {
	if (!filePath || !fileName)
		return NULL;
	char* curDelim = strchr(fileName,delim);
	if (!curDelim)
		return NULL;

	char* lastDelim = NULL;
	while (curDelim) {
		lastDelim = curDelim;
		curDelim=strchr(lastDelim+1,delim);
	}

	if (lastDelim+1-fileName==strlen(fileName)) {
		strcpy(filePath,fileName);
		filePath[strlen(fileName)]='\0';
	}

	else {
		strncpy(filePath,fileName,lastDelim-fileName+1);
		filePath[lastDelim-fileName+1]='\0';
	}


	return filePath;

}

//Функція повертає ім'я файлу по його повному шляху
char* extractFileName(char* fileName,
                      const char* filePath, char delim=PATH_DELIM) {

	if (!filePath || !fileName) return NULL;

	char* curDelim = strchr(filePath,delim);
	if (!curDelim)
		return NULL;

	char* lastDelim = NULL;
	while (curDelim) {
		lastDelim = curDelim;

		curDelim=strchr(curDelim+1,delim);
	}


	if (lastDelim+1==filePath+strlen(filePath)) return NULL;
	strcpy(fileName,lastDelim+1);

	return fileName;

}


//Виведення заголовочної інформації при запуску програми
void printInfo(char*programName,
               char* descript = NULL,bool isServer=true) {
	char fileName[BUFFER_SIZE];
	extractFileName(fileName,programName);
	std::cout<<"********************** "<<fileName;
	std::cout<<" **********************\n";

	if (descript) {
		std::cout<<descript<<std::endl;

	}

	std::cout<<"\nYou can configure "<<fileName;
	std::cout<<" using command line in format\n\n";
	std::cout<<"\t"<<fileName;

	if (isServer)

		std::cout<<" -port:<number port>\n\n";
	else

		std::cout<<" -server <server name:[port][/path]>\n\n";
	for (int i=0; i<strlen(fileName); ++i)

		std::cout<<"*";

	std::cout<<"*************************************";

	std::cout<<"*********\n\n";
}

#endif // _COMMON_UTILS_H_
