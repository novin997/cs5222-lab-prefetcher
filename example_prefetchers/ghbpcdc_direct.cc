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
#include <map>
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
    
    // Delta
    long long int delta;

} GHB_t;

typedef struct index_table
{
    // Miss address
    long long int delta;

    // Pointer to the GHB
    long long int pointer;

} index_table_t;

index_table_t index_table[INDEX_SIZE];
long long int global_pointer;
GHB_t GHB[GHB_SIZE];

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
        GHB[i].delta = 0;
    }

   // Create Index table
    for(int i = 0; i < INDEX_SIZE; i++)
    {
        index_table[i].delta = 0;
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
    long long int current_pointer = 0;
    long long int prev_pointer = (global_pointer-1) % GHB_SIZE;
    long long int delta_offset;
    std::pair <long long int,long long int> delta_corr;   
    std::unordered_map<unsigned long long int, int> hash_table;

    delta_corr.first = 0;
    delta_corr.second = 0;

    if(cache_hit == 0)
    {
        if(index_table[ip_index].pointer == -1)
        {
            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = global_pointer;

        }
        else
        {
            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = index_table[ip_index].pointer;
        }

        //calculate delta
        if(prev_pointer < 0)
            prev_pointer += GHB_SIZE;

        if(GHB[global_pointer].link_pointer != -1 && GHB[global_pointer].link_pointer != -1)
        {
            GHB[global_pointer].delta = GHB[global_pointer].miss_addr - GHB[prev_pointer].miss_addr;     
        }
        else
        {
            GHB[global_pointer].delta = 0;
        }

        // Find the first pair of delta correleration
        current_pointer = global_pointer;
        int count = 0;
        
        while(current_pointer != GHB[current_pointer].link_pointer)
        {
            if(count == 0)
            {
                delta_corr.second = GHB[current_pointer].delta;
                count++;
                current_pointer = GHB[current_pointer].link_pointer;
            }    
            else if(count == 1)
            {
                delta_corr.first = GHB[current_pointer].delta;
                count++;
                current_pointer = GHB[current_pointer].link_pointer;
                break;
            }
        }

        // Find the next delta pattern that correlate the pair of delta values
        while(current_pointer != GHB[current_pointer].link_pointer)
        {
            if(count == 2)
            {
                if(GHB[current_pointer].delta == delta_corr.second)
                {
                    delta_offset = GHB[current_pointer+1].delta;
                    count++;
                }    
            }
            else if(count == 3)
            {
                if(GHB[current_pointer].delta == delta_corr.first)
                {
                    count++;
                    break;
                }
                else if(GHB[current_pointer].delta == delta_corr.second)
                {
                    delta_offset = GHB[current_pointer+1].delta;
                }
                else
                {
                    count--;
                }
            }
            current_pointer = GHB[current_pointer].link_pointer;
        }

        if(count == 4)
        {
            int temp = l2_prefetch_line(0, addr, (long long int)GHB[global_pointer].miss_addr + delta_offset, FILL_L2);
            std::cout << temp << std::endl;
        }
        else
        {
            
        }
         
        index_table[global_pointer].delta = GHB[global_pointer].delta;
        index_table[global_pointer].pointer = global_pointer;
        
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
