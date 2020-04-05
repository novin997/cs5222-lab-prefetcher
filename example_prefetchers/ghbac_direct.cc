//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"
#include <unordered_map>
#include <iostream>

#define GHB_SIZE 256
#define INDEX_SIZE 1 << 12
#define INDEX_MASK INDEX_SIZE-1

typedef struct GHB
{
    // Miss address
    unsigned long long int miss_addr;

    // Pointer to the previous miss address in GHB
    long long int link_pointer;

} GHB_t;

typedef struct index_table
{
    // Miss address
    unsigned long long int miss_addr;

    // Pointer to the GHB
    long long int pointer;

} index_table_t;

GHB_t GHB[GHB_SIZE];
index_table_t index_table[INDEX_SIZE];
long long int global_pointer;

void l2_prefetcher_initialize(int cpu_num)
{
    printf("No Prefetching\n");
    // you can inspect these knob values from your code to see which configuration you're runnig in
    printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);

    // Create Global History Buffer table
    for(int i = 0; i < GHB_SIZE; i++)
    {
        GHB[i].miss_addr = 0;
        GHB[i].link_pointer = -1;
    }

    // Create Index table
    for(int i = 0; i < INDEX_SIZE; i++)
    {
        index_table[i].miss_addr = 0;
        index_table[i].pointer = -1;
    }

    // Set head pointer to 0
    global_pointer = 0;   
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    // uncomment this line to see all the information available to make prefetch decisions
    // printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
    unsigned long long int ip_index = ip & INDEX_MASK;
    unsigned long long int temp_addr;
    long long int current_pointer = 0;
    long long int next_pointer;
    std::unordered_map<unsigned long long int, int> hash_table;

    if(cache_hit == 0)
    {
        /* Access the index table to check if there is such an address */
        if(index_table[ip_index].miss_addr == addr)
        {   
            /* If there is an address, get pointer to the GHB */
            current_pointer = index_table[ip_index].pointer;

            /* Add the miss address to the GHB */
            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = current_pointer;

            /* Iterate the linked list of the GHB to get the Markov Prefetching */
            /* Iterate until the first pointer or if the address does not match */
            /* The Hashmap will count the highest occurance of the next prefetch address */
            
            do
            {
                next_pointer = (current_pointer+1) % GHB_SIZE;
                temp_addr = GHB[next_pointer].miss_addr;
                hash_table[temp_addr]++;
                current_pointer = GHB[current_pointer].link_pointer;
            }while(GHB[current_pointer].link_pointer != current_pointer || addr != GHB[current_pointer].miss_addr);

            /* Find the highest number of occurances in the markov prefetch */
            int max_count = 0;
            unsigned long long int dataAddress = 0;
            for(auto i : hash_table)
            {
                if(max_count < i.second)
                {
                    dataAddress = i.first;
                    max_count = i.second;
                }
            }

            /* Prefetch the address the highest number of occurances */
            int temp = l2_prefetch_line(0, addr, dataAddress, FILL_L2);
            std::cout << temp << std::endl;

            /* Update index table to the current the pointer */
            index_table[ip_index].pointer = global_pointer;

        }
        else
        {
            /* If there is no such address, update the index table and GHB */
            index_table[ip_index].miss_addr = addr;
            index_table[ip_index].pointer = global_pointer;

            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = global_pointer;

        }
        
        /* Add global_pointer */
        global_pointer = (global_pointer+1) % GHB_SIZE;
    }
    else
    {
        /* code */
    }
}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
    // uncomment this line to see the information available to you when there is a cache fill event
    //printf("0x%llx %d %d %d 0x%llx\n", addr, set, way, prefetch, evicted_addr);
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
    printf("Prefetcher heartbeat stats\n");
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
    printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
    printf("Prefetcher final stats\n");
}
