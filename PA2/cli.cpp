#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string>
#include <fcntl.h>
#include <assert.h>
#include <sstream>
#include <pthread.h>
#include <fstream>
#include <unistd.h>  // Add this line for STDOUT_FILENO and fsync
#include <vector>
#include <cstring>

using namespace std; 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void * listener(void * arg){

    
    bool flag = true;
    long int value = (long int) arg;
    FILE * file;

    while(flag == true){
        

        pthread_mutex_lock(&lock); // critical section enterance

        file = fdopen(value, "r"); // readable mode 

        char buff[100];

        if (file){

            if(fgets(buff, sizeof(buff), file) != NULL){
                
                printf("----------%ld\n", pthread_self());
                printf("%s", buff);
                fsync(STDOUT_FILENO);

                while(fgets(buff, sizeof(buff), file) != NULL){
                    printf("%s", buff);
                    fsync(STDOUT_FILENO);
                }
                    
                printf("----------%ld\n", pthread_self());
                fsync(STDOUT_FILENO);

                flag = false;

            }
        }
        fclose(file);
        pthread_mutex_unlock(&lock); // critical section end
    }

    close(value);
    return NULL;

}

int main(int arg, const char * argv[]){

    ifstream file;
    file.open("commands.txt");

    ofstream parse;
    parse.open("parse.txt");

    string line, word;

    vector<int> processesVector;
    vector<pthread_t> threadsVector;

    while(getline(file, line)){
        istringstream ss(line);
        ss >> word;

        string fileName; // output filename

        char fileDirection = '-' ;
        
        bool isBackground = false; 

        char * command[4] = {NULL, NULL, NULL, NULL};
        command[0] = strdup(word.c_str());

        string option, input;

        int index = 1;

        while(ss >> word){

            if (word == ">"){ // meaning we are now have to redirect output to desired file
                
                fileDirection = '>';

                ss >> word;

                fileName = word; // one word after > so writable file
            }  

            else if (word == "<"){

                fileDirection = '<';

                ss >> word;

                fileName = word; // one word after < so readable file

            }

            else{ // command or &

                if (word == "&"){
                    isBackground = true;
                }
                else{  // meaning it is a command
                    if(word[0] == '-'){
                        option = word;
                    }
                    else{
                        input = word;
                    }
                    command[index] = strdup(word.c_str());
                    index++;
                }
            }
        }

        parse << "----------" << endl;
        parse << "Command: " << command[0] << endl;
        parse << "Input: " << input << endl;
        parse << "Option: " << option << endl;
        parse << "Redirection: " << fileDirection << endl;

        parse << "Background: "; 
        
        if (isBackground){
            parse << "y" << endl;
        } 
        else {
            parse << "n" << endl;
        }

       

        parse << "----------" << endl;  

        parse.flush();  // so that visible immediately


        if (word == "wait"){  // if last word is wait 
            for (int i = 0; i < processesVector.size(); i++){
                waitpid(processesVector[i], NULL, 0);
            }
            processesVector.clear();

            for (int i = 0; i < threadsVector.size(); i++){
                pthread_join(threadsVector[i], NULL);
            }
            threadsVector.clear();
        }

        else{  // meaning other threads then wait or no wait

            if (fileDirection == '<'){
                
                int fd[2];

                pipe(fd);

                pthread_t thread1;

                int rc = fork();

                if(rc < 0){  // meaning couldnot fork
                    cout << "Could not fork" << endl;
                }

                else if (rc == 0){  // meaning child process
                     
                     
                     int inputFile = open(fileName.c_str(), O_RDONLY);
                     
                     dup2(inputFile, STDIN_FILENO);  // make input file to read with dup

                     close(inputFile);

                     close(fd[0]);

                     dup2(fd[1], STDOUT_FILENO); // wrtie to an anonymous file
                     
                     close(fd[1]);

                     execvp(command[0], command);

                } 

                else { // meaning  parent process

                    close(fd[1]);
                    pthread_create(&thread1, NULL, listener, (void*) fd[0] ); // (void*)
                    if(word != "&") {
                        waitpid(rc, NULL, 0);
                        pthread_join(thread1, NULL);
                    }
                    else{
                        processesVector.push_back(rc);
                        threadsVector.push_back(thread1);
                    }
                }

            }

            else if (fileDirection == '>'){
                     
                int rc = fork();

                if(rc < 0){
                    cout << "Fork failed..." << endl;
                }

                else if(rc == 0){

                    int outputFileNumber = open(fileName.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);

                    dup2(outputFileNumber, STDOUT_FILENO);
                    close(outputFileNumber);

                    execvp(command[0], command);
                
                }

                else{
                    if(word != "&"){
                        waitpid(rc, NULL, 0);
                    }
                    else{
                        processesVector.push_back(rc);
                    }
                }
            }

            else{  // if no file direction
                int fd[2];

                pipe(fd);

                pthread_t thread1;

                int rc = fork();


                if(rc < 0){  // meaning couldnot fork
                    cout << "Could not fork" << endl;
                }

                else if (rc == 0){ 

                    close(fd[0]);

                    dup2(fd[1], STDOUT_FILENO); // wrtie to an anonymous file
   
                    close(fd[1]);

                    execvp(command[0], command);

                }
                else{
                    close(fd[1]);
                    pthread_create(&thread1, NULL, listener, (void*) fd[0] );  // implement the lock
                    if(word != "&") {
                        waitpid(rc, NULL, 0);
                        pthread_join(thread1, NULL);
                    }
                    else{
                        processesVector.push_back(rc);
                        threadsVector.push_back(thread1);
                    }
                }
            }


        }

    }

    parse.close();
    file.close();

    for (int i = 0; i < processesVector.size(); i++){
        waitpid(processesVector[i], NULL, 0);
    }

    processesVector.clear();

    for(int i = 0; i < threadsVector.size(); i++){
        pthread_join(threadsVector[i], NULL);
    }

    threadsVector.clear();

    return 0;
}