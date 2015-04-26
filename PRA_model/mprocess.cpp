#include "mprocess.h"
#include "mmu.h"

#include "util/rand.h"

#include <limits>
#include <cassert>
#include <iostream>

processId_tp ProcessProtectedData::_pIdCounter = 0;
virtualPageAddress_tp ProcessProtectedData::_pageAddressCounter = 0;

ProcessProtectedData::ProcessProtectedData()
{
    assert(_pIdCounter != std::numeric_limits<processId_tp>::max());
    _PID = ++_pIdCounter;
}

virtualPageAddress_tp ProcessProtectedData::takeNextFreeAddress()
{
    if(_pageAddressCounter != std::numeric_limits<virtualPageAddress_tp>::max()) {
        return ++_pageAddressCounter;
    } else {
        assert(false);  // TODO:
    }
    assert(false);  // TODO:
    return 0;
}


Program::Program()
{
    _allPageCounter = Rand::intNumber(constants::PROCESS_PAGE_COUNT);
    _workingSetPageCounter = Rand::intNumber(constants::WORKING_SET_PAGE_COUNT);
    setProcessState(State::ALLOCATE);
}

Program::~Program()
{

}

void Program::tick(mmu_ptr MMU)
{
    if(MMU) {
        switch(_processState) {
        case State::ALLOCATE:
            if(_allPageCounter-- > 0) {
                _allPages.insert(MMU->allocatePage());
            } else {
                setProcessState(State::CHANGE_WORKING_SET);
            }
            break;
        case State::CHANGE_WORKING_SET:
            if(_workingSet.empty()) {
                _workingSetPageCounter = Rand::intNumber(constants::WORKING_SET_PAGE_COUNT);
            }
            if(_workingSetPageCounter-- > 0) {
                _workingSet.insert(getRandomPage());
            } else {
                setProcessState(State::WORK);
            }
            break;
        case State::WORK:
        {
            bool fromWorkingSet = Rand::trueWithChance(constants::CHANCE_ACTION_WITH_WORKING_SET);
            virtualPageAddress_tp address = getRandomPage(fromWorkingSet);

            if(Rand::boolean()) {
                MMU->read(address);
            } else {
                MMU->write(address, memoryData_tp(Rand::intNumber(constants::RAM_VALUES_RANGE)));
            }

            if(Rand::trueWithChance(constants::CHANCE_CHANGE_WORKING_SET)) {
                _workingSet.clear();
                setProcessState(State::CHANGE_WORKING_SET);
            }
            if(Rand::trueWithChance(constants::CHANCE_DEALLOCATE)) {
                setProcessState(State::DEALLOCATE);
            }
        }
            break;
        case State::DEALLOCATE:
            if(!_allPages.empty()) {
                virtualPageAddress_tp dPage = getRandomPage();
                MMU->releasePage(dPage);
                _allPages.erase(dPage);
                _workingSet.erase(dPage);
            } else {
                setProcessState(State::EXIT);
            }

            if(Rand::trueWithChance(constants::CHANCE_EXIT_WITHOUT_DEALLOCATE)) {  // Иногда процессы освобождают не всю свою память перед выходом
                setProcessState(State::EXIT);
            }
            break;
        case State::EXIT:
            std::cout << "Exit from process " << pid() << "\n";
            throw exitInterrupt();
            break;
        }
    }
}

void Program::setProcessState(Program::State state)
{
    _processState = state;
}

virtualPageAddress_tp Program::getRandomPage(bool fromWorkingSet)
{
    auto addressList = fromWorkingSet ? _workingSet : _allPages;
//    std::cout << "PID: " << pid() << " WS: " << _workingSet.size() << " All: " << _allPages.size() << " " << fromWorkingSet << "\n";

    assert(addressList.size() != 0);
    size_t randPage = Rand::intNumber(0, addressList.size() - 1) + 1;

    auto pageIt = addressList.cbegin();
    while(--randPage > 0) {
        ++pageIt;
    }

    return *pageIt;
}

processId_tp ProcessProtectedData::pid() const
{
    return _PID;
}
