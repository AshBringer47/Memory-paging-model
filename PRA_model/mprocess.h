#ifndef PROCESS_H
#define PROCESS_H

#include "my_global.h"

#include <set>
#include <unordered_set>
#include <unordered_map>

class exitInterrupt { };

class ProcessProtectedData {
    friend class PagedMemoryManagementUnit;
#ifdef VISUALIZE
    friend class GraphicProcess;
#endif
public:
    ProcessProtectedData();

    processId_tp pid() const;

private:
    virtualPagesTable_tp virtualPages;
    size_t pageFaultCount = 0;
    size_t pageFaultDeltaCount = 0;
    std::map<virtualPageAddress_tp, unsigned long long> _pagesFaults;
    virtualPageAddress_tp takeNextFreeAddress();

    processId_tp _PID;
    static processId_tp _pIdCounter;
    static virtualPageAddress_tp _pageAddressCounter;
};

class Program : public ProcessProtectedData
{
#ifdef VISUALIZE
    friend class GraphicProcess;
#endif
public:
    Program();
    ~Program();

    void tick(mmu_ptr MMU);

protected:
    enum State {ALLOCATE, CHANGE_WORKING_SET, WORK, DEALLOCATE, EXIT}
               _processState;
    void setProcessState(State state);

    virtualPageAddress_tp getRandomPage(bool fromWorkingSet = false);

    std::unordered_set<virtualPageAddress_tp> _allPages;
    std::unordered_set<virtualPageAddress_tp> _workingSet;

    int _allPageCounter;
    int _workingSetPageCounter;
};

#endif // PROCESS_H
