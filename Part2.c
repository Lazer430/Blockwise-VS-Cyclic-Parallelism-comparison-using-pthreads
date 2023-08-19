#define _GNU_SOURCE 
#include <sys/time.h>
#include <time.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<sched.h>
#include<stdbool.h>

const unsigned int row =  (unsigned int) pow(2,8); 
const unsigned int col = row;
int** arr;

pthread_mutex_t mutex;
bool* rowAvailability;
bool computationLeft=true;

unsigned int total = 0;

struct argumentStructure{
    unsigned int rowStartIndex;
    unsigned int rowEndIndex;
    unsigned int colStartIndex;
    unsigned int colEndIndex;
    unsigned int stepSize;
};

void populateSubFtn(unsigned int rowStartIndex,unsigned int rowEndIndex){
        
    for(int i=rowStartIndex; i<=rowEndIndex; i++){
        rowAvailability[i]=false;
    }
    
    for(unsigned int i=rowStartIndex; i<=rowEndIndex; i++){
            for(unsigned int j=0; j<=col-1; j++){
                arr[i][j] = (rand()%(16001-8000)) + 8000;
            }
        }

}

void sumSubFtn(unsigned int rowStartIndex,unsigned int rowEndIndex){
       
    for(int i=rowStartIndex; i<=rowEndIndex; i++){
        rowAvailability[i]=false;
    }
    
    for(unsigned int i=rowStartIndex; i<=rowEndIndex; i++){
            for(unsigned int j=0; j<=col-1; j++){
                total += arr[i][j];
            }
        }
}

void* populate(void* arg){
    struct argumentStructure* argStr = (struct argumentStructure*) arg;

    unsigned int colStartIndex =  argStr->colStartIndex;
    unsigned int colEndIndex = argStr->colEndIndex;
    unsigned int rowStartIndex =  argStr->rowStartIndex;
    unsigned int rowEndIndex = argStr->rowEndIndex;

        for(unsigned int i=rowStartIndex; i<=rowEndIndex; i++){
            for(unsigned int j=colStartIndex; j<=colEndIndex; j++){
                arr[i][j] = (rand()%(16001-8000)) + 8000;
                // arr[i][j] = 1;
            }
        }

}

void* populateV2(void* arg){
    struct argumentStructure* argStr = (struct argumentStructure*) arg;

    unsigned int colStartIndex =  argStr->colStartIndex;
    unsigned int colEndIndex = argStr->colEndIndex;
    unsigned int rowStartIndex =  argStr->rowStartIndex;
    unsigned int rowEndIndex = argStr->rowEndIndex;

    // populateSubFtn(rowStartIndex,rowEndIndex);    
    
    while(computationLeft==true){

        while(pthread_mutex_trylock(&mutex)!=0){
            // printf("Thread %ld is waiting for mutex\n",pthread_self());
        }
        bool isLocked=true;

        computationLeft=false;
        int firstIndex=0;
        for(int i=0; i<row; i++){
            if(rowAvailability[i]==true){
                computationLeft=true;
                firstIndex=i;
                break;
            }
        }

        if(computationLeft==true){
            printf("Thread %ld got mutex assigned 2^4 new rows\n",pthread_self());
            int lastIndex = firstIndex+argStr->stepSize-1;
            for(int i=firstIndex; i<=lastIndex; i++) rowAvailability[i]=false;
            // isLocked=false;
            // pthread_mutex_unlock(&mutex);
            populateSubFtn(firstIndex,lastIndex);
        }

        // if(isLocked==true)
        pthread_mutex_unlock(&mutex);
    }

}

void* sum(void* arg){
    struct argumentStructure* argStr = (struct argumentStructure*) arg;

    unsigned int colStartIndex =  argStr->colStartIndex;
    unsigned int colEndIndex = argStr->colEndIndex;
    unsigned int rowStartIndex =  argStr->rowStartIndex;
    unsigned int rowEndIndex = argStr->rowEndIndex;

        for(unsigned int i=rowStartIndex; i<=rowEndIndex; i++){
            for(unsigned int j=colStartIndex; j<=colEndIndex; j++){
                total += arr[i][j];
            }
        }
}

