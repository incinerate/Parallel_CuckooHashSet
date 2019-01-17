#ifndef   PHS_H
#define   PHS_H

#include <functional>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;


template <class T>
class PhasedCuckooHashSet{
    public:
        T*** tables[2];
        int capacity;
        bool isResize;

        int N;
        int LIMIT;
        int THRESHOLD;
        int PROBE_SIZE;

        int hash0(T x){
            int hashvalue = hash<T>{}(x);
            return abs((hashvalue * 1567) % 16759);
        }

        int hash1(T x){
            int hashvalue = hash<T>{}(x);
            return abs((hashvalue * 1913) % 19841);
        }

        void resizeTables(){
            isResize = true;

            T*** tmp_tables[2];
            tmp_tables[0] = tables[0];
            tmp_tables[1] = tables[1];

            capacity *= 2;

            tables[0] = (T***)malloc(sizeof(void*) * capacity);
            tables[1] = (T***)malloc(sizeof(void*) * capacity);

            for(int i = 0; i < 2; ++i){
                for(int j = 0; j < capacity; ++j){
                    tables[i][j] = (T**)malloc(sizeof(void*) * PROBE_SIZE);
                    for(int k = 0; k < PROBE_SIZE; ++k)tables[i][j][k] = NULL;
                }
            }

            for(int i = 0; i < 2; ++i){
                for(int j = 0; j < capacity / 2; ++j){
                    for(int z = 0; z < PROBE_SIZE; ++z){
                        if(tmp_tables[i][j][z] != NULL){
                            // add(*tmp_tables[i][j][z], false);
                            resize_add(*tmp_tables[i][j][z]);
                        }
                    }
                }
                free(tmp_tables[i]);
            }
            isResize = false;
        }

        virtual void resize(int oldCapacity) = 0;

        int probe_set_size(T** s){
            if(s == NULL)return 0;
            int res = 0;
            for(int i = 0; i < PROBE_SIZE; ++i)
                if(s[i] != NULL)res++;
            return res;
        }

