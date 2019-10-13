#include <iostream>

#include "ram.h"
#include "mmu.h"
#include "mprocess.h"
#include "processqueue.h"

#include <memory>
#include <thread>

using namespace std

int main()
{
    ram_ptr ram = ram_ptr(new RandomAccesMemory(constants::RAM_PAGE_COUNT));

    mmu_ptr mmu = mmu_ptr(new PagedMemoryManagementUnit(ram));

   shared_ptr<ProcessQueue> queue = shared_ptr<ProcessQueue>(new ProcessQueue(mmu));
    mmu->setProcessContainer(queue);

    for(int i = 0; i < 30; i++) {
        queue->addProcess(process_ptr(new Program()));
    }

    cin.get();
    cout << "Run\n";

    queue->run();

    while(true) {
//        cout << mmu->getPageFaultCount() << "\n";
    }

    cin.get();

    return 0;
}

