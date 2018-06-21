#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#define BUF_SIZE 100
#define NAME_SIZE 20

void* send_msg(void *arg);
void* recv_msg(void *arg);
void error_handling(char* msg);
int selectroom(void);
int interface(int);
char name[BUF_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char* argv[])
{
	int sock, sel;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=3)
	{
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	//sprintf(name, "[%s]", argv[3]);
	sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	while(1){
		if(interface(sock)>0)
			break;
		else
			continue;
	}

	sel = selectroom();
	if(sel==1){
		char* ch1 = "one";
		write(sock, ch1, strlen(ch1));
		
	} else if(sel==2){
		char* ch2 = "two";
		write(sock, ch2, strlen(ch2));
	}

	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	return 0;
}
int interface(int sock){
	int flag = 1;
	int num;
	int strlen;
	char service[5] = "join";
	char service2[6] = "login";
	char signal[3];
	//char name[BUF_SIZE];
	char pwd[BUF_SIZE];
	
	printf("which service?\n");
	printf("1. login\n");
	printf("2. join\n");
	scanf("%d", &num);
	if(num == 1){
		//사용자가 로그인을 할 것인지
		//아니면 회원가입을 할 것인지 정한다
		//여기서는 로그인을 할 것이기 때문에 
		//아이디와 패스워드를 서버에게 보낸다.
		write(sock, service2, sizeof(service2));
		printf(" ID >> ");
		scanf("%s", name);
		printf(" PWD >> ");
		scanf("%s", pwd);

		// 아이디와 패스워드를 보낸다//

		write(sock, name, sizeof(name));
		write(sock, pwd, sizeof(pwd));
		
		read(sock, signal, sizeof(signal));

		if(!strcmp(signal, "NO")){
			printf(".");
			flag = -1;
		}else if(!strcmp(signal,"YES")){
			printf("login success\n");
			flag = 1;
		}
		
		// 원준 파트 //

	}
	else if(num == 2){
			write(sock, service, sizeof(service));
			printf("[ JOIN ]\n");
			printf(" ID >> ");
			scanf("%s", name);
			printf(" PWD >> ");
			scanf("%s", pwd);
					
			write(sock, name, sizeof(name));
			write(sock, pwd, sizeof(pwd));

			strlen = read(sock, signal, sizeof(signal));
		
			if(!strcmp(signal, "NO")){
				printf("fail...\n");
				flag = -1;
			} else if(!strcmp(signal, "YES")){
				printf("success...\n");
				flag = 1;
			}
	}
	return flag;
}

int selectroom(void){
	int num;
	printf(" select chat room! \n");
	printf(" 1. No.1 chat room \n");
	printf(" 2. No.2 chat room \n");
	printf(" >> "); scanf("%d", &num);
	return num;
}

void * send_msg(void * arg)
{
	int sock = *((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	while(1)
	{
		fgets(msg, BUF_SIZE, stdin);
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			close(sock);
			exit(0);
		}

		sprintf(name_msg, "[%s]: %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
	}
	return NULL;
}

void* recv_msg(void* arg)
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len == -1)
			return (void*)-1;
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);
	}
	return NULL;
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}


