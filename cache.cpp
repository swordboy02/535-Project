#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <string>

class RAM {
private:
    std::vector<int> memory;

public:
    RAM(int size) : memory(size, 0) {}

    int read(int address) {
        if (address < 0 || address >= memory.size()) {
            std::cerr << "Error: Invalid memory address\n";
            return -1;
        }
        return memory[address];
    }

    void write(int address, int value) {
        if (address < 0 || address >= memory.size()) {
            std::cerr << "Error: Invalid memory address\n";
            return;
        }
        memory[address] = value;
    }

    void displayRAM() {
    std::cout << "RAM Contents:\n";
    for (int i = 0; i < memory.size(); i += 4) {
        std::cout << "Address " << i << ": ";
        for (int j = 0; j < 4 && i + j < memory.size(); ++j) {
            std::cout << memory[i + j] << " ";
        }
        std::cout << std::endl;
    }
}
};

struct CacheLine {
    bool valid;
    bool dirty;
    std::vector<int> data;
};


class Cache{
    private:
            int numLines;
            int lineSize;
            std::vector<CacheLine> cache;
            int cycleCount;
        
    public:
    //constructor
    Cache(int numLines, int lineSize) : numLines(numLines), lineSize(lineSize), cycleCount(0) {
        cache.resize(numLines);
        for(int i = 0; i < numLines; i++){
            cache[i].valid = false;
            cache[i].dirty = false;
            cache[i].data.resize(lineSize);
        }

    }

    //read function
    int read(int address, RAM& ram){
        int lineIndex = address % numLines;
        if(cache[lineIndex].valid){
            incrementCycleCount();
            return cache[lineIndex].data[address % lineSize];
        }
        else{
            std::cout << "Cache miss! Fetching data from memory...\n";
            incrementCycleCount();
            int dataFromRam = ram.read(address);
            cache[lineIndex].valid = true;
            cache[lineIndex].data[address % lineSize] = dataFromRam;
            return dataFromRam;
        }
    }

    //write function
    void write(int address, int value, RAM& ram){
        int lineIndex = address % numLines;
        cache[lineIndex].valid = true;
        cache[lineIndex].data[address % lineSize] = value;
        cache[lineIndex].dirty = true;
        ram.write(address, value);
        incrementCycleCount();

    }

    //display cache function
    void displayCache() {
        std::cout << "Cache Contents:\n";
        for (int i = 0; i < numLines; ++i) {
            std::cout << "Line " << i << ": Valid=" << cache[i].valid << ", Dirty=" << cache[i].dirty << std::endl;
            if (cache[i].valid) {
                std::cout << "Data:";
                for (int j = 0; j < lineSize; ++j) {
                    std::cout << " " << cache[i].data[j];
                }
                std::cout << std::endl;
            }
        }
    }
    void displayCacheWithDelay(int delayMilliseconds) {
    std::cout << "Cache Contents:\n";
    for (int i = 0; i < numLines; ++i) {
        std::cout << "Line " << i << ": V=" << cache[i].valid << ", D=" << cache[i].dirty << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
        if (cache[i].valid) {
            std::cout << "Data:";
            for (int j = 0; j < lineSize; ++j) {
                std::cout << " " << cache[i].data[j];
            }
            std::cout << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMilliseconds));
        }
    }
    std::cout << "\n";
}
    void incrementCycleCount() {
        cycleCount++;
    }

    void resetCycleCount() {
        cycleCount = 0;
    }

    int getCycleCount() {
        return cycleCount;
    }
};

int main() {

    std::ifstream commandsFile("commands.txt");
    if (!commandsFile.is_open()) {
        std::cerr << "Failed to open commands file\n";
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

    RAM ram(32);

    Cache myCache(16, 4);

    std::string line;
    while (std::getline(commandsFile, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        if (command == "W") {
            int address, value;
            iss >> address >> value;
            myCache.write(address, value, ram);
        } else if (command == "R") {
            int address;
            iss >> address;
            myCache.read(address, ram);
        } else if (command == "DISPLAYCACHE") {
            int delayMilliseconds;
            iss >> delayMilliseconds;
            myCache.displayCacheWithDelay(delayMilliseconds);
        } else if (command == "DISPLAYRAM") {
            ram.displayRAM();
        }
    }
    std::cout << "Cycle count: " << myCache.getCycleCount() << std::endl;

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    commandsFile.close();

    return 0;
}