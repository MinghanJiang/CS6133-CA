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
//prediction state
#define SN 0 // Strong confidence not taken
#define WN 1 // Weak confidence not taken
#define WT 2 // Weak confidence taken
#define ST 3 // Strong confidence taken
#define T  1 // Taken
#define NT 0 // Not taken

class counter {
public:
		counter (int entry) {
			predictor.resize(pow(2,entry));
			index = entry;
			for (int i = 0; i < predictor.size(); i++){
				predictor[i] = bitset<2> (ST);
			}
		}

		int get_idx (bitset<32> pc) {
			bitset<32> idx = bitset<32>(0);
			for (int i = 0; i < index; i++) {
				idx[i] = pc[i];
			}
			return idx.to_ulong();
		}

		int predict(int idx, int action) {
			if ((predictor[idx] == bitset<2>(ST))||(predictor[idx] == bitset<2>(WT))) {
				return T;
			}
			else if ((predictor[idx] == bitset<2>(SN))||(predictor[idx] == bitset<2>(WN))) {
				return NT;
			}
			
		}
		
		bitset<2> get_cnt(int idx) {
			return predictor[idx];
		}

		void update(int idx, int action) {
			if (action == T) {
				if (predictor[idx] == bitset<2>(WT)) { 
					predictor[idx] = bitset<2>(ST);
				}
				else if (predictor[idx] == bitset<2>(WN)) { 
					predictor[idx] = bitset<2>(ST);
				}
				else if (predictor[idx] == bitset<2>(SN)) { 
					predictor[idx] = bitset<2>(WN);
				}
			}
			else if (action == NT) {
				if (predictor[idx] == bitset<2>(WT)) { 
					predictor[idx] = bitset<2>(SN);
				}
				else if (predictor[idx] == bitset<2>(WN)) { 
					predictor[idx] = bitset<2>(SN);
				}
				else if (predictor[idx] == bitset<2>(ST)) { 
					predictor[idx] = bitset<2>(WT);
				}
			}
		}
		
		
private:
	vector<bitset<2> > predictor;
	int index;
};

int main(int argc, char* argv[]){
	ifstream config;
	config.open(argv[1]);
	int nos_entry;
	 while(!config.eof())  // read config file
    {
      config>>nos_entry;
    }
	
	counter sat_cnt(nos_entry);
	
	int prediction = 0;
	int ttl_miss = 0;
	//int prev = 0;
	//int post = 0;

	ifstream traces;
    ofstream tracesout;	
	string outname;
    outname = string(argv[2]) + ".out";
    
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
	string line;
	string xaddr;			// PC in hex
	int action;				// Branch action
	unsigned int addr;		// PC in unsigend int
	bitset<32> pc;	// PC in bitset
	
	if (traces.is_open()&&tracesout.is_open()){    
		while (getline (traces,line)){
			istringstream iss(line); 
            if (!(iss >> xaddr >> action)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            pc = bitset<32> (addr);
			
			int pc_idx = sat_cnt.get_idx(pc);
			prediction = sat_cnt.predict(pc_idx,action);
			if (prediction != action){
				ttl_miss++;
			}
			//prev = (sat_cnt.get_cnt(pc_idx)).to_ulong();
			sat_cnt.update(pc_idx,action);
			//post = (sat_cnt.get_cnt(pc_idx)).to_ulong();

			tracesout<< prediction << endl;  // Output prediction results;
		}
		traces.close();
//		tracesout<< ttl_miss << endl; // Total miss no.
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";
	
	return 0;
}
