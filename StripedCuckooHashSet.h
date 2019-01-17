#include "PhasedCuckooHashSet.h"
#include <mutex>
#include <thread>

template <class T>
class StripedCuckooHashSet : public PhasedCuckooHashSet<T> {
    private:
        mutex*** locks;
        mutex* resizeLock;
        int lockNum;

        void acquire(T x){
            mutex* lock0 = locks[0][this->hash0(x) % lockNum];
            mutex* lock1 = locks[1][this->hash1(x) % lockNum];

            while(true){

                lock0->lock();

                if(lock1->try_lock())
                    break;
                else 
                    lock0->unlock();
                
                std::this_thread::yield();
            }

        }

        void release(T x){
            mutex* lock0 = locks[0][this->hash0(x) % lockNum];
            mutex* lock1 = locks[1][this->hash1(x) % lockNum];
            lock1->unlock();
            lock0->unlock();
        }

        void resizeLocks(){
            lockNum *= 2;
            mutex*** newlocks = (mutex***)malloc(sizeof(void*) * 2);

            for(int i = 0; i < 2; ++i){
                newlocks[i] = (mutex**)malloc(sizeof(void*) * lockNum);
                for(int j = 0; j < lockNum / 2; ++j)
                    newlocks[i][j] = locks[i][j];
                for(int j = lockNum / 2; j < lockNum; ++j){
                    newlocks[i][j] = new mutex();
                    if(i==0)newlocks[i][j]->lock();
                }
                this->locks[i] = newlocks[i];
            }
        }

        void resize(int oldCapacity){

            
            for(int i = 0; i < lockNum; ++i){
                while(!locks[0][i]->try_lock())std::this_thread::yield();
            }
            if(oldCapacity != this->capacity){
                for(int i = 0; i < lockNum; ++i){
                    locks[0][i]->unlock();
                }
                return;
            }
            

            cout << oldCapacity << " is full, need to resize" <<endl;

            // this->resizeLocks(); 

            this->resizeTables();

            
            for(int i = 0; i < lockNum; ++i){
                locks[0][i]->unlock();
            }
            
        }

    public:
        StripedCuckooHashSet(int n = 1000, int limit = 10, int threshold = 2, int probe_size = 4) : PhasedCuckooHashSet<T>(n, limit, threshold, probe_size){
            lockNum = n;
            locks = (mutex***)malloc(sizeof(void*) * 2);

            for(int i = 0; i < 2; ++i){
                locks[i] = (mutex**)malloc(sizeof(void*) * lockNum);
                for(int j = 0; j < lockNum; ++j)
                    locks[i][j] = new mutex();
            }
        }
};
