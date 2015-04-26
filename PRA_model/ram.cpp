#include "ram.h"

RandomAccesMemory::RandomAccesMemory(size_t size) :
    _pages(size)
{
}

RandomAccesMemory::~RandomAccesMemory()
{

}

size_t RandomAccesMemory::size()
{
    return _pages.size();
}

std::unordered_set<realPageAddress_tp> RandomAccesMemory::pagesAddress()
{
    std::unordered_set<realPageAddress_tp> pages;
    for(size_t i = 0; i < _pages.size(); i++) {
        pages.insert(i);
    }
    return pages;
}

memoryData_tp RandomAccesMemory::read(realPageAddress_tp address) const
{
    _pages[address].R = true;
    return _pages.at(address).data;
}

void RandomAccesMemory::write(realPageAddress_tp address, memoryData_tp data)
{
    _pages[address].M = true;
    _pages[address].data = data;
}

realPage &RandomAccesMemory::page(realPageAddress_tp address)
{
    return _pages[address];
}

void RandomAccesMemory::resetBitR()
{
    for(auto &elem: _pages) {
        elem.R = false;
    }
}
