#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h>
#define BUF_SIZE 100
#define MAX_CLNT 256

// 함수 선언 /////////////////////
void *handle_clnt(void* arg);
void send_msg(char* msg, int len);
void send_msg_second(char*, int);
void error_handling(char* msg);
///////////////////////////////////
//char (*member)[10] = {"jeon", "tae", "kyeong"};

//쓰레드 인자 구조체//
struct ClntInf
{
	int sock; // 소켓 번호
	int room; // 방 번호
};

//고객 리스 구조체//
struct client
{
	char id[BUF_SIZE];
	char pwd[BUF_SIZE];
};

int clnt_cnt_first = 0; //클라이언트 소켓 번호 인덱스
int clnt_cnt_second = 0; // 클라이언트 2 소켓 번호 인덱스
int clnt_socks_first[MAX_CLNT]; // 1번 채팅방 소켓 정보
int clnt_socks_second[MAX_CLNT]; // 2번 채팅방 소켓 정보소
pthread_mutex_t mutx;


int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	int room;
	int pass;
	int fd, fd2;
	int password[BUF_SIZE];
	struct ClntInf clntinf;
	struct sockaddr_in serv_adr, clnt_adr;
	struct client* list;
	int clnt_adr_sz;
	char key[4];
	char service[10];
	int flag = 0;
	int length = 0;
	char name[BUF_SIZE];
	char pwd[BUF_SIZE];
	FILE *fp;
	int i=0;
	pthread_t t_id;
	if(argc!=2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, MAX_CLNT) == -1)
		error_handling("listen() error!");
	while(1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

		fp = fopen("list2.txt", "r");
		if(fp == NULL){
			printf("file error!\n");
		}
		else{
			//fscanf(fp, "%d\n", &length);
			list = (struct client*)malloc(sizeof(struct client)*100);
			while(!feof(fp)){
				fscanf(fp, "%s %s\n", &name, &password);
				strcpy(list[i].id, name);
				strcpy(list[i].pwd, password);
				i++;
				length++;
			}
		}

		fclose(fp);


		flag = -1;
		/* interface
		 * 로그인/ 회원가입 설정 후 처리 하는 곳*/
		while(flag < 0 ){
		
			pass = read(clnt_sock, service, sizeof(service)); // 로그인 or 회원 가입 서비스 선택
	
			// 회원 가입 서비스 //
			if(!strcmp(service, "join")){
				flag = 1;
				char pass[3] = "YES";
				char fail[2] = "NO";
				read(clnt_sock, name, sizeof(name));
				read(clnt_sock, pwd, sizeof(pwd));

				/* 중복확인*/
				for(i=0; i<length; i++){
					if(!strcmp(name, list[i].id))
					{
						write(clnt_sock, fail, sizeof(fail));
						flag = -1; //중복된 아이디가 있음//
						break;
					}
				}
				// 회원가입 가능 //
				if(flag == 1){
					//fp2 = fopen("list2.txt", "a");
					if((fd = open("list2.txt", O_WRONLY|O_APPEND)) == -1)
					{
						perror("open error()");
						return 1;
					}

					int cnt;
					cnt = write(fd, name, strlen(name));
					cnt = write(fd, " ", 1);
					cnt = write(fd, pwd, strlen(pwd));
					cnt = write(fd, "\n", 1);

					write(clnt_sock, pass, sizeof(pass));
				}
			}
			else if(!strcmp(service, "login")){
				flag = 1; // 로그인에 성공하면 flag = 1
				// 로그인에 실패하면 flag = -1
				char pass[3] = "YES";
				char fail[2] = "NO";
				read(clnt_sock, name, sizeof(name));
				read(clnt_sock, pwd, sizeof(pwd));
				/*
				 * 클라이언트의 아이디 변수는 name
				 * 클라이언트의 비밀번호 변수는 pwd(정수)
				 */
				//test 문자
				//char testid[10] = "asd";
				//char testpass[10] = "asd";

				//int cmpid = strcmp(name, testid);
				//int cmppass = strcmp(pwd, testpass);
				//
				int index=0;
				int cmpid=-1, cmppass=-1;
				for(index=0; index<length; index++){
					if((!strcmp(list[index].id, name))&&(!strcmp(list[index].pwd, pwd)))
					{
						printf("check!\n");
						cmpid = 0;
						cmppass = 0;
						break;
					}
				}
				
				if(cmpid == 0 && cmppass == 0){
					printf("login success\n");
					write(clnt_sock, pass, sizeof(pass));
				} else {
					printf("login failed");
					write(clnt_sock, fail, sizeof(fail));
					flag = -1;
				}

				//원준 파트//
				//로그인 성공시, flag == 1 유지
				//실패시, flag == -1 로 변경 
			}
			
		}

		room = read(clnt_sock, key, sizeof(key));
		key[3] = '\0';
		printf("%s\n", key);

		if(!strcmp(key, "one")){
			printf("Guest(chatting room #1)\n");
			pthread_mutex_lock(&mutx);
			clnt_socks_first[clnt_cnt_first++] = clnt_sock;
			printf("clnt_cnt_first : %d , clnt_sock : %d\n", clnt_cnt_first, clnt_sock);
			pthread_mutex_unlock(&mutx);
			
			clntinf.sock = clnt_sock;
			clntinf.room = 1;

			pthread_create(&t_id, NULL, handle_clnt, (void*)&clntinf);
			pthread_detach(t_id);
			printf("Connected client IP : %s \n", inet_ntoa(clnt_adr.sin_addr));
		} else if(!strcmp(key, "two")){
			printf("Guest(chatting room #2)\n");
			pthread_mutex_lock(&mutx);
			clnt_socks_second[clnt_cnt_second++] = clnt_sock;
			printf("clnt_cnt_second : %d , clnt_sock : %d\n", clnt_cnt_second, clnt_sock);
			pthread_mutex_unlock(&mutx);

			clntinf.sock = clnt_sock;
			clntinf.room = 2;

			pthread_create(&t_id, NULL, handle_clnt, (void*)&clntinf);
			pthread_detach(t_id);
			printf("Connected client IP : %s \n", inet_ntoa(clnt_adr.sin_addr));
		}
		
	}

	close(serv_sock);
	return 0;
}

