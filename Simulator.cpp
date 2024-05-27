/*
 * Simulator function implementations for a cache simulator
 * CSF Assignment 3
 * Lawrence Cai and Benjamin Chang
 * lcai18@jh.edu bchang26@jhu.edu
 */

// Libraries and Files
#include "Simulator.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// Statements
using std::bitset;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::hex;
using std::ifstream;
using std::string;
using std::vector;

/*
 * Constructor for the Simulator class. Initializes a cache simulation instance
 * with the given configurations for sets, blocks per set, block size,
 * policies for write misses, write operations, and eviction.
 *
 * Parameters:
 *   sets - Number of sets in the cache
 *   blocks - Number of blocks in each set
 *   size - Size of each block in bytes
 *   miss - Policy for handling write misses (No-write allocate / Write-allocate)
 *   write - Policy for write operations (Write-through / Write-back)
 *   eviction - Policy for eviction (LRU / FIFO)
 */
Simulator::Simulator(int sets, int blocks, int size, WriteMissPolicy miss, WritePolicy write, EvictionPolicy eviction)
    : sets(sets), blocks(blocks), size(size), missPolicy(miss), writePolicy(write), evictionPolicy(eviction), loads(0),
      stores(0), lhits(0), lmisses(0), shits(0), smisses(0), cycles(0)
{
    
    // Initialize Cache
    cache.sets.resize(sets);
    for (int i = 0; i < sets; ++i)
    {
        cache.sets[i].slots.resize(blocks);
    }

    
    memoryMultiplier = size / 4;
}

/*
 * Destructor for the Simulator class.
 */
Simulator::~Simulator()
{
}

/*
 * Simulates the cache based on the read/write operations from the input.
 * Each line represents an operation with a type 'l' or 's' followed
 * by a hex address. Processes each operation according to the cache's configuration and
 * and updating the cache and statistics accordingly.
 *
 * Returns:
 *   0 on successful simulation, 1 on encountering invalid input operations.
 */
int Simulator::simulate() {
    // Read from file from stdin
    string line;
    while (getline(cin, line)) {
        /* Reading Line */
        // Variables for reading line
        std::istringstream iss(line);
        string type;
        string address;
        // Parse line
        iss >> type >> address;
        if (type == "l") {
            // writes to loads/stores to/from memory take 100 cycles
            load(address);
            loads++;
        }
        else if (type == "s") {
            store(address);
            stores++;
        }
        else {
            cerr << "Invalid type" << endl;
            return 1;
        }
    }
    return 0;
}

/*
 * Converts a 32-bit unsigned integer address to a binary representation.
 *
 * Parameters:
 *   address - The 32-bit address to be converted
 *
 * Returns:
 *   The binary representation of the address as an unsigned long.
 */
int Simulator::toBinary(uint32_t address) {
    bitset<32> addressbits(address);
    return addressbits.to_ulong();
}

/*
 * Updates a cache block with the tag, validity, dirtiness, timestamps, and location.
 *
 * Parameters:
 *   tag - The tag to be updated
 *   valid - Indicating if the block is valid
 *   dirty - Indicating if the block is dirty
 *   load_ts - Timestamp of when the block was loaded
 *   access_ts - Timestamp of the most recent access
 *   index - Index of the set
 *   block - Block number within the set
 */
void Simulator::updateCache(uint32_t tag, bool valid, bool dirty, uint32_t load_ts, uint32_t access_ts, int index,
                            int block) {
    cache.sets[index].slots[block].tag = tag;
    cache.sets[index].slots[block].dirty = dirty;
    cache.sets[index].slots[block].valid = valid;
    cache.sets[index].slots[block].access_ts = access_ts;
    cache.sets[index].slots[block].load_ts = load_ts;
}


/*
 * Selects a block to evict from a cache set based on the FIFO eviction policy.
 * Identifies the block that was loaded earliest among all blocks in the specified set.
 *
 * Parameters:
 *   index - Index of the cache set from which a block is to be evicted
 *
 * Returns:
 *   The index of the block within the set to be evicted.
 */
