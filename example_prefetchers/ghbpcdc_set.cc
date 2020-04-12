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
#include <stack>

#define GHB_SIZE 256
#define INDEX_SIZE 256
#define PREFETCH_DEGREE 1
#define SET_ASSOCIATION 4
#define SET_SIZE INDEX_SIZE/SET_ASSOCIATION
#define SET_MASK SET_SIZE-1
 
//#define DEBUG
//#define TEST

typedef struct GHB
{
    // Miss address
    long long int miss_addr;

    // Pointer to the previous miss address in GHB
    long long int link_pointer;
    
} GHB_t;

typedef struct index_table
{
    // Instruction Address
    unsigned long long int ip;

    // Pointer to the GHB
    long long int pointer;

} index_table_t;

index_table_t index_table[INDEX_SIZE];
long long int global_pointer;
GHB_t GHB[GHB_SIZE];

std::deque<int> lru[SET_SIZE];

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
        index_table[i].ip = 0;
        index_table[i].pointer = -1;
    } 

    // Set head pointer to 0
    global_pointer = 0;   
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
    // uncomment this line to see all the information available to make prefetch decisions
    // printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
    unsigned long long int ip_index = ip & SET_MASK;
    long long int current_pointer = 0;
    long long int prev_pointer = 0;
    long long int delta_offset = 0;
    std::pair <long long int,long long int> delta_corr;
    std::stack<long long int> stack;

    delta_corr.first = 0;
    delta_corr.second = 0;
    
    for(int j=0; j<SET_ASSOCIATION; j++)
    {
        if(index_table[ip_index+j*SET_SIZE].ip == ip)
        {
            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = index_table[ip_index+j*SET_SIZE].pointer;

            for(std::deque<int>::iterator it = lru[ip_index].begin(); it != lru[ip_index].end();)
            {
                if(*it == j)
                {
                    it = lru[ip_index].erase(it);
                    break;
                } 
                else
                    it++;
            }
            
            lru[ip_index].push_front(j);
            index_table[ip_index+j*SET_SIZE].pointer = global_pointer;
            break;
        }
        else if(index_table[ip_index+j*SET_SIZE].pointer == -1)
        {
            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = global_pointer;

            index_table[ip_index+j*SET_SIZE].pointer = global_pointer;
            index_table[ip_index+j*SET_SIZE].ip = ip;

            lru[ip_index].push_front(j);

            global_pointer = (global_pointer+1) % GHB_SIZE;
            return;
        }
        
        //All the index are full
        if(j == SET_ASSOCIATION-1)
        {
            int i = lru[ip_index].back();
            lru[ip_index].pop_back();
            lru[ip_index].push_front(i);

            GHB[global_pointer].miss_addr = addr;
            GHB[global_pointer].link_pointer = global_pointer;

            index_table[ip_index+i*SET_SIZE].pointer = global_pointer;
            index_table[ip_index+i*SET_SIZE].ip = ip;

            global_pointer = (global_pointer+1) % GHB_SIZE;
            return;
        }
    }
    
    // Find the first pair of delta correleration
    current_pointer = global_pointer;
    int count = 0;

    #ifdef DEBUG 

    printf("(0x%llx 0x%llx %d %d %d)\n ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));
    std::cout << "Printing GHB Table" << std::endl;
    for(int i = 0; i < GHB_SIZE; i++)
    {   
        std::cout << std::hex << i << " " << GHB[i].miss_addr << " " << GHB[i].link_pointer << " " << GHB[i].delta << std::endl;
    } 
    std::cout << "Printing index Table" << std::endl;
    std::cout << index_table[ip_index].delta << " " << index_table[ip_index].pointer << " " << ip_index << " " << global_pointer << std::endl;

    #endif

    unsigned long long int distance = 0;

    while(current_pointer != GHB[current_pointer].link_pointer)
    {
        prev_pointer = GHB[current_pointer].link_pointer;
        if(current_pointer > prev_pointer)
        {
            distance += current_pointer - prev_pointer;
        }
        else
        {
            distance += current_pointer + GHB_SIZE - prev_pointer;
        }

        if(distance >= GHB_SIZE)
            break;
        
        //Calculate Delta Offset  
        delta_offset = GHB[current_pointer].miss_addr - GHB[prev_pointer].miss_addr;
        stack.push(delta_offset);
        if(count == 0)
        {
            delta_corr.second = stack.top();
            count++;
            current_pointer = prev_pointer;
        }    
        else if(count == 1)
        {
            delta_corr.first = stack.top();
            count++;
            current_pointer = prev_pointer;
            break;
        }
    }
        
    // Find the next delta pattern that correlate the pair of delta values
    while(current_pointer != GHB[current_pointer].link_pointer)
    {
        prev_pointer = GHB[current_pointer].link_pointer;
        if(current_pointer > prev_pointer)
        {
            distance += current_pointer - prev_pointer;
        }
        else
        {
            distance += current_pointer + GHB_SIZE - prev_pointer;
        }

        if(distance >= GHB_SIZE)
            break;

        delta_offset = GHB[current_pointer].miss_addr - GHB[prev_pointer].miss_addr;
        stack.push(delta_offset);
        if(count == 2)
        {
            if(delta_offset == delta_corr.second)
            {
                count++;
            }    
        }
        else if(count == 3)
        {
            if(delta_offset == delta_corr.first)
            {
                int temp = 0;
                long long int prefetch_addr = addr;
                // std::cout << "start" << std::endl;
                // std::cout << delta_corr.first << " " << delta_corr.second << std::endl;
                // std::cout << stack.top() << std::endl;
                stack.pop();
                // std::cout << stack.top() << std::endl;
                stack.pop();
                while(!stack.empty() && temp < PREFETCH_DEGREE)
                {  
                    int test;
                    prefetch_addr += stack.top();
                    if(get_l2_mshr_occupancy(0) < 8)
                        l2_prefetch_line(0, addr, prefetch_addr, FILL_L2);
                    else
                        l2_prefetch_line(0, addr, prefetch_addr, FILL_LLC);
                    // std::cout << test << " " << stack.top() << " " << prefetch_addr << std::endl;
                    stack.pop();
                    temp++;
                }
                break;
            }
            else if(delta_offset == delta_corr.second)
            {
                //Continue the code
            }
            else
            {
                count--;
            }
        }
        current_pointer = prev_pointer;   
    }

    /* Add global_pointer */
    global_pointer = (global_pointer+1) % GHB_SIZE;
    
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