void* sumV2(void* arg){
    struct argumentStructure* argStr = (struct argumentStructure*) arg;

    unsigned int colStartIndex =  argStr->colStartIndex;
    unsigned int colEndIndex = argStr->colEndIndex;
    unsigned int rowStartIndex =  argStr->rowStartIndex;
    unsigned int rowEndIndex = argStr->rowEndIndex;

    // sumSubFtn(rowStartIndex,rowEndIndex);

    while(computationLeft==true){

        while(pthread_mutex_trylock(&mutex)!=0){
            // printf("Thread %ld is waiting for mutex\n",pthread_self());
        }

        bool isLocked = true;
        computationLeft=false;
        int firstIndex=0;
        for(int i=0; i<row; i++){
            if(rowAvailability[i]==true){
                computationLeft=true;
                firstIndex=i;
                break;
            }
        }

        if(computationLeft==true){
            printf("Thread %ld got mutex assigned 2^4 new rows\n",pthread_self());
            int lastIndex = firstIndex+argStr->stepSize-1;
            for(int i=firstIndex; i<=lastIndex; i++) rowAvailability[i]=false;
            isLocked=false;
            // pthread_mutex_unlock(&mutex);
            sumSubFtn(firstIndex,lastIndex);
        }
        // if(isLocked==true)
        pthread_mutex_unlock(&mutex);
    }

}


int** makeArr(){
    
    rowAvailability=calloc(row,sizeof(bool));

    int** arr = calloc(row,sizeof(int*));
    
    for(unsigned int i=0; i<row; i++){
        arr[i]= calloc(col,sizeof(int));
    }

    return arr;
}

void printArr(){
    int count=0;
    for(unsigned int i=0; i<row; i++){
        for(int j=0; j<col; j++){
            printf("Arr[%u][%u]: %d      ",i,j,arr[i][j]);
            count++;
            if(count==4) {
                printf("\n");
                count=0;
            }
        }
    }
}

double initiateComputationV1(unsigned int stepSize){    // divisions into square 2D matrices and stepsize = row = col
    unsigned int startIndex = 0;
    unsigned int endIndex = col-1;

    const unsigned int numberOfThreads = (row*col)/(stepSize*stepSize);
    printf("\nNumber of threads will be: %u\n",numberOfThreads);
    pthread_t t[numberOfThreads];
    struct argumentStructure argToPass[numberOfThreads];

    unsigned int rowIters = row/stepSize;
    unsigned int curRowIndex=0;
    unsigned int threadCount = 0;

    printf("Populating Array\n");

    int operation_count=col*row*2;//number of op
    struct timeval start, end;

    gettimeofday( &start, NULL );

    for(unsigned int i=0; i<rowIters; i++){
        startIndex=0;
        while(startIndex+stepSize-1<=endIndex){
            argToPass[threadCount].rowStartIndex=curRowIndex;
            argToPass[threadCount].rowEndIndex=curRowIndex+stepSize-1;
            argToPass[threadCount].colStartIndex=startIndex;
            argToPass[threadCount].colEndIndex=startIndex+stepSize-1;
            startIndex=argToPass[threadCount].colEndIndex+1;
            pthread_create(&t[threadCount],NULL,populate,(void*)&argToPass[threadCount]);
            threadCount++;
        }
        curRowIndex+=stepSize;
    }

    // printf("Now joining Threads\n");
    for(unsigned int i=0; i<threadCount; i++){
        pthread_join(t[i],NULL);
    }

    printf("Now computing sum\n");

    pthread_t t2[numberOfThreads];

    startIndex = 0;
    curRowIndex=0;
    threadCount = 0;

    for(unsigned int i=0; i<rowIters; i++){
        startIndex=0;
        while(startIndex+stepSize-1<=endIndex){
            argToPass[threadCount].rowStartIndex=curRowIndex;
            argToPass[threadCount].rowEndIndex=curRowIndex+stepSize-1;
            argToPass[threadCount].colStartIndex=startIndex;
            argToPass[threadCount].colEndIndex=startIndex+stepSize-1;
            startIndex=argToPass[threadCount].colEndIndex+1;
            pthread_create(&t2[threadCount],NULL,sum,(void*)&argToPass[threadCount]);
            threadCount++;
        }
        curRowIndex+=stepSize;
    }

    for(unsigned int i=0; i<threadCount; i++){
        pthread_join(t2[i],NULL);
    }

    gettimeofday( &end, NULL );

    /* convert time to Gflop/s */
    double seconds = (end.tv_sec - start.tv_sec) +
            1.0e-6 * (end.tv_usec - start.tv_usec);
    double Gflops = 2e-9*operation_count/seconds;
    
    printf("\nAll computations done printing array:\n");
    printArr();

    printf("\nThe sum is: %u\n",total);
    printf( "Performance in Gflops %.8f Gflop/s\n", Gflops);

    return Gflops;
}

