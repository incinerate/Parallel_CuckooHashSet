#include <functional>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>

using namespace std;

template <class T>
class CuckooHashSet{
    T** tables[2];
    int length;
    int capacity;

    int N;
    int LIMIT;

    private:
    int hash0(T x){
        int hashvalue = hash<T>{}(x);
        // cout<<hashvalue<<endl;
        return abs((hashvalue * 1567) % 16759) % capacity;
    }

    int hash1(T x){
        int hashvalue = hash<T>{}(x);
        return abs((hashvalue * 1913) % 19841) % capacity;
    }

    void resize(){
        capacity *= 2;

        T** tmp_tables[2];
        tmp_tables[0] = tables[0];
        tmp_tables[1] = tables[1];

        tables[0] = (T**)malloc(sizeof(T**) * capacity);
        tables[1] = (T**)malloc(sizeof(T**) * capacity);

        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity; ++j)
                tables[i][j] = NULL;
        }


        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity / 2; ++j){
                if(tmp_tables[i][j] != NULL)
                    add(*tmp_tables[i][j]);
            }
        }
        
        free(tmp_tables[0]);
        free(tmp_tables[1]);
    }

    public:
    CuckooHashSet(int n, int limit){
        N = n;
        LIMIT = limit;

        length = 0;
        capacity = N;
        
        tables[0] = (T**)malloc(sizeof(T**) * capacity);
        tables[1] = (T**)malloc(sizeof(T**) * capacity);

        for(int i = 0; i < 2; ++i){
            for(int j = 0; j < capacity; ++j){
                tables[i][j] = NULL;
            }
        }
    }

    bool contains(T x){
        if(tables[0][hash0(x)] != NULL && *tables[0][hash0(x)] == x)return true;
        if(tables[1][hash1(x)] != NULL && *tables[1][hash1(x)] == x)return true;
        return false;
    }

    bool add(T x){
        if(contains(x))return false;

        T *tmp = (T*)malloc(sizeof(T));
        *tmp = x;

        for(int i = 0; i < LIMIT; ++i){
            if(tables[0][hash0(*tmp)] == NULL){
                tables[0][hash0(*tmp)] = tmp;
                return true;
            }
            else{
                T *t = tables[0][hash0(*tmp)];
                tables[0][hash0(*tmp)] = tmp;
                tmp = t;
            }

            if(tables[1][hash1(*tmp)] == NULL){
                tables[1][hash1(*tmp)] = tmp;
                return true;
            }
            else{
                T *t = tables[1][hash1(*tmp)];
                tables[1][hash1(*tmp)] = tmp;
                tmp = t;
            }
        }

        resize();

        return add(*tmp);
    }

    bool remove(T x){
        if(tables[0][hash0(x)] != NULL && *tables[0][hash0(x)] == x){
            free(tables[0][hash0(x)]);
            tables[0][hash0(x)] = NULL;
            return true;
        }
        if(tables[1][hash1(x)] != NULL && *tables[1][hash1(x)] == x){
            free(tables[1][hash1(x)]);
            tables[1][hash1(x)] = NULL;
            return true;
        }
        return false;
    }

    int size(){
        int res = 0;

        for(int i = 0; i < capacity; ++i){
            if(tables[0][i] != NULL)res++;
            if(tables[1][i] != NULL)res++;
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
            for(int  j = 0; j < N; ++j){
                if(tables[i][j] != NULL)cout << *tables[i][j] << "," ;
                else cout << "NULL" << ",";
            }
            cout << endl;
        }
    }
};
