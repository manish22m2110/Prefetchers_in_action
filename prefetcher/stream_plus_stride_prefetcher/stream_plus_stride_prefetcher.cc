// #include "cache.h"

// void CACHE::prefetcher_initialize() {}

// uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in) { return metadata_in; }

// uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
// {
//   return metadata_in;
// }

// void CACHE::prefetcher_cycle_operate() {}

// void CACHE::prefetcher_final_stats() {}

#include <algorithm>
#include <array>
#include <deque> // Include deque for the stream buffer
#include <map>
#define LOG2_PAGE_SIZE 12



#include "cache.h"

constexpr int PREFETCH_DEGREE = 2;
constexpr int PREFETCH_DISTANCE = 4;
constexpr int STREAM_SIZE = 64; // Fixed stream size

struct tracker_entry {
  uint64_t ip = 0;              // the IP we're tracking
  uint64_t last_cl_addr = 0;    // the last address accessed by this IP
  int64_t last_stride = 0;      // the stride between the last two addresses accessed by this IP
  uint64_t last_used_cycle = 0; // use LRU to evict old IP trackers
};

struct lookahead_entry {
  uint64_t address = 0;
  int64_t stride = 0;
  int degree = 0; // degree remaining
};

constexpr std::size_t TRACKER_SETS = 256;
constexpr std::size_t TRACKER_WAYS = 4;
std::map<CACHE*, lookahead_entry> lookahead;
std::map<CACHE*, std::array<tracker_entry, TRACKER_SETS * TRACKER_WAYS>> trackers;

// Define a stream buffer
std::map<CACHE*, std::deque<uint64_t>> stream_buffer;

void CACHE::prefetcher_initialize()
{
  // Initialize the stream buffer with invalid addresses
  stream_buffer[this] = std::deque<uint64_t>(STREAM_SIZE, 0xFFFFFFFFFFFFFFFFULL);
  std::cout << NAME << " Stream + Stride prefetcher" << std::endl;
}


void CACHE::prefetcher_cycle_operate()
{
  // Check if there is an active lookahead
  if (auto [old_pf_address, stride, degree] = lookahead[this]; degree > 0) {
    uint64_t pf_address = old_pf_address + (stride << LOG2_BLOCK_SIZE);

    // Check if the next step would exceed the degree or run off the page
    if (degree > 1 || (pf_address >> LOG2_PAGE_SIZE) == (old_pf_address >> LOG2_PAGE_SIZE)) {
      // Check the MSHR occupancy to decide if we're going to prefetch to this
      // level or not
      bool success = prefetch_line(0, 0, pf_address, (get_occupancy(0, pf_address) < get_size(0, pf_address) / 2), 0);
      if (success) {
        lookahead[this] = {pf_address, stride, degree - 1};
      }
    } else {
      lookahead[this] = {};
    }
  } else {
    // Check the stream buffer for a continuous stream
    auto& stream = stream_buffer[this];

    bool is_stream = true;
    uint64_t stride_ = stream[1] - stream[0]; // Calculate the stride between the first two addresses
    int direction = ((stream[1] - stream[0]) > 0) ? 1 : -1;

    for (int i = 1; i < STREAM_SIZE - 1; ++i) {
      if (stream[i] + stride_ != stream[i + 1]) {
        is_stream = false;
        break;
      }
    }

    // If a stream is detected, prefetch the next address in the stream
    if (is_stream) {
      // Use the last tracked address and stride to determine the next address
      uint64_t next_address = stream[STREAM_SIZE - 1] + (PREFETCH_DISTANCE)*direction;
      for (int i = 0; i < PREFETCH_DEGREE; i++) {

        // Check the MSHR occupancy and prefetch to the next level if appropriate
        bool success = prefetch_line(0, 0, next_address, (get_occupancy(0, next_address) < get_size(0, next_address) / 2), 0);
        next_address += (stride_ << LOG2_BLOCK_SIZE);
      }
    }
  }
}

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, uint8_t type, uint32_t metadata_in)
{
  // Update the stream buffer with the new address
  stream_buffer[this].pop_front();     // Remove the oldest address
  stream_buffer[this].push_back(addr); // Add the new address

  uint64_t cl_addr = addr >> LOG2_BLOCK_SIZE;
  int64_t stride = 0;

  // get boundaries of the tracking set
  auto set_begin = std::next(std::begin(trackers[this]), ip % TRACKER_SETS);
  auto set_end = std::next(set_begin, TRACKER_WAYS);

  // find the current IP within the set
  auto found = std::find_if(set_begin, set_end, [ip](tracker_entry x) { return x.ip == ip; });

  // if we found a matching entry
  if (found != set_end) {
    // calculate the stride between the current address and the last address
    // no need to check for overflow since these values are downshifted
    stride = (int64_t)cl_addr - (int64_t)found->last_cl_addr;

    // Initialize prefetch state unless we somehow saw the same address twice in
    // a row or if this is the first time we've seen this stride
    if (stride != 0 && stride == found->last_stride)
      lookahead[this] = {cl_addr, stride, PREFETCH_DEGREE};
  } else {
    // replace by LRU
    found = std::min_element(set_begin, set_end, [](tracker_entry x, tracker_entry y) { return x.last_used_cycle < y.last_used_cycle; });
  }

  // update the tracking set
  *found = {ip, cl_addr, stride, current_cycle};

  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_final_stats() {}