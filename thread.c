#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

int a = 0;

//mutex 사용
pthread_mutex_t mutex;

void * thread1(void * arg){

        printf("arg : %d\n", (int)arg);

        while(1){

                //mutex 사용 -> lock 걸면 된다.
                pthread_mutex_lock(&mutex);

                printf("thread%d a[%d]\n", (int)arg, ++a);

                //mutex lock 풀기
                pthread_mutex_unlock(&mutex);

                sleep(2);
        }

        return NULL;
}

int main(){

        pthread_t s_thread[2];
        int id1 = 77; //넘길 인자
        int id2 = 88;

        //mutex 생성
        pthread_mutex_init(&mutex, NULL);

        //thread 생성
        pthread_create(&s_thread[0], NULL, thread1, (void *)id1); //생성된 쓰레드에 대한 정보를 갖고 있음
        pthread_create(&s_thread[0], NULL, thread1, (void *)id2);

        while(1){
                printf(" main loop\n");
                sleep(1);
        }

        //pthread_join(s_thread[0], NULL);
        //pthread_join(s_thread[0], NULL);

}
  
