import std.stdio;
import std.stdint;
import std.file;
import core.runtime;

int main(){

	string fname = "codex.umz";
	uint32_t[][uint32_t] platters;
	platters[0] = cast(uint32_t[]) std.file.read(fname);
	//swap endianness:
	for(int i = 0; i < platters[0].length; i++){
		uint32_t temp = platters[0][i];
		platters[0][i] = ((temp & 0xFF000000) >> 24) | 
					((temp & 0x00FF0000) >> 8)  |
					((temp & 0x0000FF00) << 8)  |
					((temp & 0x000000FF) << 24);
	}
	
	uint32_t reg[] = cast(uint32_t[]) [0,0,0,0,0,0,0,0];
	int index = 0;
	int serial = 1;

	while(true){
		uint32_t opcode = platters[0][index];
		index++;
		uint32_t A = (opcode & 0b111000000) >> 6;
		uint32_t B = (opcode & 0b000111000) >> 3;
		uint32_t C = (opcode & 0b000000111);
		int opnum = opcode >> 28;
		//writeln("\t",opnum);
		if(opnum == 0){
			if(reg[C] != 0) reg[A] = reg[B];
		} else if(opnum == 1){
			reg[A] = platters[reg[B]][reg[C]];
		} else if(opnum == 2){
			platters[reg[A]][reg[B]] = reg[C];
		} else if(opnum == 3){
			reg[A] = reg[B] + reg[C];
		} else if(opnum == 4){
			reg[A] = reg[B] * reg[C];
		} else if(opnum == 5){
			reg[A] = reg[B] / reg[C];
		} else if(opnum == 6){
			reg[A] = (~reg[B]) | (~reg[C]);
		} else if(opnum == 7){
			return 0;
		} else if(opnum == 8){
			platters[serial] = new uint32_t[reg[C]];
			reg[B] = serial;
			serial++;
		} else if(opnum == 9){
			platters.remove(reg[C]);
		} else if(opnum == 10){
			write(cast(char)reg[C]);
		} else if(opnum == 11){
			write("input:");
			reg[C] = getchar();
		} else if(opnum == 12){
			platters[0] = platters[reg[B]].dup;
			index = reg[C];
		} else if(opnum == 13){
			A = (opcode >> 25) & 0b111;
			reg[A] = opcode & 33554431;
		}
	}
	return 0;
}