int Simulator::fifo(int index) {
    uint32_t minLoadTs = cache.sets[index].slots[0].load_ts;
    int evictBlock = 0;
    // loop through to find the earliest load
    for (int i = 1; i < blocks; ++i) { 
        uint32_t curLoadTs = cache.sets[index].slots[i].load_ts;
        if (curLoadTs < minLoadTs) {
            minLoadTs = curLoadTs;
            evictBlock = i;
        }
    }
    if (cache.sets[index].slots[evictBlock].dirty == true) {
        // if dirty write to memory
        cycles += 100 * memoryMultiplier;
    }
    return evictBlock;
}


/*
 * Selects a block to evict based on the LRU eviction policy.
 * Identifies the block that was accessed earliest among all blocks in the specified set.
 *
 * Parameters:
 *   index - Index of the cache set from which a block is to be evicted
 *
 * Returns:
 *   The index of the block within the set to be evicted.
 */
int Simulator::lru(int index)
{
    uint32_t minAccessTs = cache.sets[index].slots[0].access_ts;
    int evictBlock = 0;
    // loop through to find the earliest access
    for (int i = 1; i < blocks; ++i) { 
        uint32_t curAccessTs = cache.sets[index].slots[i].access_ts;
        if (curAccessTs < minAccessTs) {
            minAccessTs = curAccessTs;
            evictBlock = i;
        }
    }
    // if dirty write to memory
    if (cache.sets[index].slots[evictBlock].dirty == true) {
        
        cycles += 100 * memoryMultiplier;
    }
    return evictBlock;
}

/*
 * Evicts a block from a specified cache set.
 * Calls either fifo() or lru() based on the Simulator's evictionPolicy.
 *
 * Parameters:
 *   index - Index of the cache set from which a block is to be evicted
 *
 * Returns:
 *   The index of the block within the set that has been selected for eviction.
 */
int Simulator::evict(int index) {
    if (static_cast<int>(evictionPolicy) == 1) { // fifo
        return fifo(index);
    }
    else
    {
        return lru(index);
    }
}


/*
 * Loads a data block into the cache based on a given address string.
 * The function calculates the cache index, tag from the address, and checks for cache hits.
 * Handles cache misses by either loading data into an empty block or evicting an existing block.
 * Updates the relevant cache statistics accordingly.
 *
 * Parameters:
 *   sAddress - The memory address in hexadecimal string format to load from
 */
void Simulator::load(string sAddress) {
    // convert from string to binary
    unsigned int address = toBinary(stoul(sAddress, nullptr, 16));
    // Calculate bit lengths
    unsigned int offsetBits = log2(size);
    unsigned int indexBits = log2(sets);
    // Extract index
    unsigned int indexMask = (1 << indexBits) - 1;
    unsigned int index = (address >> offsetBits) & indexMask;
    // Extract tag
    unsigned int tag = address >> (offsetBits + indexBits);
    int emptyBlock = -1;
    // check if in cache, block will describe if it's a miss or the index to
    // write to, or if all blocks are occupied
    int block = checkMem(tag, index, emptyBlock);
    if (block < 0 || block == blocks) {                                     // miss
        cycles += 100 * memoryMultiplier; // miss needs to load from memory into cache
        lmisses++;
        if (block == blocks) { // cache is full we neec to evict
            emptyBlock = evict(index);
            updateCache(tag, true, false, loads + stores, loads + stores, index, emptyBlock); // updating the cache
        }
        else { // cache is not full
            updateCache(tag, true, false, loads + stores, loads + stores, index, emptyBlock); // updating the cache
        }
    }
    else {                // hit
        cycles += 1; // load from cache
        lhits++;
        cache.sets[index].slots[block].access_ts = loads + stores; // update time for lru
    }
}

/*
 * Stores a data block into the cache based on a given address string.
 * The function calculates the cache index, tag from the address, and checks for cache hits.
 * Handles cache misses according to the write miss policy by either ignoring the cache or updating it.
 * Updates the relevant cache statistics and state based on the write policy.
 *
 * Parameters:
 *   sAddress - The memory address in hexadecimal string format to store to
 */
void Simulator::store(string sAddress) {
    unsigned int address = stoul(sAddress, nullptr, 16);
    // Calculate bit lengths
    unsigned int offsetBits = log2(size);
    unsigned int indexBits = log2(sets);

    // Extract index
    unsigned int indexMask = (1 << indexBits) - 1;
    unsigned int index = (address >> offsetBits) & indexMask;

    // Extract tag
    unsigned int tag = address >> (offsetBits + indexBits);

    // check if in cache
    int emptyBlock = -1;
    int block = checkMem(tag, index, emptyBlock);
    // miss
    if (block < 0 || block == blocks) {
        writeMiss(tag, index, block, emptyBlock); // if miss
        smisses += 1;
    }
    else { // hit
        cycles += 1; // store to cache
        shits += 1;  // if hit
        writeHit(index, block);
    }
}

