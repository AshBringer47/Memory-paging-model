#ifndef PROCESSQUEUE_H
#define PROCESSQUEUE_H

#include "my_global.h"
#include "mmu.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <map>

class Program;

class ProcessQueue
{
public:
    ProcessQueue(mmu_ptr MMU);
    ~ProcessQueue();

    void addProcess(std::shared_ptr<Program> process);
    process_ptr getProcess(processId_tp pid);

    void run();
    void stop();

    unsigned long tickCount() const;

    void setSleepTime(std::chrono::milliseconds time);
    std::chrono::milliseconds getSleepTime() const;

private:
    mmu_ptr _MMU;
    std::map<processId_tp, std::shared_ptr<Program>> _processList;

    std::atomic_bool  _threadWorking;
    std::atomic_bool  _checkingLookWork;
    std::atomic_ullong _tickCount;

    void processingQueue();
    std::atomic<std::chrono::milliseconds> _sleepTime;

    std::thread _queueThread;
    std::mutex _mutex;
};

#endif // PROCESSQUEUE_H
