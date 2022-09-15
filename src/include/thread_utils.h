/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class ThreadsAlternator
{
    private:
        unsigned int Nthr;
        
        std::mutex main_mutex,
                   threads_allow_run_mutex,
                   threads_waiting_mutex;
        std::vector<std::mutex> thread_mutex;
        
        #ifdef THR_NOTIFY_SEPARATE
        std::vector<std::condition_variable> threads_cv;
        #else
        std::condition_variable threads_cv;
        #endif
        std::vector<bool> threads_allow_run;
        
        std::condition_variable main_cv;
        std::vector<bool> threads_waiting_val;
        
        bool all_threads_waiting();
        void allow_all_threads_running(bool var=true);
        void allow_thread_running(unsigned int ID,bool var);
        void set_all_threads_waiting(bool val=false);
        void set_thread_waiting(unsigned int ID,bool val=true);
        bool thread_last_running(unsigned int ID);
        bool thread_running_allowed(unsigned int ID);
        
        class Predicate_Main
        {
            public:
                ThreadsAlternator *TA;
                
                Predicate_Main(ThreadsAlternator *TA_)
                    :TA(TA_)
                { }
                bool operator() () { return TA->all_threads_waiting(); }
        };
        
        class Predicate_Thread
        {
            public:
                ThreadsAlternator *TA;
                unsigned int ID;
                
                Predicate_Thread(ThreadsAlternator *TA_,unsigned int ID_)
                    :TA(TA_), ID(ID_)
                { }
                bool operator() () { return TA->thread_running_allowed(ID); }
        };
    
    public:
        ThreadsAlternator(int Nthr);
        
        std::mutex& get_main_mutex();
        int get_N_threads();
        std::mutex& get_thread_mutex(unsigned int ID);
        void main_wait_threads(std::unique_lock<std::mutex> &lock);
        void signal_main(unsigned int ID);
        void signal_threads();
        void thread_wait_ok(unsigned int ID,std::unique_lock<std::mutex> &lock);
};

class ThreadsPool
{
    public:
        int Nthr,Rthr;
        
        std::vector<bool> threads_running;
        std::vector<std::thread*> threads;
        
        std::mutex lock;
        
        ThreadsPool();
        ThreadsPool(int Nthr);
        ~ThreadsPool();
        
        void resize_pool(int Nthr);
        
        template<typename F,class... Args>
        void deep_run(int ID,F f,Args... args)
        {
            (*f)(args...);
            
            lock.lock();
            
            Rthr--;
            threads_running[ID]=false;
            
            lock.unlock();
        }
        
        template<class C,class... Args>
        void deep_run_m(int ID,void (C::*f)(Args...),C *c,Args... args)
        {
            ((*c).*f)(args...);
            
            lock.lock();
            
            Rthr--;
            threads_running[ID]=false;
            
            lock.unlock();
        }
        
        void join_threads();
        
        template<typename F,class... Args>
        void run(F f,Args... args)
        {
            while(true)
            {
                lock.lock();
                
                if(Rthr<Nthr)
                {
                    int i;
                    for(i=0;i<Nthr;i++)
                    {
                        if(threads_running[i]==false) break;
                    }
                    
                    if(threads[i]!=nullptr)
                    {
                        threads[i]->join();
                        delete threads[i];
                    }
                    
                    threads[i]=new std::thread(deep_run<F,Args...>,this,i,f,args...);
                    threads_running[i]=true;
                    Rthr++;
                    
                    lock.unlock();
                    break;
                }
                else 
                {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
        
        template<class C,class... Args>
        void run_m(void (C::*f)(Args...),C *c,Args... args)
        {
            while(true)
            {
                lock.lock();
                
                if(Rthr<Nthr)
                {
                    int i;
                    for(i=0;i<Nthr;i++)
                    {
                        if(threads_running[i]==false) break;
                    }
                    
                    if(threads[i]!=nullptr)
                    {
                        threads[i]->join();
                        delete threads[i];
                    }
					
					void (ThreadsPool::*f_sub)(int,void (C::*)(Args...),C*,Args...)=
						&ThreadsPool::deep_run_m<C,Args...>;
                    
                    threads[i]=new std::thread(f_sub,this,i,f,c,args...);
                    threads_running[i]=true;
                    Rthr++;
                    
                    lock.unlock();
                    break;
                }
                else 
                {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        }
};

int max_threads_number();

#endif // THREAD_UTILS_H
