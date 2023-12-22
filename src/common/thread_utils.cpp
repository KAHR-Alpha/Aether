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

#include <iostream>

#include <thread_utils.h>

std::mutex out_mutex;

template<typename T>
void var_cout(T const &A)
{
    std::cout<<A<<std::endl;
}

template<typename T,typename... Args>
void var_cout(T const &A,Args... args)
{
    std::cout<<A<<" ";
    var_cout(args...);
}

template<typename... Args>
void thr_cout(Args... args)
{
    out_mutex.lock();
    var_cout(args...);
    out_mutex.unlock();
}

//#######################
//   ThreadsAlternator
//#######################

class TAPredicateVI
{
    public:
        int i;
        std::vector<bool> &condition;
        
        TAPredicateVI(int i_,std::vector<bool> &condition_)
            :i(i_), condition(condition_)
        { }
        bool operator() ()
        {
            return condition[i];
        }
};
        
class TAPredicateV
{
    public:
        std::vector<bool> &condition;
        
        TAPredicateV(std::vector<bool> &condition_)
            :condition(condition_)
        { }
        bool operator() ()
        {
            bool r=true;
            for(unsigned int i=0;i<condition.size();i++)
                r*=condition[i];
            #ifdef THR_DEBUG
            thr_cout("Predicate Main",r);
            #endif
            return r;
        }
};

ThreadsAlternator::ThreadsAlternator(int Nthr_)
    :Nthr(Nthr_),
     thread_mutex(Nthr),
     #ifdef THR_NOTIFY_SEPARATE
     threads_cv(Nthr),
     #endif
     threads_allow_run(Nthr),
     threads_waiting_val(Nthr)
{
    for(unsigned int i=0;i<Nthr;i++)
    {
        threads_allow_run[i]=false;
        threads_waiting_val[i]=false;
    }
}

bool ThreadsAlternator::all_threads_waiting()
{
    std::lock_guard<std::mutex> guard(threads_waiting_mutex);
    
    bool r=true;
    
    for(unsigned int i=0;i<Nthr;i++) r*=threads_waiting_val[i];
    
    return r;
}

void ThreadsAlternator::allow_thread_running(unsigned int ID,bool var)
{
    std::lock_guard<std::mutex> guard(threads_allow_run_mutex);
    
    threads_allow_run[ID]=var;
}

void ThreadsAlternator::allow_all_threads_running(bool var)
{
    std::lock_guard<std::mutex> guard(threads_allow_run_mutex);
    
    for(unsigned int i=0;i<Nthr;i++) threads_allow_run[i]=var;
}

std::mutex& ThreadsAlternator::get_main_mutex() { return main_mutex; }

int ThreadsAlternator::get_N_threads() { return Nthr; }

void ThreadsAlternator::main_wait_threads(std::unique_lock<std::mutex> &lock)
{
    #ifdef THR_DEBUG
    thr_cout(this,"Main waiting Start");
    #endif
    
    Predicate_Main p(this);
    
    main_cv.wait(lock,p);
    
    set_all_threads_waiting(false);
    
    #ifdef THR_DEBUG
    thr_cout(this,"Main waiting End");
    #endif
}

void ThreadsAlternator::set_all_threads_waiting(bool val)
{
    std::lock_guard<std::mutex> guard(threads_waiting_mutex);
    
    for(unsigned int i=0;i<Nthr;i++) threads_waiting_val[i]=val;
}

void ThreadsAlternator::set_thread_waiting(unsigned int ID,bool val)
{
    std::lock_guard<std::mutex> guard(threads_waiting_mutex);
    
    threads_waiting_val[ID]=val;
}

void ThreadsAlternator::signal_main(unsigned int ID)
{
    #ifdef THR_DEBUG
    thr_cout(this,"Thread",ID,"Signal main lock Start");
    #endif
    
    main_mutex.lock();
    
    #ifdef THR_DEBUG
    thr_cout(this,"Thread",ID,"Signal main lock End");    
    thr_cout("Nthr:",Nthr);
    #endif
    
    if(thread_last_running(ID))
    {
        set_thread_waiting(ID,true);
        main_cv.notify_one();
    }
    else set_thread_waiting(ID,true);
    
    #ifdef THR_DEBUG
    thr_cout(this,"Thread",ID,"Signal main Unlock");
    #endif
    
    main_mutex.unlock();
}

