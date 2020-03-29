//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"
#include <bits/stdc++.h>

#define GHB_SIZE 64
#define INDEX_SIZE 1 << 6
#define INDEX_MASK INDEX_SIZE-1

typedef struct GHB
{
    // Miss address
    unsigned long long int miss_addr;
    
    // Pointer to the previous miss address in GHB
    unsigned long long int link_pointer;

} GHB_t;

typedef struct index_table
{
    // Miss address
    unsigned long long int miss_addr;
    
    // Pointer to the GHB
    unsigned long long int pointer;

} index_table_t;

GHB_t GHB[GHB_SIZE];
index_table_t index_table[INDEX_SIZE];
unsigned long long int head_pointer;

void l2_prefetcher_initialize(int cpu_num)
{
    printf("No Prefetching\n");
    // you can inspect these knob values from your code to see which configuration you're runnig in
    printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);

    // Create Global History Buffer table


    // Set head pointer to 0
    head_pointer = 0;   

}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    // uncomment this line to see all the information available to make prefetch decisions
    // printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
    unsigned long long int addr_index = addr & INDEX_MASK;
    unsigned long long int current_pointer = 0;
    unsigned long long int end_pointer = 0;
    unordered_map<unsigned long long int,int> hash;

    if(cache_hit == 0)
    {
        /* Access the index table to check if there is such an address */
        if(index_table[addr_index].missaddr == ip)
        {   
            /* If there is an address, get pointer to the GHB */
            current_pointer = index_table[addr_index].pointer;

            /* Add the miss address to the GHB */
            GHB[head_pointer].miss_addr = ip;
            GHB[head_pointer].link_pointer = current_pointer;

            /* Iterate the linked list of the GHB to get the Markov Prefetching */
            while(GHB[current_pointer].link_pointer != 0 || current_pointer != head_pointer)
            {
                temp_addr = GHB[current_pointer+1].miss_addr;
                hash[temp_addr]++;
                current_pointer = GHB[current_pointer].link_pointer; 
            }
        }
        else
        {
            /* If there is no such address, update the index table */
            index_table[addr_index].missaddr = ip;
            index_table[addr_index].pointer = head_pointer; 
        }
        
        


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
