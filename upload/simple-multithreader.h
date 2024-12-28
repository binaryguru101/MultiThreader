#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <chrono>

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

typedef struct{
  std::function<void(int)> lambda;
  int start,end;
}Vector_thread_args;


typedef struct{
  std::function<void(int,int)> lambda;
  int first_start,first_end;
  int second_start,second_end;
}Matrix_thread_args;


int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main

void* vector_threading (void *values){
    Vector_thread_args *value= (Vector_thread_args*)values;
    int i = value->start;
    
    while(i < value->end) {
        value->lambda(i);
        i++;
    }
  return NULL;
}

void* matrix_threading(void *values){
    Matrix_thread_args *value = (Matrix_thread_args*)values;
    int i = value->first_start;
    while(i<value->first_end){
      int j = value->second_start;
      while(j<value->second_end){
        value->lambda(i,j);
        j++;
      }
      i++;
    }
    return NULL;

}

int parallel_for(int start,
                 int end,
                 std::function<void(int)> &&lambda, int n_threads) {

    auto start_time = std::chrono::high_resolution_clock::now();

    int ind_S = start; 
    const int total_numbers = end - start;
    const int rem = total_numbers % n_threads; 
    const int division = total_numbers / n_threads;

    Vector_thread_args args[n_threads]; 
    pthread_t thread_id[n_threads];     

    for (int i = 0; i < n_threads; i++) {
        
        int chunk = division + (i < rem ? 1 : 0);

        
        args[i].lambda = lambda;
        args[i].start = ind_S;
        args[i].end = ind_S + chunk;

        
        ind_S += chunk;

        if (pthread_create(&thread_id[i], NULL, vector_threading, (void*)&args[i]) != 0) {
            
            for (int j = 0; j < i; j++) {
                pthread_join(thread_id[j], NULL);
            }
            return 1; 
        }
    }

    
    for (int i = 0; i < n_threads; i++) {
        pthread_join(thread_id[i], NULL);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "Total execution time for parallel_for 1d: " 
              << duration.count() << " seconds " << duration.count()*1000<<" milliseconds"<< std::endl;

    return 0; 
}



void parallel_for(int low1D, int high1D, int low2D, int high2D, std::function<void(int, int)> lambda, int numThreads)
{ 
    auto start_time = std::chrono::high_resolution_clock::now();

    int ind_S = low1D;

    int totalRows = high1D - low1D;
    int rem = totalRows % numThreads; 
    int div_size = totalRows / numThreads;

    pthread_t Thread_ID[numThreads];
    Matrix_thread_args threadData[numThreads];
    
    for (int i = 0; i < numThreads; ++i)
    {
        int Division = div_size + (i < rem ? 1 : 0); 
        threadData[i].first_start = ind_S;
        threadData[i].first_end = ind_S + Division;

        threadData[i].second_start = low2D;
        threadData[i].second_end = high2D;
        threadData[i].lambda = lambda;


        pthread_create(&Thread_ID[i], NULL, matrix_threading, static_cast<void *>(&threadData[i]));

        ind_S += Division;
    }

    
    for (int i = 0; i < numThreads; ++i)
    {
        pthread_join(Thread_ID[i], NULL);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();


    std::chrono::duration<double> duration = end_time - start_time;

    std::cout << "Total execution time for parallel_for 2d: " 
              << duration.count() << " seconds" <<duration.count()*1000<<" milliseconds"<<std::endl;

  
}




