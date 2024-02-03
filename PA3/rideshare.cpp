#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

int numTeamA;
int numTeamB;


sem_t teamASem;
sem_t teamBSem;


pthread_barrier_t barrier;

pthread_mutex_t mutex;
pthread_mutex_t mutex2;


int carID = 0;



int fanA = 0;
int fanB = 0;

int carCountA = 0;
int carCountB = 0;

void* teamAfan(void* arg) {

    pthread_t tid = pthread_self();
    
    // Phase 1: Looking for a car
    pthread_mutex_lock(&mutex);
    cout << "Thread ID: " << tid << ", Team: A, I am looking for a car\n";
    fanA++;

    if (fanA >= 2 && fanB >= 2) {  // 2-2
        for (int i = 0; i < 2; i++) {
           
            sem_post(&teamBSem);
        }

        for (int i = 0; i < 2; i++) {
            
            sem_post(&teamASem);
        }   

        fanA -= 2;
        fanB -= 2;    
        pthread_mutex_unlock(&mutex);
    } 
    
    else if (fanA == 4) {    // 4 A
        for(int i = 0; i < 4; i ++){
            sem_post(&teamASem);
        }

        fanA -= 4;
        pthread_mutex_unlock(&mutex);
    } 
    
    else {     
        pthread_mutex_unlock(&mutex);
        sem_wait(&teamASem);
    }

    pthread_barrier_wait(&barrier);


    pthread_mutex_lock(&mutex);
    cout << "Thread ID: " << tid << ", Team: A, I have found a spot in a car\n";
    carCountA++;
 
    if ((carCountA + carCountB) == 4 ) {
        cout << "Thread ID: " << tid << ", Team: A, I am the captain and driving the car with ID " << carID << "\n";
        carID++;
        carCountA = 0;
        carCountB = 0;

        sem_post(&teamASem);
        
    }

    fanA = 0;
    fanB = 0;

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* teamBfan(void* arg) {

    pthread_t tid = pthread_self();

   
    pthread_mutex_lock(&mutex);
    cout << "Thread ID: " << tid << ", Team: B, I am looking for a car\n";
    fanB++;

    if (fanA >= 2 && fanB >= 2) {
        for (int i = 0; i < 2; i++) {
            pthread_mutex_unlock(&mutex);
            sem_post(&teamASem);
        }

        for (int i = 0; i < 2; i++) {
            pthread_mutex_unlock(&mutex);
            sem_post(&teamBSem);
        }

        fanA -= 2;
        fanB -= 2;
        pthread_mutex_unlock(&mutex);
    } 

    else if (fanB == 4) {
        for (int i = 0; i < 4; i++) {
            sem_post(&teamBSem);
        }

        pthread_mutex_unlock(&mutex);
        
        fanB -= 4;

    } 
    
    else {
        pthread_mutex_unlock(&mutex);
        sem_wait(&teamBSem);
    }

    pthread_barrier_wait(&barrier);

    pthread_mutex_lock(&mutex);
    cout << "Thread ID: " << tid << ", Team: B, I have found a spot in a car\n";
    carCountB++;


    if ((carCountA + carCountB) == 4) {
        cout << "Thread ID: " << tid << ", Team: B, I am the captain and driving the car with ID " << carID << "\n";
        carID++;
        carCountB = 0;
        carCountA = 0;
        sem_post(&teamBSem);

    }

    fanA = 0;
    fanB = 0;



    pthread_mutex_unlock(&mutex);


    return NULL;
}

int main(int argc, char* argv[]) {

    numTeamA = stoi(argv[1]);
    numTeamB = stoi(argv[2]);

    // validness
    if (numTeamA % 2 != 0 || numTeamB % 2 != 0 || (numTeamA + numTeamB) % 4 != 0) {
        cout << "Main terminates..." << endl;
        return 1;
    }


    sem_init(&teamASem, 0, 0);
    sem_init(&teamBSem, 0, 0);
  
    pthread_mutex_init(&mutex, NULL);

    pthread_barrier_init(&barrier, NULL, 4);


    pthread_t threads[numTeamA + numTeamB];
    int threadIDs[numTeamA + numTeamB];

    for (int i = 0; i < numTeamA + numTeamB; ++i) {
        threadIDs[i] = i;
        if (i < numTeamA) {
            pthread_create(&threads[i], NULL, teamAfan, &threadIDs[i]);
        } else {
            pthread_create(&threads[i], NULL, teamBfan, &threadIDs[i]);
        }
    }


    for (int i = 0; i < numTeamA; ++i) {
        sem_post(&teamASem);
    }


    for (int i = 0; i < numTeamB; ++i) {
        sem_post(&teamBSem);
    }


    for (int i = 0; i < numTeamA + numTeamB ; ++i) {
        pthread_join(threads[i], NULL);
    }


    sem_destroy(&teamASem);
    sem_destroy(&teamBSem);

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);
    

    cout << "The main terminates.\n";

    return 0;
}
