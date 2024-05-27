#include <stdio.h>
#include <iostream>
#include "Simulator.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

bool validateArguments(int argc, char *argv[]);
bool parseInputParameters(int &sets, int &blocks, int &size, string &miss, string &write, string &eviction, char *argv[]);
bool convertMissPolicy(const string &miss, WriteMissPolicy &missPolicy);
bool convertWritePolicy(const string &write, const string &miss, WritePolicy &writePolicy);
bool convertEvictionPolicy(const string &eviction, EvictionPolicy &evictionPolicy);

/*
 * The main entry point for the cache simulator.
 *
 * Parameters:
 *   argc - The number of command line arguments
 *   argv - The array of command line arguments
 *
 * Returns:
 *   0 if the simulation runs successfully, 1 otherwise.
 */
int main(int argc, char *argv[]) {
    if (!validateArguments(argc, argv)) {
        return 1;
    }
    int sets, blocks, size;
    string miss, write, eviction;
    if (!parseInputParameters(sets, blocks, size, miss, write, eviction, argv)) {
        return 1;
    }
    WriteMissPolicy missPolicy;
    if (!convertMissPolicy(miss, missPolicy)) {
        cerr << "Invalid miss policy." << endl;
        return 1;
    }
    WritePolicy writePolicy;
    if (!convertWritePolicy(write, miss, writePolicy)) {
        cerr << "Invalid write policy." << endl;
        return 1;
    }
    EvictionPolicy evictionPolicy;
    if (!convertEvictionPolicy(eviction, evictionPolicy)) {
        cerr << "Invalid eviction policy." << endl;
        return 1;
    }
    Simulator sim(sets, blocks, size, missPolicy, writePolicy, evictionPolicy);
    if (sim.simulate() == 1) {
        return 1;
    }
    sim.print();
    return 0;
}


/*
 * Converts a string representation of the miss policy to its corresponding enum value.
 *
 * Parameters:
 *   miss - The string representation of the miss policy
 *   missPolicy - Reference to the WriteMissPolicy enum to store the converted value
 *
 * Returns:
 *   True if the conversion is successful, False otherwise.
 */
bool convertMissPolicy(const string &miss, WriteMissPolicy &missPolicy) {
    if (miss == "write-allocate") {
        missPolicy = WriteMissPolicy::WriteAllocate;
        return true;
    } else if (miss == "no-write-allocate") {
        missPolicy = WriteMissPolicy::NoWriteAllocate;
        return true;
    }
    return false; // Indicate failure to convert
}

/*
 * Converts a string representation of the write policy to its corresponding enum value.
 *
 * Parameters:
 *   write - The string representation of the write policy
 *   miss - The string representation of the miss policy, used for validation
 *   writePolicy - Reference to the WritePolicy enum to store the converted value
 *
 * Returns:
 *   True if the conversion is successful, False otherwise.
 */
bool convertWritePolicy(const string &write, const string &miss, WritePolicy &writePolicy) {
    if (write == "write-back") {
        if (miss == "no-write-allocate") {
            cerr << "write-back and no-write-allocate cannot be specified together" << endl;
            return false;
        }
        writePolicy = WritePolicy::WriteBack;
        return true;
    } else if (write == "write-through") {
        writePolicy = WritePolicy::WriteThrough;
        return true;
    }
    return false; // Indicate failure to convert
}

/*
 * Converts a string representation of the eviction policy to its corresponding enum value.
 *
 * Parameters:
 *   eviction - The string representation of the eviction policy
 *   evictionPolicy - Reference to the EvictionPolicy enum to store the converted value
 *
 * Returns:
 *   True if the conversion is successful, False otherwise.
 */
bool convertEvictionPolicy(const string &eviction, EvictionPolicy &evictionPolicy) {
    if (eviction == "fifo") {
        evictionPolicy = EvictionPolicy::FIFO;
        return true;
    } else if (eviction == "lru") {
        evictionPolicy = EvictionPolicy::LRU;
        return true;
    }
    return false; // Indicate failure to convert
}
/*
* Checks if a number is a power of 2.
* 
* Parameters:
*   x - The number to check
*
* Returns:
*  True if the number is a power of 2, False otherwise.
*/
bool isPowerOfTwo(int x) {
    return x && !(x & (x - 1));
}

/*
 * Parses input parameters from command line arguments for the cache configuration.
 *
 * Parameters:
 *   sets - Reference to the number of sets in the cache
 *   blocks - Reference to the number of blocks per set
 *   size - Reference to the size of each block
 *   miss - Reference to the string indicating the miss policy
 *   write - Reference to the string indicating the write policy
 *   eviction - Reference to the string indicating the eviction policy
 *   argv - The array of command line arguments
 *
 * Returns:
 *   True if the parameters are successfully parsed, False otherwise.
 */
bool parseInputParameters(int &sets, int &blocks, int &size, string &miss, string &write, string &eviction, char *argv[]) {
    sets = std::atoi(argv[1]);
    blocks = std::atoi(argv[2]);
    size = std::atoi(argv[3]);
    miss = argv[4];
    write = argv[5];
    eviction = argv[6];

    // Validate block size
    if (size < 4) {
        cerr << "ERROR: Block size (" << size << ") is less than the minimum allowed size of 4 bytes." << endl;
        return false;
    }
    if (!isPowerOfTwo(size)) {
        cerr << "ERROR: Block size (" << size << ") is not a power of 2." << endl;
        return false;
    }

    // Validate number of sets
    if (!isPowerOfTwo(sets) || sets <= 0) {
        cerr << "ERROR: Number of sets (" << sets << ") is not a positive power of 2." << endl;
        return false;
    }

    // Validate number of blocks per set
    if (blocks <= 0) {
        cerr << "ERROR: Number of blocks per set (" << blocks << ") cannot be less than 1." << endl;
        return false;
    }

    return true;
}

/*
 * Validates the command line arguments for the simulator.
 *
 * Parameters:
 *   argc - The number of arguments passed to the program
 *   argv - The array of arguments passed to the program
 *
 * Returns:
 *   True if the arguments are valid, False otherwise.
 */
bool validateArguments(int argc, char *argv[]) {
    if (argc < 7) {
        cout << "Arguments provided: " << argc - 1 << endl;
        cerr << "Usage: " << argv[0] << " <# of sets> <# of blocks> <block size> <write policy> <miss policy> <eviction policy>" << endl;
        return false;
    }
    return true;
}