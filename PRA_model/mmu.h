#ifndef MMU_H
#define MMU_H

#include "processqueue.h"
#include "my_global.h"

#include <cstddef>
class ProcessQueue;
class Program;
class Table;

#include <unordered_map>
#include <unordered_set>
class PageStorage {
public:
    struct _Hash;

    struct fullPageAddress {
        fullPageAddress(processId_tp processId, virtualPageAddress_tp address) {
             _PID = processId;
             _address = address;
        }

        bool operator ==(const fullPageAddress &other) const {
            return this->_PID == other._PID
                    && this->_address == other._address;
        }

    private:
        friend class _Hash;
        processId_tp _PID;
        virtualPageAddress_tp _address;
    };

    struct _Hash {
      size_t operator()(const fullPageAddress &p) const {
        return std::hash<processId_tp>()(p._PID) ^ std::hash<virtualPageAddress_tp>()(p._address);
      }
    };

    bool contains(processId_tp process, virtualPageAddress_tp address);
    memoryData_tp getData(processId_tp process, virtualPageAddress_tp address) const;
    void setData(processId_tp process, virtualPageAddress_tp address, const memoryData_tp &data);
    void erase(processId_tp process, virtualPageAddress_tp address);
    void clear();

private:
    std::unordered_map<fullPageAddress, memoryData_tp, _Hash> _storageDevice;
};

class PagedMemoryManagementUnit /*: public RandomAccesMemory*/
{
public:
    PagedMemoryManagementUnit(ram_ptr ram);
    ~PagedMemoryManagementUnit();

    void bindProcess(process_ptr process);
    void releaseProcess();

    void setProcessContainer(std::shared_ptr<ProcessQueue> processContainer);

    virtualPageAddress_tp allocatePage();
    void releasePage(virtualPageAddress_tp address);
    void releaseProcessPages();

    memoryData_tp read(virtualPageAddress_tp address);
    void write(virtualPageAddress_tp address, memoryData_tp data);

    size_t getPageFaultCount();

    void resetBitR();

protected:
    void checkVirtualAddress(virtualPageAddress_tp address);

    void ensureLinked(virtualPageAddress_tp address);
    virtualPage &getPage(virtualPageAddress_tp address, processId_tp pid = 0);
    realPageAddress_tp getEnsureRamPageAddress(virtualPageAddress_tp address);
    processId_tp currentProcessId() const;

    void pageFault_IRQ(virtualPageAddress_tp virtualAddress);

    void storePage(virtualPageAddress_tp virtualAddress);
    void storePage(virtualPageAddress_tp virtualAddress, processId_tp pid);
    void restorePage(realPageAddress_tp realAddress, virtualPageAddress_tp virtualAddress);

    void moveToFree(realPageAddress_tp address);
    void moveToOccupied(realPageAddress_tp address);

    ram_ptr _ram;
    std::shared_ptr<ProcessQueue> _processContainer;
    process_ptr _currentProcess;
    size_t _pageFaultCount = 0;

    std::unordered_set<realPageAddress_tp> _freePages;
    std::vector<realPageAddress_tp> _occupiedPages;

    PageStorage _storage;
};

#endif // MMU_H
