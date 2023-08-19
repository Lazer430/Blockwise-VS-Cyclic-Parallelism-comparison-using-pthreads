#define _GNU_SOURCE 
#include <sys/time.h>
#include <time.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include <math.h>
#include<sched.h>
#include<stdbool.h>

bool useAffinity = true;

struct argumentStructure{
    double* arr;
    int startIndex;
    int endIndex;
    int coreID;
    int threadID;
};

void* populate(void* arg){
    struct argumentStructure* argStr = (struct argumentStructure*) arg;
    double* arr = argStr->arr;
    int startIndex =  argStr->startIndex;
    int endIndex = argStr->endIndex;

    if(useAffinity==true){

        cpu_set_t setOfCPUs;
        CPU_ZERO(&setOfCPUs);
        CPU_SET(argStr->coreID,&setOfCPUs);

        int status = pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&setOfCPUs);

        if(status!=0){
            printf("\n!!!!!!!!!!!!!!!!! Error restricting thread to core %d !!!!!!!!!!!!!!!!!\n",argStr->coreID);
        }

        if(CPU_ISSET(argStr->coreID,&setOfCPUs)!=0){
            printf("Thread %d tid:%lu assigned to core %d\n",argStr->threadID,pthread_self(),argStr->coreID);
        }

    }

    for(int i=startIndex; i<=endIndex; i++){
        arr[i]=(double) i;
    }

}

void* calcSqrt(void* arg){

    struct argumentStructure* argStr = (struct argumentStructure*) arg;

    double* arr = argStr->arr;
    int startIndex =  argStr->startIndex;
    int endIndex = argStr->endIndex;

    if(useAffinity==true){
        cpu_set_t setOfCPUs;
        CPU_ZERO(&setOfCPUs);
        CPU_SET(argStr->coreID,&setOfCPUs);

        int status = pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&setOfCPUs);

        if(status!=0){
            printf("\n!!!!!!!!!!!!!!!!! Error restricting thread to core %d !!!!!!!!!!!!!!!!!\n",argStr->coreID);
        }

        if(CPU_ISSET(argStr->coreID,&setOfCPUs)!=0){
            printf("Thread %d tid:%lu assigned to core %d\n",argStr->threadID,pthread_self(),argStr->coreID);
        }
    }

    for(int i=startIndex; i<=endIndex; i++){
        arr[i]=(double) sqrt(arr[i]);
    }

}

void printResultingArray(double* arr, int sizeOfArr){
    printf("\n\nPrinting Resulting Array: \n");
    int count=0;
    for(int i=0; i<sizeOfArr; i++){
        if(count==8) {
            count=0;
            printf("\n");
        }
        printf("arr[%d]: %.2f",i,arr[i]);

        if(i!=sizeOfArr-1){
            printf(", ");
        }else printf("\n");

        count++;
    }

    printf("\n");
}

