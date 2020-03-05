#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <bits/stdc++.h>

using namespace std;

const char* notFind = "HTTP/1.1 404 Not found\n\n404 Not found";
const char* STR_GET = "GET";

int split(char *s, char (*ss)[100], const char *sp = " \n"){
    int spLen = strlen(sp), sLen = strlen(s), ssPointer = 0;
    int last = 0;
    for(int i = 0; i < sLen; i++){
        bool isItv = 0;
        for(int j = 0; j < spLen; j++){
            if(s[i] == sp[j]){
                isItv = 1;
                break;
            }
        }
        if(isItv){
            int ssP = 0;
            for(int j = last; j < i; j++){
                ss[ssPointer][ssP++] = s[j];
            }
            ss[ssPointer++][ssP] = '\0';
            last = i + 1;
        }
    }
    return ssPointer;
}

bool cmpString(const char *a, const char *b){
    int lena = strlen(a), lenb = strlen(b);
    if(lena != lenb) return 0;
    for(int i = 0; i <= lena; i++) if(a[i] != b[i]) return 0;
    return 1;
}

void copyString(char* d, const char* s, int end = 0){
    memcpy(d, s, strlen(s) * sizeof(char));
    if(end){
        d[strlen(s) + 1] = '\0';
    }
}

void stringToCharPointer(string s, char* d){
    int len = s.length();
    for(int i = 0; i < len; i++){
        d[i] = s[i];
    }
    d[len + 1] = '\0';
}

void prtStr(char* s){
    int len = strlen(s);
    for(int i = 0; i < len; i++){
        printf("%d'%c|", (int)s[i], s[i]);
    }
    printf("\n");
    for(int i = 0; i < len; i++){
        printf("%c", s[i]);
    }
    printf("\n");
}

class TCPserver
{
public:
    int serverSock, port, bufLen, bufPionter;
	static const int bufMaxLen = 1024;
    struct sockaddr_in sockAddr;
    char IP[20], buf[bufMaxLen * 2 + 10];

    void init(){
        serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		bufLen = bufPionter = 0;
        sockAddr.sin_family = AF_INET;  //使用IPv4地址
        sockAddr.sin_addr.s_addr = inet_addr(IP);  //具体的IP地址
        sockAddr.sin_port = htons(port);  //端口
    }

    TCPserver(const char *IP_, int port_){
       memcpy(IP, IP_, (strlen(IP_) + 1) * sizeof(char));
       port = port_;
       init();
    }

	int readLine(int sock, char* ret){
        int retPointer = 0;
		for(;;){
            if(bufPionter == bufLen){
                bufPionter = bufLen = 0;

                fd_set fds;
                struct timeval timeout = {1, 0};
                FD_ZERO(&fds);
                FD_SET(sock, &fds);

                printf("selecting...\n");
                int selRet = select(sock + 1, &fds, NULL, NULL, &timeout);
                printf("select finish.\n");

                if(selRet <= 0){
                    printf("select fail.\n");
                    return selRet;
                }

                printf("reading...\n");
                int readRet = read(sock, buf, bufMaxLen);
                if(readRet == -1){
                    if(errno == EINTR){
                        continue;
                    }
                    else{
                        return -1;
                    }
                }
                else if(readRet == 0){
                    return 0;
                }
                else{
                    bufLen = readRet;
                }
                printf("reading finish.\n");
            }

            while(bufPionter < bufLen){
                char ch = buf[bufPionter++];
                ret[retPointer++] = ch;
                if(ch == '\n'){
                    ret[retPointer] = '\0';
                    return retPointer;
                }
            }
        }
	}

    void work(){
        bind(serverSock, (struct sockaddr*) &sockAddr, sizeof(sockAddr));
        listen(serverSock, 128);

        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);
        int clientSock = accept(serverSock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

        for(;;){
            char _buf[bufMaxLen];
            int readRet = readLine(clientSock, _buf);
            if(readRet <= 0){
                break;
            }

            char __buf[4] = {_buf[0], _buf[1], _buf[2], '\0'};
            if(cmpString(__buf, "GET")){
                printf("%s", _buf);
                char strings[20][100];
                int spLen = split(_buf, strings);
                for(int i = 0; i < spLen; i++){
                    printf("%s\n", strings[i]);
                }
            }

        }
        
        printf("sedding message...\n");
        write(clientSock, notFind, strlen(notFind));
		
		close(clientSock);
		close(serverSock);
		init();
    }

    ~TCPserver(){
		close(serverSock);
	}
};


int main(){
    TCPserver s("127.0.0.1", 80);
	s.work();
    return 0;
}