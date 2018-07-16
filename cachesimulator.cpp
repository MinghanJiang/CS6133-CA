/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss




struct config{
       int L1blocksize;
       int L1setsize;
       int L1size;
       int L2blocksize;
       int L2setsize;
       int L2size;
       };

/* you can define the cache class here, or design your own data structure for L1 and L2 cache*/
class cache {
public:

	cache(int cache_size, int cache_asct, int cache_blk)
	{
		cache_block.resize(cache_size);
		empty_bit.resize(cache_size);
		size = cache_size;
		blk = cache_blk;
		b = log2(cache_blk);
		if (cache_asct == 0) { // fully associated
			rr_cnt.resize(1);
			asct = cache_size / cache_blk;
			s = 0;
		}
		else { // direct mapped or set associative
			rr_cnt.resize(cache_size / (cache_asct*cache_blk));
			asct = cache_asct;
			s = log2(cache_size / (cache_asct*cache_blk));
		}
	}

	unsigned long get_index(bitset<32> addr) {
		bitset<32> addr_idx = bitset<32>(0);
		for (int i = 0; i < (s + b); i++) {
			addr_idx[i] = addr[i];
		}
		return addr_idx.to_ulong();
	}

	bool compare(bitset<32> addr, int index) {
		bool hit = false;
		for (int i = 0; i < asct; i++) {
			if (addr == cache_block[index + (size / asct)*i] && (empty_bit[index + (size / asct)*i] == '1')) {
				hit = true;
				break;
			}
			else {
				hit = false;
			}
		}
		return hit;
	}

	int check_avl(bitset<32> addr, int index) {
		int avl_idx = 0;
		for (int i = 0; i < asct; i++) {
			if (cache_block[index + (size / asct)*i].none() && (empty_bit[index + (size / asct)*i] == '0')) {
				avl_idx = index + (size / asct)*i;
				break;
			}
			else {
				avl_idx = -1;
			}
		}
		return avl_idx;
	}

	void place(int avl_idx, bitset<32> addr) {
		int offset = avl_idx % blk;
		for (int i = 0; i < blk; i++) {
			cache_block[avl_idx - offset + i] = bitset<32>(addr.to_ulong() - offset + i);
			empty_bit[avl_idx - offset + i] = '1';
		}
	}

	/*int get_rr(bitset<32> addr, int index) {
		bitset<32> set = bitset<32>(0);
		for (int i = 0; i < s; i++) {
			set[i] = addr[i + b];
		}
		unsigned long new_way = rr_cnt[set.to_ulong()];
		return new_way;
	}*/

	void replace(bitset<32> addr, int index) {
		bitset<32> set = bitset<32>(0);
		for (int i = 0; i < s; i++) {
			set[i] = addr[i+b];
		}
		unsigned long new_way = rr_cnt[set.to_ulong()];
		int offset = index % blk;
		for (int i = 0; i < blk; i++) {
			cache_block[index - offset + i + new_way*(size / asct)] = bitset<32>(addr.to_ulong() - offset + i);
			empty_bit[index - offset + i + new_way*(size / asct)] = '1';
		}
		if (rr_cnt[set.to_ulong()] == (asct - 1)) {
			rr_cnt[set.to_ulong()] = 0;
		}
		else {
			rr_cnt[set.to_ulong()] = rr_cnt[set.to_ulong()] + 1;
		}
	}
private:
	vector<bitset<32> > cache_block;
	vector<bitset<1> > empty_bit;
	vector<int> rr_cnt;
	int size;
	int asct;
	int blk;
	int s;
	int b;
};

int main(int argc, char* argv[]){


    
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while(!cache_params.eof())  // read config file
    {
      cache_params>>dummyLine;
      cache_params>>cacheconfig.L1blocksize;
      cache_params>>cacheconfig.L1setsize;              
      cache_params>>cacheconfig.L1size;
      cache_params>>dummyLine;              
      cache_params>>cacheconfig.L2blocksize;           
      cache_params>>cacheconfig.L2setsize;        
      cache_params>>cacheconfig.L2size;
      }
    
  
  
   // Implement by you: 
   // initialize the hirearch cache system with those configs
   // probably you may define a Cache class for L1 and L2, or any data structure you like
	cache L1(cacheconfig.L1size * 1024, cacheconfig.L1setsize, cacheconfig.L1blocksize);
	cache L2(cacheconfig.L2size * 1024, cacheconfig.L2setsize, cacheconfig.L2blocksize);
  
	long unsigned idx1 = 0;
	long unsigned idx2 = 0;
//	int rr_cnt1 = 0;
//	int rr_cnt2 = 0;
	bitset<32> evt_addr = bitset<32>(0);
   
   
  int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
  int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
   
   
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    
    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;        
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;
    
    if (traces.is_open()&&tracesout.is_open()){    
        while (getline (traces,line)){   // read mem access file and access Cache
            
            istringstream iss(line); 
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

			idx1 = L1.get_index(accessaddr);
			idx2 = L2.get_index(accessaddr);
           
           // access the L1 and L2 Cache according to the trace;
              if (accesstype.compare("R")==0)
              
             {    
                 //Implement by you:
                 // read access to the L1 Cache, 
                 //  and then L2 (if required), 
                 //  update the L1 and L2 access state variable;

				  if (L1.compare(accessaddr, idx1)) { // L1 read hits; L2 has no access
					  L1AcceState = RH;
					  L2AcceState = NA;
				  }
				  else {
					  L1AcceState = RM; // L1 misses
					  if (L2.compare(accessaddr, idx2)) { // L2 read hits
						  L2AcceState = RH;
						  if (L1.check_avl(accessaddr, idx1) > 0) { // L1 has empty cache block
							  L1.place(L1.check_avl(accessaddr, idx1), accessaddr);
						  }
						  else { // L1 has no empty block; has to be evicted
							  L1.replace(accessaddr, idx1); // replace L1
						  }
					  }
					  else { // L2 misses
						  L2AcceState = RM;
						  if (L2.check_avl(accessaddr, idx2) > 0) { // L2 has empty cache block
							  L2.place(L2.check_avl(accessaddr, idx2), accessaddr);
						  }
						  else {
							  L2.replace(accessaddr, idx2); // replace L2
						  }
						  if (L1.check_avl(accessaddr, idx1) > 0) { // L1 has empty cache block
							  L1.place(L1.check_avl(accessaddr, idx1), accessaddr);
						  }
						  else { // L1 has no empty block; has to be evicted
							  L1.replace(accessaddr, idx1); // replace L1
						  }
					  }
				  }
                                  
             }
             else 
             {    
                   //Implement by you:
                  // write access to the L1 Cache, 
                  //and then L2 (if required), 
                  //update the L1 and L2 access state variable;
				 if (L1.compare(accessaddr, idx1)) { // L1 write hits;
					 L1AcceState = WH;
					 L2AcceState = NA;
				 }
				 else { // L1 write misses; bypass L1
					 L1AcceState = WM;
					 if (L2.compare(accessaddr, idx2)) { // L2 write hits
						 L2AcceState = WH;
					 }
					 else {
						 L2AcceState = WM;
					 }
				 }          
                  }
              
              
             
            tracesout<< L1AcceState << " " << L2AcceState <<endl;  // Output hit/miss results for L1 and L2 to the output file;
             
             
        }
        traces.close();
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";


   
    
  

   
    return 0;
}