double initiateComputationV2(unsigned int stepSize){    // divisions into square 2D matrices and stepsize = row = col
    
    for(int i=0; i<row; i++) rowAvailability[i]=true;

    pthread_mutex_init (&mutex,NULL);
    
    unsigned int startIndex = 0;
    unsigned int endIndex = col-1;

    const unsigned int numberOfThreads = 2;
    pthread_t t[numberOfThreads];
    pthread_t t2[numberOfThreads];
    struct argumentStructure argToPass[numberOfThreads];

    unsigned int curRowIndex=0;

    printf("Populating Array\n");

    int operation_count=col*row*2;//number of op
    struct timeval start, end;

    gettimeofday( &start, NULL );

    for(int i=0; i<numberOfThreads; i++){
        argToPass[i].rowStartIndex=curRowIndex;
        argToPass[i].rowEndIndex=curRowIndex+stepSize-1;
        argToPass[i].colStartIndex=0;
        argToPass[i].colEndIndex=col-1;
        argToPass[i].stepSize=stepSize;
        curRowIndex+=stepSize;
        pthread_create(&t[i],NULL,populateV2,(void*)&argToPass[i]);
    }

    // printf("Now joining Threads\n");
    for(unsigned int i=0; i<numberOfThreads; i++){
        pthread_join(t[i],NULL);
    }

    printf("\nNow calculating sum\n");

    computationLeft=true;

    for(int i=0; i<row; i++) rowAvailability[i]=true;

    curRowIndex=0;
    for(int i=0; i<numberOfThreads; i++){
        argToPass[i].rowStartIndex=curRowIndex;
        argToPass[i].rowEndIndex=curRowIndex+stepSize-1;
        argToPass[i].colStartIndex=0;
        argToPass[i].colEndIndex=col-1;
        argToPass[i].stepSize=stepSize;
        curRowIndex+=stepSize;
        pthread_create(&t2[i],NULL,sumV2,(void*)&argToPass[i]);
    }

    // printf("Now joining Threads\n");
    for(unsigned int i=0; i<numberOfThreads; i++){
        pthread_join(t2[i],NULL);
    }

    gettimeofday( &end, NULL );

    /* convert time to Gflop/s */
    double seconds = (end.tv_sec - start.tv_sec) +
            1.0e-6 * (end.tv_usec - start.tv_usec);
    double Gflops = 2e-9*operation_count/seconds;
    
    printf("\nAll computations done printing array:\n");
    char c;
    printf("Press enter to print array.\nGflops calculated halting to show cyclic fashion of version 2.\n");
    scanf("%c",&c);
    printArr();

    printf("\nThe sum is: %u\n",total);
    printf( "Performance in Gflops %.8f Gflop/s\n", Gflops);

    pthread_mutex_destroy(&mutex);

    return Gflops;
}

int main(int argc, char* argv[]){

    system("clear");
    printf("Running Q2 Fasih Abdullah i200432 CS-F Note Array Printing is not included in Gflop calculation\n\n");

    arr = makeArr();

    double gflop1,gflop2,gflop3,gflop4;

    unsigned int stepSize = (unsigned int) pow(2,2); 


    printf("\nVersion 1:");
    printf("\nCase 1: Divide into 2*2 Blocks");
    gflop1 = initiateComputationV1(stepSize);
    total=0;
    
    stepSize = (unsigned int) pow(2,4);
    printf("\nCase 2: Divide into 4*4 Blocks"); 
    gflop2 = initiateComputationV1(stepSize);
    total=0;
    
    stepSize = (unsigned int) pow(2,8);;
    printf("\nCase 3: Divide into 8*8 Blocks"); 
    gflop3 = initiateComputationV1(stepSize);
    total=0;

    double mean = (double)(gflop1+gflop2+gflop3)/3.0;
    double diff1 = gflop1-mean;
    double diff2 = gflop2-mean;
    double diff3 = gflop3-mean;
    double variance = (pow(diff1,2)+pow(diff2,2)+pow(diff3,2))/2;
    
    stepSize = (unsigned int) pow(2,4);
    printf("\n=================================================================\n");
    printf("\nVersion 2:\n");
    gflop4 = initiateComputationV2(stepSize);


    printf("\n=================================================================\n");
    printf("Version 1:\n");
    printf("The Gflop results are: \n");
    printf("Case 2^2*2^2: Gflops: %.8f\n",gflop1);
    printf("Case 2^4*2^4: Gflops: %.8f\n",gflop2);
    printf("Case 2^8*2^8: Gflops: %.8f\n",gflop3);
    printf("Mean Gflops: %.8f\n",mean);
    printf("Sample Variance: %.8f\n",variance);
    printf("\nVersion 2: (2^4 thread division in cyclic)\n");
    printf("Gflops: %.8f",gflop4);
    printf("\n=================================================================\n");

    return 0;
}