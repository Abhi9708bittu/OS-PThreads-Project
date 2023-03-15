#include <bits/stdc++.h>
#include <pthread.h>
#include <unistd.h>
#include <queue>
using namespace std;

#define N 5

struct Philosopher{
    int id;
    pthread_t tid;
    int state;
    //0=thinking
    //1=hungry
    //2=eating
    int w;
    //0=waiting
    //1=not waiting
    int r;
    //0=not running
};


struct Philosopher philosophers[N];
int times = 4;
int phil[N];

class monitor{

    pthread_mutex_t mut;
    //int sem;

public:

    void test(int n){
        if(philosophers[(n+1)%N].state !=2 && philosophers[(n+N-1)%N].state!=2 && philosophers[n].state==1){
            philosophers[n].state = 2;
            cout << "Philosopher " << (n + 1) << " is eating" <<endl;
            philosophers[n].w = 1;
        }
    }

    void take_fork(int n){
        philosophers[n].state = 1;
        test(n);
        if(philosophers[n].state!=2){
            philosophers[n].w = 0;
            do{
                test(n);
            }while(philosophers[n].w==0);
        }
    }

    void put_fork(int n){
        philosophers[n].state = 0;
        test((n+1)%N);
        test((n+N-1)%N);
    }

    monitor(){
    	pthread_mutex_init(&mut, NULL);
    }
}

object;

void* philo(void* arg){
    int c=0;
    while(c<times){
        int i = *(int*)arg;
        sleep(1);
        object.take_fork(i);
        sleep(1);
        object.put_fork(i);
        c++;
    }
}

int main(){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for(int i=0; i<N; i++){
        philosophers[i].id = i;
        philosophers[i].state = 0;
        philosophers[i].w = 1;
    }

    for(int i=0; i<N; i++){
        phil[i]=i;
    }

    for(int i=0; i<N; i++){
        pthread_create(&philosophers[i].tid, &attr, philo, &phil[i]);
    	cout << "Philosopher " << (i + 1) << " is thinking" << endl;
    }
    for(int i=0; i<N; i++){
        pthread_join(philosophers[i].tid, NULL);
    }

    pthread_attr_destroy(&attr);
    pthread_exit(NULL);

    return (0);
}
