#ifndef RAM_H
#define RAM_H

#include "my_global.h"

#include <cstddef>
#include <vector>
#include <unordered_set>

class PagedMemoryManagementUnit;

class RandomAccesMemory
{
public:
    // Встановлення та очищення бітів R та M апаратне
    RandomAccesMemory(size_t size);
    ~RandomAccesMemory();

    size_t size();

    std::unordered_set<realPageAddress_tp> pagesAddress();

    memoryData_tp read(realPageAddress_tp address) const;
    void write(realPageAddress_tp address, memoryData_tp data);

    realPage &page(realPageAddress_tp address);

    void resetBitR();

protected:
    mutable RAMpages_tp _pages;
};

#endif // RAM_H
