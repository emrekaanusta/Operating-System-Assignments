#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <list>


using namespace std;

class HeapManagementLibrary{
    
    private:

        struct dataNode   // vector element
        {

            int ID;
            int size;
            int index;

            dataNode(int id, int size2, int index2) : 
            ID(id),
            size(size2),
            index(index2){}

            bool operator==(const dataNode& other) const {
                return ID == other.ID && size == other.size && index == other.index;
    }
        };

        pthread_mutex_t mutex;  // lock initialazation

        list<dataNode> dataList;
        

    public:
        HeapManagementLibrary(){
            mutex = PTHREAD_MUTEX_INITIALIZER;
        }

        int initHeap(int size){

            dataNode newNode = dataNode(-1, size, 0);

            cout << "Memory initialized..." ;

            print();

            dataList.push_back(newNode);

            for( dataNode curr : dataList){

            cout << "[" << curr.ID << "][" << curr.size << "][" << curr.index  << "]" ; 
            }

            cout << endl;

            return 1;
        }
    
        int myMalloc(int threadID, int size){

            pthread_mutex_lock(&mutex);

            for (list<dataNode>::iterator currNode = dataList.begin(); currNode != dataList.end(); currNode ++ ) {

                if(currNode -> size >= size && currNode -> ID == -1){

                    int createdIndex = currNode -> index;


                    int createdSize = currNode -> size;


                    dataNode createdNode = dataNode (threadID , size, createdIndex);
                    // create a new node with the desired inputs


                    dataList.insert(currNode, createdNode);
                    // insert at the right place

                    if( createdSize <= 0){  // not enough size delete the current node
                        dataList.erase(currNode);
                    }

                    else{
                        // enough size so increase the size with the input

                        currNode -> index = createdIndex + size;

                        currNode -> size = createdSize - size;
                    }


                    std::cout << "Allocated for thread " << threadID << endl;


                    pthread_mutex_unlock(&mutex);

                    return createdIndex;

                }

            }

            cout << "Can not allocate, requested size "<< size << " for thread " << threadID << " is bigger than remaining size "<< endl;



            pthread_mutex_unlock(&mutex);

            return -1;

        } 


        int myFree(int ID, int index){

            pthread_mutex_lock(&mutex);

            for (list<dataNode>::iterator currNode = dataList.begin(); currNode != dataList.end(); currNode ++ ) {


                if ( currNode -> index == index && currNode -> ID == ID ){

                    // check if there is a matching node

                    if ( currNode != dataList.begin()){

                        list<dataNode>::iterator passedNode = currNode;

                        passedNode--;

                        if(passedNode -> ID == -1){

                            passedNode -> size = passedNode -> size + currNode -> size;

                            currNode = dataList.erase(currNode);

                            currNode--;

                        }

                    }
                


                currNode -> ID = -1;


                list <dataNode> :: iterator upcomingNode = currNode;

                upcomingNode ++ ;

                // pass to the next node 

                if(upcomingNode != dataList.end() && upcomingNode -> ID == -1 ){

                    // delete upcoming node and transfer the size of it to the current one

                    currNode -> size = currNode -> size + upcomingNode -> size;

                    dataList.erase(upcomingNode);

                }

                cout << "Freed for thread " << ID << endl;


                pthread_mutex_unlock(&mutex);

                return 1;
                }
            }

            cout << "No matching node to free for the thread "<< ID << endl;

            pthread_mutex_unlock(&mutex);

            return -1;

        }



    void print() {
        pthread_mutex_lock(&mutex);

        auto lastNode = std::prev(dataList.end());  // Get an iterator to the last node

        for (auto currNode = dataList.begin(); currNode != dataList.end(); ++currNode) {
            cout << "[" << currNode->ID << "][" << currNode->size << "][" << currNode->index << "]";
            
            if (currNode != lastNode) {  // so that last node does not print ---
                cout << "---";
            }
        }

        cout << endl;

        pthread_mutex_unlock(&mutex);
    }
};

/*   

Please use print function after the other function calls. I did not 
call print function inside the other functions. Below is an example that is working.


int main() {
    HeapManagementLibrary heapManager;

    // Initialize the heap with a size of 100
    heapManager.initHeap(100);

    // First thread allocates 40 bytes
    int thread1_alloc1 = heapManager.myMalloc(0, 40);
    // Print the state of the heap after the first allocation
    heapManager.print();

    // First thread frees the first allocation
    heapManager.myFree(0, thread1_alloc1);
    // Print the state of the heap after the first free
    heapManager.print();

    thread1_alloc1 = heapManager.myMalloc(0, 20);
    // Print the state of the heap after the first allocation
    heapManager.print();

       // First thread frees the first allocation
    heapManager.myFree(0, thread1_alloc1);
    // Print the state of the heap after the first free
    heapManager.print();




    // First thread allocates 20 bytes
    int thread2_alloc2 = heapManager.myMalloc(1, 40);
    // Print the state of the heap after the second allocation
    heapManager.print();

    // First thread frees the second allocation
    heapManager.myFree(1, thread2_alloc2);
    // Print the state of the heap after the second free
    heapManager.print();

    // Second thread allocates 30 bytes
    thread2_alloc2 = heapManager.myMalloc(1, 20);
    // Print the state of the heap after the third allocation
    heapManager.print();

    // Second thread frees the third allocation
    heapManager.myFree(1, thread2_alloc2);
    // Print the state of the heap after the third free
    heapManager.print();

    return 0;
}

*/