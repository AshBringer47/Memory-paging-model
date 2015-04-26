#include "processqueue.h"

#include "mprocess.h"

#include <exception>

ProcessQueue::ProcessQueue(mmu_ptr MMU) :
    _MMU(MMU),
    _threadWorking(true),
    _checkingLookWork(false),
    _tickCount(0),
    _queueThread(&ProcessQueue::processingQueue, this)
{
    _sleepTime.store(std::chrono::milliseconds(10));
    _queueThread.detach();
}

ProcessQueue::~ProcessQueue()
{

}

void ProcessQueue::addProcess(std::shared_ptr<Program> process)
{
    if(process) {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_processList.count(process->pid()) == 0) {
            _processList[process->pid()] = process;
        } else {
            throw std::invalid_argument("duplicated process in ProcessQueue::addProcess.");
        }

    } else {
        throw std::invalid_argument("nullptr in ProcessQueue::addProcess.");
    }
}

void ProcessQueue::run()
{
    _checkingLookWork = true;
}

void ProcessQueue::stop()
{
    _checkingLookWork = false;
}

unsigned long ProcessQueue::tickCount() const
{
    return _tickCount;
}

void ProcessQueue::setSleepTime(std::chrono::milliseconds time)
{
    _sleepTime.store(time);
}

std::chrono::milliseconds ProcessQueue::getSleepTime() const
{
    return _sleepTime;
}

process_ptr ProcessQueue::getProcess(processId_tp pid)
{
    if(_processList.count(pid) == 1) {
        return _processList.at(pid);
    } else {
        return process_ptr();
    }
}

#include <iostream>
void ProcessQueue::processingQueue()
{
    while(_threadWorking) {
        if(_checkingLookWork && !_processList.empty()) {
            _tickCount++;
            _mutex.lock();
            for(auto it = std::begin(_processList); it != std::end(_processList); ++it) {
                std::shared_ptr<Program> process = it->second;

                _MMU->bindProcess(process);
                try {
                    process->tick(_MMU);
                } catch(exitInterrupt) {
                    _MMU->releaseProcessPages();
                    it = _processList.erase(it);
                    std::cout << _processList.size() << " processes left\n";

                    if(_processList.empty()) {
                        std::cout << "Empty process queue\n";
                    }
                }
                _MMU->releaseProcess();

                if(_tickCount % constants::TICKS_TO_RESET_R_BIT == 0) {
                    _MMU->resetBitR();
                }
            }
            _mutex.unlock();
        }
        std::this_thread::sleep_for(_sleepTime.load());
    }
}