void* handle_clnt(void* arg)
{
	struct ClntInf *args = (struct ClntInf*)arg;
	int clnt_sock = args->sock;
	int room = args->room;
	int str_len1 = 0, str_len2=0;
	int i;
	char msg1[BUF_SIZE];
	char msg2[BUF_SIZE];

	/* chatting room 1 service */

	if(room == 1){
		while((str_len1 = read(clnt_sock, msg1, sizeof(msg1)))!=0){
			send_msg(msg1, str_len1);
		}


		pthread_mutex_lock(&mutx);
		for(i=0; i<clnt_cnt_first; i++)
		{
			if(clnt_sock == clnt_socks_first[i])
			{
				while(i++< clnt_cnt_first-1)
					clnt_socks_first[i] = clnt_socks_first[i+1];
				break;
			}
		}

		clnt_cnt_first--;
		pthread_mutex_unlock(&mutx);

	/* chatting room 2 service */

	} else if (room == 2){	
		while((str_len2 = read(clnt_sock, msg2, sizeof(msg2)))!=0){
			send_msg_second(msg2, str_len2);
		}



		pthread_mutex_lock(&mutx);
		for(i=0; i<clnt_cnt_second; i++)
		{
			if(clnt_sock == clnt_socks_second[i])
			{
				while(i++< clnt_cnt_second-1)
					clnt_socks_second[i] = clnt_socks_second[i+1];
				break;
			}
		}

		clnt_cnt_second--;
		pthread_mutex_unlock(&mutx);
	}

	close(clnt_sock);
	return NULL;
}

void send_msg(char* msg, int len)
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt_first; i++)
		write(clnt_socks_first[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void send_msg_second(char* msg, int len)
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt_second; i++)
		write(clnt_socks_second[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

















