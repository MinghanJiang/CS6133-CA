#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.


class RF
{
public:
	bitset<32> ReadData1, ReadData2;
	RF()
	{
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}

	void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
	{
		// implement the funciton by you. 
		ReadData1 = Registers[RdReg1.to_ulong()];
		ReadData2 = Registers[RdReg2.to_ulong()];
		if (WrtEnable.any()) {
			Registers[WrtReg.to_ulong()] = WrtData;
		}
	}

	void OutputRF()
	{
		ofstream rfout;
		rfout.open("RFresult.txt", std::ios_base::app);
		if (rfout.is_open())
		{
			rfout << "A state of RF:" << endl;
			for (int j = 0; j<32; j++)
			{
				rfout << Registers[j] << endl;
			}

		}
		else cout << "Unable to open file";
		rfout.close();

	}
private:
	vector<bitset<32> >Registers;

};

class ALU
{
public:
	bitset<32> ALUresult;
	bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
	{
		// implement the ALU operations by you. 
		unsigned long int op = ALUOP.to_ulong();
		if (op == ADDU)
		{
			bool carry = false;
			for (int i = 0; i < 32; i++)
			{
				ALUresult[i] = oprand1[i] ^ oprand2[i] ^ carry;
				carry = (oprand1[i] && oprand2[i]) | (oprand1[i] && carry) | (oprand2[i] && carry);
			}
		}
		else if (op == SUBU)
		{
			bool borrow = false;
			for (int i = 0; i < 32; i++)
			{
				if (!borrow) {
					ALUresult[i] = oprand1[i] ^ oprand2[i];
					borrow = !oprand1[i] && oprand2[i];
				}
				else {
					ALUresult[i] = !(oprand1[i] ^ oprand2[i]);
					borrow = !(oprand1[i] && !oprand2[i]);
				}
			}
		}
		else if (op == AND) {
			for (int i = 0; i < 32; i++) {
				ALUresult[i] = oprand1[i] && oprand2[i];
			}
		}
		else if (op == OR) {
			for (int i = 0; i < 32; i++) {
				ALUresult[i] = oprand1[i] | oprand2[i];
			}
		}
		else if (op == NOR) {
			for (int i = 0; i < 32; i++) {
				ALUresult[i] = !(oprand1[i] | oprand2[i]);
			}
		}
		return ALUresult;
	}
};

class INSMem
{
public:
	bitset<32> Instruction;
	INSMem()
	{
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i = 0;
		imem.open("imem.txt");
		if (imem.is_open())
		{
			while (getline(imem, line))
			{
				IMem[i] = bitset<8>(line);
				i++;
			}

		}
		else cout << "Unable to open file";
		imem.close();

	}

	bitset<32> ReadMemory(bitset<32> ReadAddress)
	{
		// implement by you. (Read the byte at the ReadAddress and the following three byte).
		unsigned long addr = ReadAddress.to_ulong();
		string str1, str2, str3, str4;
		str1 = IMem[addr].to_string();
		str2 = IMem[addr+1].to_string();
		str3 = IMem[addr+2].to_string();
		str4 = IMem[addr+3].to_string();
		Instruction = bitset <32>(str1 + str2 + str3 + str4);
		return Instruction;
	}

private:
	vector<bitset<8> > IMem;

};

