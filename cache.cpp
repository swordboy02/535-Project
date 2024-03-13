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

        int readRAM(int address) {
            if (address < 0 || address >= memory.size()) {
                std::cerr << "Error: Invalid memory address\n";
                return -1;
            }

            std::cout << "RAM Read " << memory[address] << " from address: " << address << std::endl;
            return memory[address];
        }

        void writeRAM(int address, int value) {
            if (address < 0 || address >= memory.size()) {
                std::cerr << "Error: Invalid memory address\n";
                return;
            }

            memory[address] = value;
            std::cout << "RAM Write " << memory[address] << " from address: " << address << std::endl;
        }

        void displayRAM() {
        std::cout << "\n";
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
    int readCache(int address, RAM& ram){
        int lineIndex = address % numLines;
        int val;

        if (cache[lineIndex].valid) {
            incrementCycleCount();
            val = cache[lineIndex].data[address % lineSize];
        } else {
            std::cout << "Cache miss! Fetching data from memory...\n";
            incrementCycleCount();
            int dataFromRam = ram.readRAM(address);
            cache[lineIndex].valid = true;
            cache[lineIndex].data[address % lineSize] = dataFromRam;
            val = dataFromRam;
        }

        std::cout << "Cache Read " << val << " from address: " << address << std::endl;
        return val;
    }

    //write function
    void writeCache(int address, int value, RAM& ram){
        int lineIndex = address % numLines;
        cache[lineIndex].valid = true;
        cache[lineIndex].data[address % lineSize] = value;
        cache[lineIndex].dirty = true;

        ram.writeRAM(address, value);
        incrementCycleCount();
    }

    //display cache function
    void displayCache() {
        std::cout << "\n";
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
        std::cout << "\n";
        std::cout << "Cache Contents (Delay):\n";
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
            } else {
                std::cout << "No Data\n";
            }
            std::cout << "--\n";
        }
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

void driver(std::ifstream& commandsFile) {
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
            myCache.writeCache(address, value, ram);
        } else if (command == "R") {
            int address;
            iss >> address;
            myCache.readCache(address, ram);
        } else if (command == "DISPLAYCACHE") {
            myCache.displayCache();
        } else if (command == "DISPLAYCACHEDELAY") {
            int delayMilliseconds;
            iss >> delayMilliseconds;
            myCache.displayCacheWithDelay(delayMilliseconds);
        } else if (command == "DISPLAYRAM") {
            ram.displayRAM();
        } else if (command == "WAIT") {
            std::cout << "Press Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            std::cerr << "Invalid command: " << command << std::endl;
        }
    }
    std::cout << "\n";
    std::cout << "Cycle count: " << myCache.getCycleCount() << std::endl;

}

int main() {
    std::ifstream commandsFile("commands.txt");
    if (!commandsFile.is_open()) {
        std::cerr << "Failed to open commands file\n";
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    driver(commandsFile);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    commandsFile.close();

    return 0;
}