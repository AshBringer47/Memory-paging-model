#include <iostream>

#include "ram.h"
#include "mmu.h"
#include "mprocess.h"
#include "processqueue.h"

#include <memory>
#include <thread>

int main()
{
    ram_ptr ram = ram_ptr(new RandomAccesMemory(constants::RAM_PAGE_COUNT));

    mmu_ptr mmu = mmu_ptr(new PagedMemoryManagementUnit(ram));

    std::shared_ptr<ProcessQueue> queue = std::shared_ptr<ProcessQueue>(new ProcessQueue(mmu));
    mmu->setProcessContainer(queue);

    for(int i = 0; i < 30; i++) {
        queue->addProcess(process_ptr(new Program()));
    }

    std::cin.get();
    std::cout << "Run\n";

    queue->run();

    while(true) {
//        std::cout << mmu->getPageFaultCount() << "\n";
    }

    std::cin.get();

    return 0;
}