class DataMem
{
public:
	bitset<32> readdata;
	DataMem()
	{
		DMem.resize(MemSize);
		ifstream dmem;
		string line;
		int i = 0;
		dmem.open("dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem, line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file";
		dmem.close();

	}
	bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
	{
		// implement by you.
		unsigned long addr = Address.to_ulong();
		string str1, str2, str3, str4;
		str1 = DMem[addr].to_string();
		str2 = DMem[addr + 1].to_string();
		str3 = DMem[addr + 2].to_string();
		str4 = DMem[addr + 3].to_string();
		if (readmem.any()) {
			readdata = bitset <32>(str1 + str2 + str3 + str4);
		}
		else if (writemem.any()) {
			for (int i = 31; i >= 0; i--) {
				if (i > 23) {
					DMem[addr][i - 24] = WriteData[i];
				}
				else if (i > 15) {
					DMem[addr + 1][i - 16] = WriteData[i];
				}
				else if (i > 7) {
					DMem[addr + 2][i - 8] = WriteData[i];
				}
				else {
					DMem[addr + 3][i] = WriteData[i];
				}
			}
		}
		return readdata;
	}

	void OutputDataMem()
	{
		ofstream dmemout;
		dmemout.open("dmemresult.txt");
		if (dmemout.is_open())
		{
			for (int j = 0; j< 1000; j++)
			{
				dmemout << DMem[j] << endl;
			}

		}
		else cout << "Unable to open file";
		dmemout.close();

	}

private:
	vector<bitset<8> > DMem;

};



int main()
{
	RF myRF;
	ALU myALU;
	INSMem myInsMem;
	DataMem myDataMem;
	bitset<32> PC = bitset<32>(0);

	while (1)
	{
		// Fetch
		bitset <32> ins = myInsMem.ReadMemory(PC);
		// If current insturciton is "11111111111111111111111111111111", then break;
		if (ins.all()) {
			break;
		}
		// decode(Read RF)
		bitset <6> op_code = bitset <6>(0);
		bitset <1> wrtenable = bitset <1>(0);
		bitset <3> alu_op = bitset <3>(0);
		bitset <32> alu_result = bitset <32>(0);
		bitset <32> rwdata = bitset <32>(0);
		bitset <32> signext = bitset <32>(0);
		bitset<32> nextPC = bitset<32>(0);
		bitset <5> rg1 = bitset <5>(0);
		bitset <5> rg2 = bitset <5>(0);
		bitset <5> rg3 = bitset <5>(0);
		bool isload = false;
		bool isstore = false;
		bool itype = false;
		bool jtype = false;
		bool isbranch = false;
		bool ishalt = false;
		for (int i = 26; i < 32; i++) {
			op_code[i - 26] = ins[i];
		}
		isload = (op_code == bitset<6>(0x23));
		isstore = (op_code == bitset<6>(0x2B));
		itype = ((op_code != bitset<6>(0)) && (op_code != bitset<6>(2)));
		jtype = (op_code == bitset<6>(2));
		isbranch = (op_code == bitset<6>(4));
		ishalt = (op_code == bitset<6>(0xFF));
		if (isstore | isbranch | jtype) {
			wrtenable = bitset<1>(0);
		}
		else {
			wrtenable = bitset<1>(1);
		}
		if (isload | isstore) {
			alu_op = bitset<3>(1);
		}
		else if (op_code.none()) {
			for (int i = 0; i < 3; i++) {
				alu_op[i] = ins[i];
			}
		}
		else {
			for (int i = 26; i < 29; i++) {
				alu_op[i-26] = ins[i];
			}
		}
		// Execute
		PC = bitset<32>(PC.to_ulong() + 4);
		if (!jtype && !ishalt) {
			for (int i = 0; i < 4; i++) {
				rg1[i] = ins[i + 21];
				rg2[i] = ins[i + 16];
				rg3[i] = ins[i + 11];
			}
			myRF.ReadWrite(rg1, rg2, rg3, bitset<32>(0), 0);
			if (itype) {
				if (isbranch) {
					if (myRF.ReadData1 == myRF.ReadData2) {
						signext.set(0, 0);
						signext.set(1, 0);
						for (int i = 2; i < 32; i++) {
							if (i < 18) {
								signext[i] = ins[i-2];
							}
							else {
								signext[i] = ins[15];
							}
						}
						PC = myALU.ALUOperation(bitset<3>(1), PC, signext);
					}
				}
				else {
					for (int i = 0; i < 32; i++) {
						if (i < 16) {
							signext[i] = ins[i];
						}
						else {
							signext[i] = ins[15];
						}
					}
					alu_result = myALU.ALUOperation(alu_op, myRF.ReadData1, signext);

				}
			}
			else {
				alu_result = myALU.ALUOperation(alu_op, myRF.ReadData1, myRF.ReadData2);
			}
		}
		else if (jtype) {
			nextPC.set(0, 0);
			nextPC.set(1, 0);
			for (int i = 2; i < 32; i++) {
				if (i < 28) {
					nextPC[i] = ins[i - 2];
				}
				else {
					nextPC[i] = PC[i];
				}
			}
			PC = nextPC;
		}
		else {
			break;
		}
		// Read/Write Mem
		if (isload) { 
			rwdata = myDataMem.MemoryAccess(alu_result, bitset<32>(0), bitset<1>(1), bitset<1>(0));
		}
		if (isstore) {
			rwdata = myDataMem.MemoryAccess(alu_result, myRF.ReadData2, bitset<1>(0), bitset<1>(1));
		}
		// Write back to RF
		if (wrtenable.any()) {
			if (itype) {
				if (isload) {
					myRF.ReadWrite(rg1, rg2, rg2, rwdata, wrtenable);
				}
				else {
					myRF.ReadWrite(rg1, rg2, rg2, alu_result, wrtenable);
				}
			}
			else {
				myRF.ReadWrite(rg1, rg2, rg3, alu_result, wrtenable);
			}
		}
		myRF.OutputRF();
	}
	myDataMem.OutputDataMem(); // dump data mem

	return 0;

}
