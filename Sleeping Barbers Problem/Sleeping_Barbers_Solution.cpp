#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <iostream>
using namespace std;

const int maxcustomer = 10;
const int maxbarber = 2;
const int sizequeue = 5;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cuscondition = PTHREAD_COND_INITIALIZER;
pthread_cond_t barcondition = PTHREAD_COND_INITIALIZER;

queue<int> waitroom;
int waitcustomer = 0;

void *barber(void *arg)
{
    int id = *(int *)arg;
    while (true)
    {
        pthread_mutex_lock(&mtx);
        while (waitroom.empty())
        {
            cout << "Barber " << id << " is going to sleep." << endl;
            pthread_cond_wait(&cuscondition, &mtx);
        }
        int custoid = waitroom.front();
        waitroom.pop();
        waitcustomer--;
        cout << "Barber " << id << " is cutting the hair of customer " << custoid << endl;
        pthread_mutex_unlock(&mtx);
        sleep(3);
        pthread_exit(0);
    }
}

void *customer(void *arg)
{
    int id = *(int *)arg;
    pthread_mutex_lock(&mtx);
    if (waitcustomer == sizequeue)
    {
        cout << "Customer " << id << " leaves because the waiting room is full." << endl;
        pthread_mutex_unlock(&mtx);
        return NULL;
    }

    waitroom.push(id);
    waitcustomer++;
    cout << "Customer " << id << " enters the waiting room." << endl;
    pthread_cond_signal(&cuscondition);
    while (waitroom.front() != id)                           // changed condition to avoid holding lock while signaling barber
    {
        pthread_cond_wait(&barcondition, &mtx);
    }
    pthread_mutex_unlock(&mtx);
    pthread_exit(0);
}


int main()
{
    pthread_t barbers[maxbarber];
    pthread_t customers[maxcustomer];
    int barberid[maxbarber];
    int customerid[maxcustomer];
    for (int i = 0; i < maxbarber; i++)
    {
        barberid[i] = i;
        pthread_create(&barbers[i], NULL, barber, &barberid[i]);
    }
    for (int j = 0; j < maxcustomer; j++)
    {
        customerid[j] = j;
        pthread_create(&customers[j], NULL, customer, &customerid[j]);
        sleep(1);
    }

    for (int i = 0; i < maxbarber; i++)
    {
        pthread_join(barbers[i], NULL);
    }
    for (int k = 0; k < maxcustomer; k++)
    {
        pthread_join(customers[k], NULL);
    }

    return 0;
}
