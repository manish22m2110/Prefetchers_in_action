#include <deque>
#include <iostream>

#include "cache.h"
#include <unordered_map>

constexpr int STREAM_SIZE = 64;
int PREFETCH_DEGREE = 64;
int PREFETCH_DISTANCE = 64;

// Struct to store information about a stream.
struct StreamEntry {
  uint64_t last_address;  // Last accessed memory address in the stream.
  int direction;          // Direction of the stream (-1 for decreasing addresses, 1 for increasing addresses).
  int consecutive_misses; // Number of consecutive cache misses within the same stream.
};

// constexpr std::size_t STREAM_SETS = 256;
// constexpr std::size_t STREAM_WAYS = 4;

// std::map<CACHE*, std::array<StreamEntry, STREAM_SETS * STREAM_WAYS>> streamTable;

// Hash table to track streams based on instruction pointers (IP).
std::unordered_map<uint64_t, StreamEntry> streamTable;

// Queue to hold prefetch addresses.
std::deque<uint64_t> prefetchQueue;

void CACHE::prefetcher_initialize()
{
  std::cout << "Stream Prefetcher Initialized" << std::endl;
  std::cout << "Stream Prefetcher Degree: " << PREFETCH_DEGREE << std::endl;
  std::cout << "Stream Prefetcher Distance: " << PREFETCH_DISTANCE << std::endl;
}
int calculate_throttled_degree(long double accuracy_rate, int prefetch_degree)
{
  // double accuracy_rate = calculate_accuracy_rate();

  // Define thresholds for adjusting the prefetch degree
  double high_accuracy_threshold = 70.0;
  double low_accuracy_threshold = 50.0;
  int original_degree = prefetch_degree;
  // FILE *fptr = fopen("/home/sunanda/Course-Files-IITB/CS_683/pa2/logs/predeg.txt","a");

  // Adjust the prefetch degree based on accuracy rate
  if (accuracy_rate >= high_accuracy_threshold) {
    return 2 * original_degree;
  } else if (accuracy_rate >= low_accuracy_threshold) {
    // If accuracy is between the high and low thresholds, reduce the degree by half
    // fprintf(fptr, "2 %d ; ", original_degree/2);
    std::max(original_degree / 2, 1);
  } else {

    return 1;
  }
}
// This function is called when a cache access is made.
uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  // Only initiate prefetching if it's a cache miss.
  int upPrefetchDeg = 1; // Default value
  int upPrefDist = 1;
  long double accuracy_rate = pf_issued > 0 ? (pf_useful * 100.0) / pf_issued : 0;

  upPrefetchDeg = calculate_throttled_degree(accuracy_rate, PREFETCH_DEGREE);
  upPrefDist = calculate_throttled_degree(accuracy_rate, PREFETCH_DISTANCE);

  PREFETCH_DEGREE = upPrefetchDeg;
  PREFETCH_DISTANCE = upPrefDist;

  if (!cache_hit) {

    // Cache miss, update the stream entry for this IP.
    auto it = streamTable.find(ip);
    if (it != streamTable.end()) {
      StreamEntry& entry = it->second;
      if (entry.direction == 0) {
        // First time direction is determined.
        int64_t stride = (int64_t)addr - (int64_t)entry.last_address;
        entry.direction = (stride >= 0) ? 1 : -1;
      } else {
        // Check if the current access is in the same stream.
        int64_t stride = (int64_t)addr - (int64_t)entry.last_address;
        if ((stride >= 0 && entry.direction == 1) || (stride < 0 && entry.direction == -1)) {
          entry.consecutive_misses++;
          if (entry.consecutive_misses >= STREAM_SIZE) {
            // Detected a full stream of cache misses, start prefetching.
            uint64_t next_addr = addr + (PREFETCH_DISTANCE * entry.direction);
            for (int i = 0; i < PREFETCH_DEGREE; i++) {
              // This check ensures that prefetch addresses stay within the same page.
              if ((next_addr >> LOG2_PAGE_SIZE) == (entry.last_address >> LOG2_PAGE_SIZE)) {
                prefetchQueue.push_back(next_addr);
              }
              next_addr += (PREFETCH_DISTANCE * entry.direction);
            }
            entry.consecutive_misses = 0; // Reset consecutive misses.
          }
        } else {
          // Access is not in the same stream, reset consecutive misses and direction.
          entry.consecutive_misses = 0;
          entry.direction = 0;
        }
      }
      entry.last_address = addr;
    } else {
      // Create a new stream entry for this IP.
      StreamEntry newEntry;
      newEntry.last_address = addr;
      newEntry.direction = 0;
      newEntry.consecutive_misses = 1; // Initialize with 1 consecutive miss.
      streamTable[ip] = newEntry;
    }
  }
  return metadata_in;
}

// This function is called periodically to issue prefetch requests.
void CACHE::prefetcher_cycle_operate()
{
  // Issue prefetch requests from the queue.
  while (!prefetchQueue.empty()) {
    uint64_t addr = prefetchQueue.front();
    prefetchQueue.pop_front();

    // Check MSHR occupancy to decide whether to prefetch into L2 or LLC.

    // condition checks if the MSHR occupancy is less than half of the cache size. If true, it attempts to prefetch to the L2 cache.
    bool success = prefetch_line(0, 0, addr, (get_occupancy(0, addr) < get_size(0, addr) / 2), 0);
  }
}

// This function is called when a prefetch operation fills a cache line.
uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

// This function is called at the end to provide final prefetching statistics.
void CACHE::prefetcher_final_stats() {}