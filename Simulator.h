/*
 * Simulator functions for a cache simulator
 * CSF Assignment 3
 * Lawrence Cai and Benjamin Chang
 * lcai18@jh.edu bchang26@jhu.edu
 */

// Guards
#ifndef SIMULATOR_H
#define SIMULATOR_H

// Libraries and Files
#include <stdint.h>

#include <bitset>
#include <string>
#include <vector>
// Policies
enum class WriteMissPolicy { WriteAllocate, NoWriteAllocate };
enum class EvictionPolicy { LRU, FIFO };
enum class WritePolicy { WriteThrough, WriteBack };

// Cache Data Structure
struct Slot {
  /* Variables */
  bool valid;
  bool dirty;
  uint32_t tag;
  int policy;
  uint32_t load_ts, access_ts;
};

struct Set {
  std::vector<Slot> slots;
};
struct Cache {
  std::vector<Set> sets;
};

// Class Definition
class Simulator {
 public:
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
  Simulator(int sets, int blocks, int size, WriteMissPolicy write,
            WritePolicy miss, EvictionPolicy evictio);

  /*
 * Destructor for the Simulator class.
 */
  ~Simulator();

/*
 * Simulates the cache based on the read/write operations from the input.
 * Each line represents an operation with a type 'l' or 's' followed
 * by a hex address. Processes each operation according to the cache's configuration and
 * and updating the cache and statistics accordingly.
 *
 * Returns:
 *   0 on successful simulation, 1 on encountering invalid input operations.
 */
  int simulate();

/*
 * Prints the simulation's final statistics to standard output, including total loads,
 * stores, hits, misses, and the total number of cycles taken.
 */
  void print();

 private:
  /* Variables */
  // Input Information
  int sets;
  int blocks;
  int size;
  WriteMissPolicy missPolicy;
  WritePolicy writePolicy;
  EvictionPolicy evictionPolicy;
  std::string traceFile;
  // Statistics

  uint32_t loads;
  uint32_t stores;
  int lhits;
  int lmisses;
  int shits;
  int smisses;
  int cycles;
  int memoryMultiplier;
  // Cache
  Cache cache;
  // Memory
  /* Methods */
 
/*
 * Loads a data block into the cache based on a given address string.
 * The function calculates the cache index, tag from the address, and checks for cache hits.
 * Handles cache misses by either loading data into an empty block or evicting an existing block.
 * Updates the relevant cache statistics accordingly.
 *
 * Parameters:
 *   sAddress - The memory address in hexadecimal string format to load from
 */
  void load(std::string address);

/*
 * Stores a data block into the cache based on a given address string.
 * The function calculates the cache index, tag from the address, and checks for cache hits.
 * Handles cache misses according to the write miss policy by either ignoring the cache or updating it.
 * Updates the relevant cache statistics and state based on the write policy.
 *
 * Parameters:
 *   sAddress - The memory address in hexadecimal string format to store to
 */
  void store(std::string address);

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
  int checkMem(uint32_t tag, int index, int &evictInd);

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
  void writeMiss(uint32_t tag, int index, int inputBlock, int empytInd);
/*
 * Updates the cache in when a store hits an existing cache block.
 * Depending on the write policy, it marks the block as dirty or updates the memory immediately.
 *
 * Parameters:
 *   index - Index of the cache set containing the block to be updated
 *   inputBlock - Block number within the set to be updated
 */
  void writeHit(int index, int inputBlock);

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
  void updateCache(uint32_t tag, bool valid, bool dirty, uint32_t load_ts,
                   uint32_t access_ts, int index, int block);
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
  int fifo(int index);
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
  int lru(int index);
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
  int evict(int index);

/*
 * Converts a 32-bit unsigned integer address to a binary representation.
 *
 * Parameters:
 *   address - The 32-bit address to be converted
 *
 * Returns:
 *   The binary representation of the address as an unsigned long.
 */
  int toBinary(uint32_t address);
};

#endif