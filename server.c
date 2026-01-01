#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define CLNT_MAX 10
#define BUFFSIZE 200

int g_clnt_socks[CLNT_MAX]; //클라이언트 소켓, 글로벌 함수니까 g
int g_clnt_count = 0;

//init 걸려면 인자 있어야 함, 모든 쓰레드에서 사용할 거라 맨 위에 있음
pthread_mutex_t g_mutex;

void send_all_clnt(char * msg, int my_sock){

        pthread_mutex_lock(&g_mutex);

        for(int i=0; i<g_clnt_count; i++){

                if(g_clnt_socks[i] != my_sock){
                        write(g_clnt_socks[i], msg, strlen(msg)+1);
                }
        }
        pthread_mutex_unlock(&g_mutex);
}

//고객 전용 함수
void * clnt_connection(void * arg){

        int clnt_sock = (int)arg;
        int str_len = 0;

        char msg[BUFFSIZE];
        int i;

        //읽으면 뿌린다
        while(1){
                str_len = read(clnt_sock, msg, sizeof(msg)); //내가 연락한 클라이언트만 빼고 전달
                if(str_len == -1){
                        printf("clnt[%d] close\n", clnt_sock); //읽으면
                        break;
                }
                send_all_clnt(msg, clnt_sock); //접속된 모든 클라이언트한테 읽은 내용을 전달
                printf("%s\n", msg);

        }

        pthread_mutex_lock(&g_mutex);

        close(clnt_sock);

        //고객 하나 사라졌으니까 소켓 저장해 놓은 걸 없앨려고 함.
        for(int i=0; i<g_clnt_count; i++){
                if(clnt_sock == g_clnt_socks[i]){
                        for(; i<g_clnt_count-1; i++){
                                g_clnt_socks[i] = g_clnt_socks[i+1];
                        }
                        break;
                }
        }

        g_clnt_socks[CLNT_MAX];
        g_clnt_count = 0;

        pthread_mutex_unlock(&g_mutex);

        pthread_exit(0);
        return NULL;
}

int main(int argc, char ** argv){

        int serv_sock; //서버 소켓
        int clnt_sock; //클라이언트 소켓

        pthread_t t_thread;

        struct sockaddr_in clnt_addr;
        int clnt_addr_size;

        struct sockaddr_in serv_addr; //포트 번호 지정 위해 구조체 선언

        pthread_mutex_init(&g_mutex, NULL); //init 걸기

        //빈껍데기 소켓 만드는 함수
        serv_sock = socket(PF_INET, SOCK_STREAM, 0); //소켓을 열건데 형태가 IPv4를 사용 + TCP 통신 + 0(프로토콜)

        //bind할 정보
        serv_addr.sin_family = AF_INET; //PF_INET하고 같은 의미. P는 프로토콜, A는 address 설정할 때
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //바인드 할 때 어떤 주소로 바인드 할거냐.. INADDR_ANY = 내 현재 PC IP. htonl = inaddr_any를 호스트 방식의 오더를 네트워크 오더 방식인 빅 엔디안으로 바꿔라라는 의미. l = 32bit           
        serv_addr.sin_port = htons(7989);//port 넣기. s = 16bit

        //bind 시작
        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
                printf("bind error\n");
        }

        //listen 시작
        if(listen(serv_sock, 5) == -1){ //몇 명을 listen 상태로 대기할 수 있느냐
                printf("listen error\n");
        }

        char buff[200]; //버퍼에 담기
        int recv_len = 0; //얼만큼 받아왔는지

        while(1){

                clnt_addr_size = sizeof(clnt_addr);

                //accept 시작 (listen이 끝날 때마다 고객 맞이)
                clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

                //고객이 동시에 들어오고 나가면 문제가 발생될 수 있다. -> mutex 걸어야 한다.
                pthread_mutex_lock(&g_mutex);
                //고객이 1번 들어올 때마다 저장
                g_clnt_socks[g_clnt_count++] = clnt_sock;
                pthread_mutex_unlock(&g_mutex); //lock 해제

                //thread로 생성
                pthread_create(&t_thread, NULL, clnt_connection, (void *)clnt_sock);
        }
}