int main(int argc, char* argv[]){

    useAffinity=true;

    system("clear");
    printf("Running Q1 Fasih Abdullah i200432 CS-F Please Run Terminal in Full Screen\n\n");

    int numberOfCores = atoi(argv[1]);
    printf("Number of CPU Cores: %d\n",numberOfCores);

    int sizeArr = (int) pow(2,16);
    double* arr = calloc(sizeArr,sizeof(double));
    printf("Size of array is: %d\n",sizeArr);

    pthread_t tid[numberOfCores];
    pthread_t tid2[numberOfCores];

    char c;
    struct argumentStructure argToPass[numberOfCores];
    int startIndex=0;
    int endIndex=sizeArr-1;
    int sizeSteps = sizeArr/numberOfCores;

    for(int i=0; i<numberOfCores; i++){
        argToPass[i].arr=arr;
        argToPass[i].startIndex=startIndex;
        startIndex+=sizeSteps;
        argToPass[i].endIndex=startIndex-1;
        argToPass[i].coreID=i;
        argToPass[i].threadID=i;
    }

    printf("\n");

    for(int i=0; i<numberOfCores; i++){
        printf("Block %d: arr[%d] to arr[%d] \n",i,argToPass[i].startIndex,argToPass[i].endIndex);
    }

    printf("\nUsing Affinity: \n");

    int operation_count=sizeArr*2; //number of op is same as size of array but 2 times since populating and calculating sqrt
    struct timeval start, end;

    gettimeofday( &start, NULL );

    printf("\nPopulating Array\n");
    for(int i=0; i<numberOfCores; i++){
        pthread_create(&tid[i],NULL,populate,(void*)&argToPass[i]); 
    }

    for(int i=0; i<numberOfCores; i++){
        pthread_join(tid[i],NULL);
    }

    printf("\nCalculating Sqrt in Array\n");
    for(int i=0; i<numberOfCores; i++){
        pthread_create(&tid2[i],NULL,calcSqrt,(void*)&argToPass[i]); 
    }

    for(int i=0; i<numberOfCores; i++){
        pthread_join(tid2[i],NULL);
    }

    gettimeofday( &end, NULL );
    
    printf("\nPress enter to print array");
    scanf("%c",&c);
    printResultingArray(arr,sizeArr);

    /* convert time to Gflop/s */
    double seconds = (end.tv_sec - start.tv_sec) +
            1.0e-6 * (end.tv_usec - start.tv_usec);
    double Gflops = 2e-9*operation_count/seconds;
    printf( "Performance in Gflops %.3f Gflop/s\n", Gflops );   // print Gflops for operarions skipping time taken for printing

    printf("\nPress enter to repeat without affinity");
    scanf("%c",&c);
    printf("\n \n");

    pthread_t tid3[numberOfCores];
    pthread_t tid4[numberOfCores];
    arr=NULL;
    arr = calloc(sizeArr,sizeof(double));

    for(int i=0; i<numberOfCores; i++){
        argToPass[i].arr=arr;
    }

    useAffinity=false;
    
    gettimeofday( &start, NULL );
    printf("\nPopulating Array\n");
    for(int i=0; i<numberOfCores; i++){
        pthread_create(&tid3[i],NULL,populate,(void*)&argToPass[i]); 
    }

    for(int i=0; i<numberOfCores; i++){
        pthread_join(tid3[i],NULL);
    }

    // printf("\nPress enter to print array");
    // scanf("%c",&c);
    // printResultingArray(arr,sizeArr);

    printf("\nCalculating Sqrt in Array\n");
    for(int i=0; i<numberOfCores; i++){
        pthread_create(&tid4[i],NULL,calcSqrt,(void*)&argToPass[i]); 
    }

    for(int i=0; i<numberOfCores; i++){
        pthread_join(tid4[i],NULL);
    }
    gettimeofday( &end, NULL );

    printf("\nPress enter to print array");
    scanf("%c",&c);
    printResultingArray(arr,sizeArr);

    /* convert time to Gflop/s */
    double seconds2 = (end.tv_sec - start.tv_sec) +
            1.0e-6 * (end.tv_usec - start.tv_usec);
    double Gflops2 = 2e-9*operation_count/seconds2;

    double mean = (Gflops+Gflops2)/2;
    double diff1 = Gflops-mean;
    double diff2 = Gflops2-mean;
    double variance = (pow(diff1,2)+pow(diff2,2))/1;

    printf( "No Affinity Performance in Gflops %.3f Gflop/s\n", Gflops2);
    printf( "Affinity Performance in Gflops %.3f Gflop/s\n", Gflops);

    printf( "\nMean Performance in Gflops %.3f Gflop/s\n", (Gflops+Gflops2)/2);
    printf( "Difference in Performance in Gflops %.3f Gflop/s\n", (Gflops-Gflops2));
    printf( "Sample Variance: %.5f\n", variance);
    

    printf("\nTime for printing has been ignored\n");
    
    return 0;
}