void ThreadsAlternator::signal_threads()
{
    #ifdef THR_DEBUG
    thr_cout(this,"Main signal threads lock Start");
    #endif
    
    for(unsigned int i=0;i<Nthr;i++) thread_mutex[i].lock();
    
    #ifdef THR_DEBUG
    thr_cout(this,"Main signal threads lock End");
    #endif
    
    allow_all_threads_running();
    #ifdef THR_NOTIFY_SEPARATE
    for(unsigned int i=0;i<Nthr;i++) threads_cv[i].notify_one();
    #else
    threads_cv.notify_all();
    #endif
    
    for(unsigned int i=0;i<Nthr;i++) thread_mutex[i].unlock();
        
    #ifdef THR_DEBUG
    thr_cout(this,"Main signal threads lock Unlock");
    #endif
}

std::mutex& ThreadsAlternator::get_thread_mutex(unsigned int ID)
{
    return thread_mutex[ID];
}

bool ThreadsAlternator::thread_last_running(unsigned int ID)
{
    bool r=true;
    
    for(unsigned int i=0;i<ID;i++)
    {
        r*=threads_waiting_val[i];
    }
    
    for(unsigned int i=ID+1;i<Nthr;i++)
    {
        r*=threads_waiting_val[i];
    }
    
    return r;
}

bool ThreadsAlternator::thread_running_allowed(unsigned int ID)
{
    std::lock_guard<std::mutex> guard(threads_allow_run_mutex);
    
    return threads_allow_run[ID];
}

void ThreadsAlternator::thread_wait_ok(unsigned int ID,std::unique_lock<std::mutex> &lock)
{
    #ifdef THR_DEBUG
    thr_cout(this,"Thread",ID,"waiting Start");
    #endif
    
    Predicate_Thread p(this,ID);
    
    #ifdef THR_DEBUG
    if(threads_allow_run[ID]==true) thr_cout("BUG");
    #endif
    
    #ifdef THR_NOTIFY_SEPARATE
    threads_cv[ID].wait(lock,p);
    #else
    threads_cv.wait(lock,p);
    #endif
    
    #ifdef THR_DEBUG
    if(threads_allow_run[ID]==false) thr_cout("BUG_");
    #endif
    
    allow_thread_running(ID,false);
    
    #ifdef THR_DEBUG
    thr_cout(this,"Thread",ID,"waiting End");
    #endif
}

//#################
//   ThreadsPool
//#################

ThreadsPool::ThreadsPool()
    :Nthr(1), Rthr(0),
     threads_running(Nthr,false),
     threads(Nthr,nullptr)
{
}

ThreadsPool::ThreadsPool(int Nthr_)
    :Nthr(Nthr_), Rthr(0),
     threads_running(Nthr,false),
     threads(Nthr,nullptr)
{
}

ThreadsPool::~ThreadsPool()
{
    join_threads();
}

void ThreadsPool::join_threads()
{
    for(int i=0;i<Nthr;i++)
    {
        if(threads[i]!=nullptr)
        {
            threads[i]->join();
            delete threads[i];
            
            threads[i]=nullptr;
        }
        
        threads_running[i]=false;
    }
    
    Rthr=0;
}

void ThreadsPool::resize_pool(int Nthr_)
{
    Nthr=Nthr_;
    threads_running.resize(Nthr);
    threads.resize(Nthr);
    
    for(int i=0;i<Nthr;i++)
    {
        threads_running[i]=false;
        threads[i]=nullptr;
    }
}

//

int max_threads_number()
{
    #ifndef MAX_NTHR
        return std::thread::hardware_concurrency();
    #else
        return std::min(std::thread::hardware_concurrency(),static_cast<unsigned int>(MAX_NTHR));
    #endif
}
