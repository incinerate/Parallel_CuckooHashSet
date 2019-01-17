#include "StripedCuckooHashSet.h"
#include "CuckooHashSet.h"
//#include "TransactionalCuckooHashSet.h"
#include "TransactionalPhasedCuckooHashSet.h"
#include <thread>
#include <iostream>
#include <sys/time.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <stdlib.h>

using namespace std;

int** records;

void do_work(int id, StripedCuckooHashSet<int>* hs, int OPERATIONS){
    //cout << OPERATIONS << endl;
    for(int i = 0; i < OPERATIONS; ++i){
        if(abs(rand() % 10) < 4){
            if(hs->remove(rand()))records[id][0]++;
            else records[id][1]++;
        }
        else{
            if(hs->add(rand()))records[id][2]++;
            else records[id][3]++;
        }
    }
}

void do_work1(int id, TransactionalPhasedCuckooHashSet<int>* hs, int OPERATIONS){
    //cout << OPERATIONS << endl;
    for(int i = 0; i < OPERATIONS; ++i){
        if(abs(rand() % 10) < 4){
            if(hs->remove(rand()))records[id][0]++;
            else records[id][1]++;
        }
        else{
            if(hs->add(rand()))records[id][2]++;
            else records[id][3]++;
        }
    }
}

int main(int argc, char *argv[]){

    int thread_num = 100;
    int n = 2;
    int limit = 100;
    int threshold = 1;
    int probe_size = 2;
    int OPERATIONS = 20000;

    int expect = 0;
    struct timeval start;
    struct timeval end;
    int remains;

    thread* threads[thread_num];

    records = (int**)malloc(sizeof(void*) * thread_num);
    for(int i = 0; i < thread_num; ++i){
        records[i] = (int*)malloc(sizeof(int) * 4);
        memset(records[i], 0, sizeof(int) * 4);
    }

    srand(time(0));
    
    CuckooHashSet<int>* chs = new CuckooHashSet<int>(n, limit);
    //chs->populate();
    gettimeofday(&start, NULL);
    for(int i = 0; i < OPERATIONS; ++i){
        if(abs(rand() % 10) < 4){
            chs->remove(rand());
        }
        else{
            chs->add(rand());
        }
    }
    gettimeofday(&end, NULL);

    cout << "Seq:" << endl; 
    cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    cout << "size: " << chs->size() << endl;
    // chs->print();
    //cout << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;

    free(chs);

    StripedCuckooHashSet<int>* hs = new StripedCuckooHashSet<int>(n, limit, threshold, probe_size);
    //hs->populate();

    remains = OPERATIONS;

    for(int i = 0; i < thread_num; ++i){
        if(i < thread_num - 1)threads[i] = new thread(do_work, i, hs, (int)(OPERATIONS / thread_num));
        else threads[i] = new thread(do_work, i, hs, remains);
        remains -= (int)(OPERATIONS / thread_num);
    }

    gettimeofday(&start, NULL);

    for(int i = 0; i < thread_num; ++i)
        threads[i]->join();

    gettimeofday(&end, NULL);

    for(int i = 0; i < thread_num; ++i){
        expect = expect - records[i][0] + records[i][2];
    }

    cout << "Concurrent:" << endl; 
    cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    cout << "operation failed: " << hs->size() - expect << endl;
    cout << "size: " << hs->size() << endl;
    // hs->print();
    //cout << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;

    free(hs);

    TransactionalPhasedCuckooHashSet<int>* ths = new TransactionalPhasedCuckooHashSet<int>(n, limit, threshold, probe_size);
    //ths->populate();

    records = (int**)malloc(sizeof(void*) * thread_num);
    for(int i = 0; i < thread_num; ++i){
        records[i] = (int*)malloc(sizeof(int) * 4);
        memset(records[i], 0, sizeof(int) * 4);
    }

    remains = OPERATIONS;
    for(int i = 0; i < thread_num; ++i){
        if(i < thread_num - 1)threads[i] = new thread(do_work1, i, ths, OPERATIONS / thread_num);
        else threads[i] = new thread(do_work1, i, ths, remains);
        remains -= (int)(OPERATIONS / thread_num);
    }

    gettimeofday(&start, NULL);

    for(int i = 0; i < thread_num; ++i)
        threads[i]->join();

    gettimeofday(&end, NULL);

    expect = 0;
    for(int i = 0; i < thread_num; ++i){
        expect = expect - records[i][0] + records[i][2];
    }

    cout << "TransactionalHashSet:" << endl; 
    cout << "total time: " << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;
    cout << "operation failed: " << ths->size() - expect << endl;
    cout << "size: " << ths->size() << endl;
    //ths->print();
    //cout << (end.tv_sec - start.tv_sec) * 1000000 + ((int)end.tv_usec - (int)start.tv_usec) << endl;

    free(ths);

    return 0;
}