        bool probe_set_add(T** s, T x){
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(s[i] == NULL){
                    s[i] = (T*) malloc(sizeof(T));
                    *s[i] = x;
                    return true;
                }
            }
            return false;
        }

        bool probe_set_remove(T** s, T x){
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(s[i] != NULL && *s[i] == x){
                    free(s[i]);
                    for(int j = i + 1; j < PROBE_SIZE; ++j)s[j - 1] = s[j];
                    s[PROBE_SIZE - 1] = NULL;
                    return true;
                }
            }
            return false;
        }

        bool probe_set_contains(T** s, T x){
            if(s == NULL)return false;
            for(int i = 0; i < PROBE_SIZE; ++i){
                if(s[i] != NULL && *s[i] == x)return true;
            }
            return false;
        }

        bool relocate(int i, int hi, int oldCapacity, bool useLock = true){

            int hj = 0;
            int j = 1 - i;

            for(int round = 0; round < LIMIT; ++round){
                
                T** iSet = tables[i][hi];

                //resizing
                if(iSet == NULL || iSet[0] == NULL)return true;

                T y = *iSet[0];

                if(useLock)acquire(y);

                // if(useLock && oldCapacity != capacity){
                //     release(y);
                //     return true;
                // }

                if(oldCapacity != capacity){
                    if(useLock)
                        release(y);
                    // cout<<"OK"<<endl;
                    return true;
                }

                

                if(i==0) hj = hash1(y) % capacity;
                else if(i==1) hj = hash0(y) % capacity;

                T** jSet = tables[j][hj];

                if(jSet == NULL){
                    if(useLock)release(y);
                    return false;
                }

                if(!useLock){
                    probe_set_remove(iSet, y);
                    if(probe_set_size(jSet) < THRESHOLD){
                        probe_set_add(jSet, y);
                        return true;
                    }
                    else if(probe_set_size(jSet) < PROBE_SIZE){
                        probe_set_add(jSet, y);
                        i = 1 - i;
                        hi = hj;
                        j = 1 - j;
                    }
                    else{
                        probe_set_add(iSet, y);
                        return false;
                    }
                }

                if(probe_set_remove(iSet, y)){
                    if(probe_set_size(jSet) < THRESHOLD){
                        probe_set_add(jSet, y);
                        if(useLock)release(y);
                        return true;
                    }
                    else if(probe_set_size(jSet) < PROBE_SIZE){
                        probe_set_add(jSet, y);
                        i = 1 - i;
                        hi = hj;
                        j = 1 - j;
                        if(useLock)release(y);
                    }
                    else{
                        probe_set_add(iSet, y);
                        if(useLock)release(y);
                        return false;
                    }
                }
                else if(probe_set_size(iSet) >= THRESHOLD){
                    if(useLock)release(y);
                    continue;
                }
                else{
                    if(useLock)release(y);
                    return true;
                }
            }

            return false;
        }

        virtual void acquire(T x) = 0;

        virtual void release(T x) = 0;

    public:
        PhasedCuckooHashSet(int n = 1000, int limit = 10, int threshold = 2, int probe_size = 4){
            N = n;
            LIMIT = limit;
            THRESHOLD = threshold;
            PROBE_SIZE = probe_size;

            capacity = N;

            tables[0] = (T***)malloc(sizeof(void*) * capacity);
            tables[1] = (T***)malloc(sizeof(void*) * capacity);

            for(int i = 0; i < 2; ++i){
                for(int j = 0; j < capacity; ++j){
                    tables[i][j] = (T**)malloc(sizeof(void*) * PROBE_SIZE);
                    memset (tables[i][j],0,sizeof(void*) * PROBE_SIZE);
                }
            }

            isResize = false;
        }

        bool contains(T x){
            if( probe_set_contains(tables[0][hash0(x) % capacity], x) )return true;
            if( probe_set_contains(tables[1][hash1(x) % capacity], x) )return true;
            return false;
        }

        bool add(T x){
            acquire(x);

            if(contains(x)){
                release(x);
                return false;
            }

            T *tmp = (T*)malloc(sizeof(T));
            *tmp = x;

            int h0 = hash0(x) % capacity, h1= hash1(x) % capacity;
            int i = -1, h = -1;
            bool mustResize = false;

            T** set0 = tables[0][h0];
            T** set1 = tables[1][h1];

            if( probe_set_size(set0) < THRESHOLD){
                probe_set_add(set0, x);
                release(x);
                return true;
            }
            else if( probe_set_size(set1) < THRESHOLD){
                probe_set_add(set1, x);
                release(x);
                return true;
            }
            else if( probe_set_size(set0) < PROBE_SIZE){
                probe_set_add(set0, x);
                i = 0;
                h = h0;
            }
            else if( probe_set_size(set1) < PROBE_SIZE){
                probe_set_add(set1, x);
                i = 1;
                h = h1;
            }
            else{
                mustResize = true;
            }
            int oldCapacity = capacity;
            release(x);

            if(mustResize){
                resize(oldCapacity);
                return add(x);    
            }
            else if(!relocate(i,h,oldCapacity, true)){
                resize(oldCapacity);
            }

            return true;
        }

        bool resize_add(T x){

            if(contains(x)){
                return false;
            }

            T *tmp = (T*)malloc(sizeof(T));
            *tmp = x;

            int h0 = hash0(x) % capacity, h1= hash1(x) % capacity;
            int i = -1, h = -1;
            bool mustResize = false;

            T** set0 = tables[0][h0];
            T** set1 = tables[1][h1];

            if( probe_set_size(set0) < THRESHOLD){
                probe_set_add(set0, x);
                return true;
            }
            else if( probe_set_size(set1) < THRESHOLD){
                probe_set_add(set1, x);
                return true;
            }
            else if( probe_set_size(set0) < PROBE_SIZE){
                probe_set_add(set0, x);
                i = 0;
                h = h0;
            }
            else if( probe_set_size(set1) < PROBE_SIZE){
                probe_set_add(set1, x);
                i = 1;
                h = h1;
            }
            else{
                mustResize = true;
            }
            int oldCapacity = capacity;

            if(mustResize){
                // resize(oldCapacity);
                cout <<"resize  in resize: "<< oldCapacity <<endl;
                resizeTables();
                return resize_add(x);    
            }
            else if(!relocate(i,h,oldCapacity, true)){
                // resize(oldCapacity);
                cout <<"relocated return false while resize: "<< oldCapacity <<endl;
                resizeTables();
                cout<<"resizeTables finished"<<endl;
            }
            return true;
        }

        bool remove(T x){
            acquire(x);
            int h0 = hash0(x) % capacity;
            if(probe_set_remove(tables[0][h0], x)){
                release(x);
                return true;
            }
            int h1 = hash1(x) % capacity;
            if(probe_set_remove(tables[1][h1], x)){
                release(x);
                return true;
            }
            release(x);
            return false;
        }

        int size(){
            int res = 0;

            for(int i = 0; i < capacity; ++i){
                res += probe_set_size(tables[0][i]);
                res += probe_set_size(tables[1][i]);
            }

            return res;
        }

        void populate(){
            int count = 0;

            srand(time(NULL));
            while(count < 1024){
                if(add(rand())){
                    count++;
                }
            }
        }

        void print(){
            for(int  i = 0; i < 2; ++i){
                for(int  j = 0; j < capacity; ++j){
                    if(tables[i][j] != NULL){
                        for(int z = 0; z < PROBE_SIZE; ++z){
                            if(tables[i][j][z] != NULL)cout << *tables[i][j][z] << ",";
                            else cout << "NULL,";
                        }
                    }
                    else{
                        for(int z = 0; z < PROBE_SIZE; ++z){
                            cout << "NULL,";
                        }
                    }
                    cout << endl;
                }
                cout << endl;
            }
        }
};

#endif
