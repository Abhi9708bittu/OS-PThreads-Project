#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

typedef struct
{
	pthread_mutex_t mutex;
	pthread_mutex_t mutex2;
	pthread_cond_t cond;
	long long int value;
	long long int progress;
	long long int pending_tasks;

}
Semaphore;

// block

void _block(Semaphore* sem, long long int i)
{
	pthread_mutex_lock(&sem->mutex2);
	while(sem->progress + i < 0) pthread_cond_wait(&sem->cond, &sem->mutex2);
	pthread_mutex_unlock(&sem->mutex2);
}

// sem_lock

void sem_lock(Semaphore* sem)
{
	pthread_mutex_lock(&sem->mutex);

	sem->value--;
	if (sem->pending_tasks == 0)
	{
		pthread_mutex_lock(&sem->mutex2);
		sem->progress = 0;
		pthread_mutex_unlock(&sem->mutex2);
	}
	sem->pending_tasks++;
	int sem_value = sem->value, sem_progress = sem->progress;

	pthread_mutex_unlock(&sem->mutex);
	if(sem_value<0)
	{
		_block(sem, sem_value - sem_progress);
	}
}

// sem_release

void sem_release(Semaphore* sem)
{
	pthread_mutex_lock(&sem->mutex);
	pthread_mutex_lock(&sem->mutex2);

	sem->value++;
	sem->progress++;
	sem->pending_tasks--;
	pthread_cond_broadcast(&sem->cond);

	pthread_mutex_unlock(&sem->mutex2);
	pthread_mutex_unlock(&sem->mutex);
}

// sem_init

void sem_init(Semaphore* sem, long long int val)
{
	pthread_mutex_init(&sem->mutex, NULL);
	pthread_mutex_init(&sem->mutex2, NULL);
	pthread_cond_init(&sem->cond, NULL);
	sem->value = val;
	sem->pending_tasks = 0;
}


Semaphore mutex, wrt, enter;
pthread_t readerThreads[25], writerThreads[25];
int readerctr = 0;
int var = 0;

void *reader(int i){
    printf("Reader %d arrives\n", i+1);

    sem_lock(&enter);
    sem_lock(&mutex);
    sleep(5);
    readerctr++;
    if(readerctr == 1)  sem_lock(&wrt);
    sem_release(&mutex);
    sem_release(&enter);

    printf("CRITICAL SECTION ...\n");
    printf("Reader %d is reading var = %d\n", i+1, var);
    sleep(5);

    sem_lock(&mutex);
    readerctr--;
    if(readerctr == 0)  sem_release(&wrt);
    sem_release(&mutex);
    sleep(5);
    pthread_exit(0);
}

void *writer(int i){
    printf("Writer %d arrives\n", i+1);

    sem_lock(&enter);
    sem_lock(&wrt);
    sleep(5);
    sem_release(&enter);

    printf("CRITICAL SECTION ...\n");
    var++;
    printf("Writer %d is writing var = %d\n", i+1, var);
    sleep(5);

    sem_release(&wrt);
    sleep(5);
    pthread_exit(0);
}

int main(){
    int n, m;
    printf("Enter number of readers: ");
    scanf("%d", &n);
    printf("Enter number of writers: ");
    scanf("%d", &m);

    sem_init(&mutex, 1);
    sem_init(&wrt, 1);
    sem_init(&enter, 1);

    for(int i=0; i<n; i++){
        pthread_create(&readerThreads[i], NULL, reader, i);
    }

    for(int i=0; i<m; i++){
        pthread_create(&writerThreads[i], NULL, writer, i);
    }

    for(int i=0; i<n; i++){
        pthread_join(readerThreads[i], NULL);
    }

    for(int i=0; i<m; i++){
        pthread_join(writerThreads[i], NULL);
    }

    return 0;
}
