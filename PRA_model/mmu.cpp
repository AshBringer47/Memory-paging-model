#include "mmu.h"

#include "processqueue.h"
#include "util/rand.h"
#include "mprocess.h"
#include "ram.h"

#include <algorithm>
#include <cassert>

void PagedMemoryManagementUnit::pageFault_IRQ(virtualPageAddress_tp virtualAddress)
{
    _pageFaultCount++;
    _currentProcess->_pagesFaults[virtualAddress]++;

    if(_freePages.empty()) {
        realPageAddress_tp realAddress = constants::BAD_REAL_ADDRESS;
        tick_tp maxTick = 0;

        static size_t i = 0;
        if(i >= _occupiedPages.size()) {
            i = 0;
        }
        assert(i < _occupiedPages.size());

        for(; i < _occupiedPages.size(); ++i) {
            const realPageAddress_tp &address = _occupiedPages.at(i);

            realPage &rPage = _ram->page(address);
            if(rPage.R) {
                rPage.lastAccesTime = _processContainer->tickCount();
            } else {
                tick_tp ticksDelta = _processContainer->tickCount() - rPage.lastAccesTime;
                if(ticksDelta > constants::TICKS_WORKING_SET) {
                    realAddress = address;
                    break;
                } else if(realAddress == constants::BAD_REAL_ADDRESS || ticksDelta > maxTick) {
                    realAddress = address;
                    maxTick = ticksDelta;
                }
            }
        }

        if(realAddress == constants::BAD_REAL_ADDRESS) {
            realAddress = _occupiedPages.at(Rand::intNumber(0,  _occupiedPages.size() - 1));
        }

        realPage &rPage = _ram->page(realAddress);

        storePage(rPage.vPageAddress, rPage.pid);
        restorePage(realAddress, virtualAddress);

    } else {
        realPageAddress_tp realAddress = *_freePages.begin();
        restorePage(realAddress, virtualAddress);
    }
}

void PagedMemoryManagementUnit::storePage(virtualPageAddress_tp virtualAddress, processId_tp pid)
{
    virtualPage &vPage = getPage(virtualAddress, pid);
    assert(vPage.P == true);

    auto realAddress = vPage.memAddress;
    if(_ram->page(realAddress).M) {
        _storage.setData(pid, virtualAddress, _ram->read(realAddress));
    }

    moveToFree(realAddress);

//    page.memAddress <- trash
    vPage.P = false;
}


void PagedMemoryManagementUnit::storePage(virtualPageAddress_tp virtualAddress)
{
    storePage(virtualAddress, currentProcessId());
}

void PagedMemoryManagementUnit::restorePage(realPageAddress_tp realAddress, virtualPageAddress_tp virtualAddress)
{
    realPage &rPage = _ram->page(realAddress);
    if(_storage.contains(currentProcessId(), virtualAddress)) {
        _ram->write(realAddress, _storage.getData(currentProcessId(), virtualAddress));
        rPage.M = false;
    } else {
        _ram->page(realAddress).M = true;
    }

    rPage.pid = currentProcessId();
    rPage.vPageAddress = virtualAddress;

    virtualPage &vPage = getPage(virtualAddress);

    vPage.memAddress = realAddress;
    vPage.P = true;

    moveToOccupied(realAddress);
}

void PagedMemoryManagementUnit::moveToFree(realPageAddress_tp address)
{
    auto it = std::find(_occupiedPages.begin(), _occupiedPages.end(), address);
    assert(it != _occupiedPages.end());
    _freePages.insert(*it);
    _occupiedPages.erase(it);
}

void PagedMemoryManagementUnit::moveToOccupied(realPageAddress_tp address)
{
    auto it = _freePages.find(address);

    assert(it != _freePages.end());
    _occupiedPages.push_back(*it);
    _freePages.erase(it);
}
PagedMemoryManagementUnit::PagedMemoryManagementUnit(ram_ptr ram) :
    _ram(ram)
{
    if(!ram) {
        throw std::invalid_argument("Bad RAM pointer");
    }
    _freePages = ram->pagesAddress();
    _occupiedPages.reserve(_freePages.size());
}

