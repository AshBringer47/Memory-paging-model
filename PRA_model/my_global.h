#ifndef GLOBAL_TYPES
#define GLOBAL_TYPES

#include "util/range.h"

#include <chrono>
#include <cstddef>
#include <vector>
#include <memory>
#include <cmath>
#include <map>

// Зашилимо реалізацію таблить сторінок і таблить таблиць сторінок на інший раз
//struct virtualAddress {
//    unsigned int address : 32;
//    union {
//        unsigned int directoryNumber : 10;
//        unsigned int tableNumber : 10;
//        unsigned int offset : 12;
//    };
//};

#include <stdint.h>

class PagedMemoryManagementUnit;
class RandomAccesMemory;
class Program;

using mmu_ptr = std::shared_ptr<PagedMemoryManagementUnit>;
using ram_ptr = std::shared_ptr<RandomAccesMemory>;
using process_ptr = std::shared_ptr<Program> ;

using address_tp = unsigned int;
using realPageAddress_tp = address_tp;
using virtualPageAddress_tp = address_tp ;
using processId_tp = unsigned int;
using memoryData_tp = int;
using tick_tp = unsigned long;

namespace constants {
constexpr size_t RAM_PAGE_COUNT  = 20;

constexpr virtualPageAddress_tp BAD_VIRTUAL_ADDRESS = -1;
constexpr realPageAddress_tp    BAD_REAL_ADDRESS    = -1;

constexpr double CHANCE_ACTION_WITH_WORKING_SET = 9 / 10.0;
constexpr double CHANCE_CHANGE_WORKING_SET = 0;
constexpr double CHANCE_DEALLOCATE = 0.0 / 100.0;
constexpr double CHANCE_EXIT_WITHOUT_DEALLOCATE = 0.25 / 100;

constexpr range<int> PROCESS_PAGE_COUNT{4, 10};
constexpr range<int> WORKING_SET_PAGE_COUNT{1, 1};
constexpr range<int> RAM_VALUES_RANGE{10, 100};

constexpr tick_tp TICKS_TO_RESET_R_BIT = 50;
constexpr tick_tp TICKS_WORKING_SET = 30;       // Здається, це залежить від кількості процесів, та їх інтенсивності
}

struct virtualPage {
    bool P = false;
    realPageAddress_tp memAddress = constants::BAD_REAL_ADDRESS;
};

struct realPage {
    bool R = false;
    bool M = false;
    processId_tp pid = 0;
    virtualPageAddress_tp vPageAddress = constants::BAD_VIRTUAL_ADDRESS;
    memoryData_tp data = memoryData_tp(0);


    tick_tp lastAccesTime = 0;      // TODO: створити для цього інший список з цими даними?
};

using virtualPagesTable_tp = std::map<virtualPageAddress_tp, virtualPage>;
using RAMpages_tp = std::vector<realPage>;


#endif // GLOBAL_TYPES