/*
 * Updates the cache in when a store hits an existing cache block.
 * Depending on the write policy, it marks the block as dirty or updates the memory immediately.
 *
 * Parameters:
 *   index - Index of the cache set containing the block to be updated
 *   inputBlock - Block number within the set to be updated
 */
void Simulator::writeHit(int index, int inputBlock) {
    cache.sets[index].slots[inputBlock].access_ts = loads + stores; // update time for lru
    //Write Back
    if (static_cast<int>(writePolicy) == 1) { 
        cache.sets[index].slots[inputBlock].dirty = true; // block is dirty
    }
    else  { // write through
        cycles += 100; // write through also stores to memory
    }
}

/*
 * Handles a store operation that misses the cache based on the write miss and
 * write policies. It may write to memory directly or load the relevant block into the cache
 * before performing the store. 
 * Updates cache and cache statistics accordingly.
 * Parameters:
 *   tag - The tag from the address
 *   index - The cache set index from the address
 *   inputBlock - Indicates whether the cache set is full 
 *   emptyInd - Index of an empty block within the set if the set is not full
 */
void Simulator::writeMiss(uint32_t tag, int index, int inputBlock, int emptyInd) {
    // nowrite allocate
    if (static_cast<int>(missPolicy) == 1) {
        cycles += 100; // write to memory
    }
    // write allocate
    else {
        // bring relevant memory block into cache before store
        cycles += 100 * memoryMultiplier; 
        //if cache was full our "empty" index is now where we are evicting                                     
        if (inputBlock == blocks) {
            emptyInd = evict(index);
        }
        // WriteBack
        if (static_cast<int>(writePolicy) == 1) {
            // updating cache in the empty or evicted index depending on whether all blocks were used
            updateCache(tag, true, true, loads + stores, loads + stores, index,
                        emptyInd); // write_back so mark as dirty
            cycles += 1;           // write to cache
        }
        else { // Write Through
            cycles += 100; // write to memory
            cycles += 1;   // also write to cache
            // updating cache in the empty or evicted index depending on whether
            // all blocks were used
            updateCache(tag, true, false, loads + stores, loads + stores, index, emptyInd);
        }
    }
}

/*
 * Checks if a given tag is present in the specified cache set.
 * It also identifies an empty block in the set if one exists.
 *
 * Parameters:
 *   tag - The tag to search for in the cache set
 *   index - The index of the cache set to search within
 *   emptyBlock - A reference parameter to be updated to the index of an empty block
 *
 * Returns:
 *   The index of the block containing the tag if found; otherwise,
 *   -1 if an empty block is found, or the number of allocated blocks indicating a miss and there are no empty blocks.
 */
int Simulator::checkMem(uint32_t tag, int index, int &emptyBlock) {
    if (index >= (int)cache.sets.size()) {
        return -1;
    }
    Set curSet = cache.sets[index];
    int numAllocated = 0;
    // go through all blocks and see if tags match
    for (int i = 0; i < blocks; ++i) {
        // block found from index
        Slot block = curSet.slots[i];
        // check valid
        if (!block.valid) {
            emptyBlock = i; // storing to reference to indicate an empty place in cache to use
            continue;
        }
        // if tags match, then we have a "hit"
        if (block.tag == tag) {
            emptyBlock = i;
            return i;
        }
        numAllocated += 1;
    }
    if (numAllocated < blocks) { //miss but there are empty blocks
        return -1;
    }
    // if we iterate through all lines within all sets, then we have a "miss", and all blocks are full so we return to indicate all blocks are full
    return numAllocated;
}

/*
 * Prints the simulation's final statistics to standard output, including total loads,
 * stores, hits, misses, and the total number of cycles taken.
 */
void Simulator::print() {
    cout << "Total loads: " << loads << endl;
    cout << "Total stores: " << stores << endl;
    cout << "Load hits: " << lhits << endl;
    cout << "Load misses: " << lmisses << endl;
    cout << "Store hits: " << shits << endl;
    cout << "Store misses: " << smisses << endl;
    cout << "Total cycles: " << cycles << endl;
}