PagedMemoryManagementUnit::~PagedMemoryManagementUnit()
{
}

void PagedMemoryManagementUnit::bindProcess(process_ptr process)
{
    _currentProcess = process;
}

void PagedMemoryManagementUnit::releaseProcessPages()
{
    auto &table = _currentProcess->virtualPages;

    for(auto it = std::begin(table); it != std::end(table);) {
        virtualPageAddress_tp cur = it->first;
        ++it;
        releasePage(cur);
    }
}

void PagedMemoryManagementUnit::releaseProcess()
{
    _currentProcess.reset();
}

void PagedMemoryManagementUnit::setProcessContainer(std::shared_ptr<ProcessQueue> processContainer)
{
    if(!processContainer) {
        throw std::invalid_argument("Bad processContainer pointer");
    }
    _processContainer = processContainer;
}

virtualPageAddress_tp PagedMemoryManagementUnit::allocatePage()
{
    virtualPagesTable_tp &pages = _currentProcess->virtualPages;
    virtualPageAddress_tp address = _currentProcess->takeNextFreeAddress();

    pages[address] = virtualPage();

    return address;
}

void PagedMemoryManagementUnit::releasePage(virtualPageAddress_tp address)
{
    if(_storage.contains(currentProcessId(), address)) {
        _storage.erase(currentProcessId(), address);
    }

    virtualPage &page = getPage(address);

    if(page.P) {
        realPageAddress_tp realAddr = page.memAddress;

        moveToFree(realAddr);
    }

    _currentProcess->virtualPages.erase(address);
}

memoryData_tp PagedMemoryManagementUnit::read(virtualPageAddress_tp address)
{
    checkVirtualAddress(address);
    return _ram->read(getEnsureRamPageAddress(address));
}

void PagedMemoryManagementUnit::write(virtualPageAddress_tp address, memoryData_tp data)
{
    checkVirtualAddress(address);
    _ram->write(getEnsureRamPageAddress(address), data);
}

size_t PagedMemoryManagementUnit::getPageFaultCount()
{
    return _pageFaultCount;
}

void PagedMemoryManagementUnit::resetBitR()
{
    _ram->resetBitR();
}

void PagedMemoryManagementUnit::checkVirtualAddress(virtualPageAddress_tp address)
{
    if(_currentProcess->virtualPages.find(address) == _currentProcess->virtualPages.end()) {
        throw std::runtime_error("Segmentation fault");
    }
}

void PagedMemoryManagementUnit::ensureLinked(virtualPageAddress_tp address)
{
    if(getPage(address).P != true) {
        pageFault_IRQ(address);
    }
}

virtualPage &PagedMemoryManagementUnit::getPage(virtualPageAddress_tp address, processId_tp pid)
{
    process_ptr process = (pid == 0 ? _currentProcess : _processContainer->getProcess(pid));
    return process->virtualPages[address];
}

realPageAddress_tp PagedMemoryManagementUnit::getEnsureRamPageAddress(virtualPageAddress_tp address)
{
    ensureLinked(address);
    auto page = getPage(address);
    assert(page.P == true);
    return page.memAddress;
}

processId_tp PagedMemoryManagementUnit::currentProcessId() const
{
    return _currentProcess->pid();
}

bool PageStorage::contains(processId_tp process, virtualPageAddress_tp address)
{
    return _storageDevice.count(fullPageAddress(process, address)) >= 1;
}

memoryData_tp PageStorage::getData(processId_tp process, virtualPageAddress_tp address) const
{
    return _storageDevice.at(fullPageAddress(process, address));
}

void PageStorage::setData(processId_tp process, virtualPageAddress_tp address, const memoryData_tp &data)
{
    _storageDevice[fullPageAddress(process, address)] = data;
}

void PageStorage::erase(processId_tp process, virtualPageAddress_tp address)
{
    _storageDevice.erase(fullPageAddress(process, address));
}

void PageStorage::clear()
{
    _storageDevice.clear();
}
