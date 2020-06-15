#ifndef PROGRAM_INTERFACE_H
#define PROGRAM_INTERFACE_H

#include <cstdlib>
#include <math.h>
#include <vector>
#include <numeric>
#include <string>
#include <emp-tool/emp-tool.h>
#include "sequential_2pc.h"
#include "sequential_2pc_exec.h"
#include "SYGC_config.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

using namespace std;

void dec_vector_to_bin_1(bool*& bin, vector<int64_t> dec, vector<uint64_t> bit_len){
	uint64_t BIT_LEN = accumulate(bit_len.begin(), bit_len.end(), 0);
	uint64_t size = dec.size();
	uint64_t k = 0;
	for (uint64_t i = 0; i < size; i++){
		bitset<64> bits(dec[i]);
		for (uint64_t j = 0; j < bit_len[i]; j++){
			bin[BIT_LEN-1-k] = bits[bit_len[i]-1-j];
			k++;
		}
	}
}

class SYGCPI{
	public:
	NetIO* io;
	int party;	
	SequentialC2PC* twopc;
	
	vector<int64_t> input, output;
	vector<uint64_t> bit_width_A, bit_width_B;	
	//string input_hex_str, output_hex_str;
	
	lmkvm* lmkvm_A;
	lmkvm* lmkvm_B;
	
	uint64_t retreived_index_A = 0, retreived_index_B = 0;
	
	SYGCPI(NetIO* io, int party, int total_PRE = 10000, int total_ANDS = 10000) {
		this->party = party;
		this->io = io;		
		twopc = new SequentialC2PC(io, party, total_PRE, total_ANDS);
		io->flush();
		twopc->function_independent();
		io->flush();
	}	

	/***register inputs from Alice or Bob, must be followed by gen_input_labels and retrieve_input_labels***/
	
	void register_input(int owner, uint64_t bit_width, int64_t val = 0){
	/*Register input of a particular owner, must be called before gen_input_labels()
	val is the secret value of the input known to the owner, val is ignored if the input is owned by the other party*/
		if(owner == party) input.push_back(val);
		if(owner == ALICE) bit_width_A.push_back(bit_width);
		else bit_width_B.push_back(bit_width);
	}

	/***assign public or secret value to a secret variable***/
	
	void sign_extend(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width_target, uint64_t bit_width){
		y_x->copy(a_x, 0, bit_width);
		for (uint64_t i = bit_width; i < bit_width_target; i++)
			y_x->copy(a_x->at(bit_width-1), i, 1);
	}
	void unsign_extend(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width_target, uint64_t bit_width){
		y_x->copy(a_x, 0, bit_width);
		for (uint64_t i = bit_width; i < bit_width_target; i++)
			y_x->copy(twopc->const_lmkvm->at(0), i, 1);
	}
	
	void assign(lmkvm* y_x, int64_t val, uint64_t bit_width){
	/*y = val, val is known to both parties, bit_width <= 64*/
		bitset<64> bin(val);
		for (uint64_t i = 0; i < bit_width; i++){
			y_x->copy(twopc->const_lmkvm->at((uint64_t)bin[i]), i, 1);
		}
	}	
	void assign(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width){
	/*y = a_x*/
		y_x->copy(a_x, 0, bit_width);
	}	
	void assign(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width_y, uint64_t bit_width_a){
	/*y = a_x*/
		if(bit_width_a < bit_width_y){
			lmkvm* a1_x = new lmkvm(bit_width_y);
			sign_extend(a1_x, a_x, bit_width_y, bit_width_a);
			assign(y_x, a1_x, bit_width_y);
			delete a1_x;
		}
		else assign(y_x, a_x, bit_width_y);
	}
	
	/***create secret variable***/
	
	lmkvm* TG_int(uint64_t bit_width){
		lmkvm* Z;
		Z = new lmkvm(bit_width);
		return Z;
	}

	/***create secret variable and register input or assign public value***/
	
	lmkvm* TG_int_init(int owner, uint64_t bit_width, auto val){
		auto Z = TG_int(bit_width);
		
		if (owner == PUBLIC)
			assign(Z, val, bit_width);
		else
			register_input(owner, bit_width, val);
		
		return Z;
	}

	/***cleare memory alloacted to secret variable***/

	void clear_TG_int(auto& A){
		delete A;
	}

	/***generate input labels and retrieve to corresponding secret variables***/
	
	void gen_input_labels(){
	/*Generate and transfer the labels for inputs of both parties, includes OT*/
		reverse(input.begin(), input.end()); 
		reverse(bit_width_A.begin(), bit_width_A.end()); 
		reverse(bit_width_B.begin(), bit_width_B.end()); 
		
		uint64_t n1 = accumulate(bit_width_B.begin(), bit_width_B.end(), 0), n1_0 = 0;
		uint64_t n2 = accumulate(bit_width_A.begin(), bit_width_A.end(), 0), n2_0 = 0;
		uint64_t n = (party == ALICE)? n2: n1;
		uint64_t cycles_B = 1, cyc_rep_B = 1, cycles_A = 1, cyc_rep_A = 1;
		vector<uint64_t> bit_width = (party == ALICE)? bit_width_A: bit_width_B;
		
		bool *IN = new bool[n];
		dec_vector_to_bin_1(IN, input, bit_width);
		
		lmkvm_B = new lmkvm(cyc_rep_B*n1);
		lmkvm_A = new lmkvm(cyc_rep_A*n2);
		
		twopc->new_input_labels(n1, n2, lmkvm_B, lmkvm_A, IN);
		
		input.clear();
		input.shrink_to_fit();
		bit_width_A.clear();
		bit_width_A.shrink_to_fit();
		bit_width_B.clear();
		bit_width_B.shrink_to_fit();
		bit_width.clear();
		bit_width.shrink_to_fit();
		delete [] IN;
	}
	
	void clear_input_labels(){
	/*clear memory for generated input labels, should only be called after all the labels are retrieved*/
		delete lmkvm_A;
		delete lmkvm_B;
		retreived_index_A = 0;
		retreived_index_B = 0;
	}
	
	void retrieve_input_labels(lmkvm* retreived_lmkvm, int owner, uint64_t bit_width){
	/*Labels of input have to be retrieved in the same order they were registered with register_input(), must be called after gen_input_labels()*/
		if(owner == ALICE){ 
			retreived_lmkvm->copy(lmkvm_A->at(retreived_index_A), 0, bit_width);
			retreived_index_A += bit_width;
		}
		else {
			retreived_lmkvm->copy(lmkvm_B->at(retreived_index_B), 0, bit_width);
			retreived_index_B += bit_width;
		}
	}	

	/***reveal secret variable***/
	
	int64_t reveal(lmkvm* lmkvm_R, uint64_t bit_width, bool is_signed = true){
	/*Combine shares to reveal the secret value of an integer*/
		InputOutput* InOut = new InputOutput(0);
		InOut->init("", 0, bit_width, "", 0, 1);		
		bool *out = new bool[bit_width];
		memset(out, false, bit_width*sizeof(bool));
		
		twopc->reveal(bit_width, out, lmkvm_R);			
		InOut->fill_output(out);
		string output_hex_str = InOut->read_output();
		
		vector<int64_t> output;	
		parseGCOutputString(output, output_hex_str, bit_width, 0, is_signed);
		
		delete InOut;
		return output.back();
	}
	int64_t reveal(lmkvm* lmkvm_R){
		return reveal(lmkvm_R, lmkvm_R->num_bits);
	}

	/***arithmetic and logical operations***/
	
	void fun(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width, string op){
		string netlist_address = string(NETLIST_PATH_PI) + op + "_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void fun(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b, string op){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		lmkvm* a1_x = new lmkvm(bit_width);
		lmkvm* b1_x = new lmkvm(bit_width);
		sign_extend(a1_x, a_x, bit_width, bit_width_a);
		sign_extend(b1_x, b_x, bit_width, bit_width_b);
		fun(y_x, a1_x, b1_x, bit_width, op);
		delete a1_x;
		delete b1_x;
	}
	void fun(lmkvm* y_x, lmkvm* a_x, int64_t b, uint64_t bit_width, string op){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		fun(y_x, a_x, b_x, bit_width, op);
		delete b_x;
	}
	
	/*y = a + b*/
	void add(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "add");
	}	
	void add(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "add");
	}

	/*y = a - b*/
	void sub(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "sub");
	}	
	void sub(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "sub");
	}
	
	/*y = a < b*/
	void lt(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "lt");
	}	
	void lt(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "lt");
	}
	
	/*y = max(a, b)*/
	void max(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "max");
	}	
	void max(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "max");
	}
	
	/*y = min(a, b)*/
	void min(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		fun(y_x, a_x, b_x, bit_width, "min");
	}	
	void min(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		fun(y_x, a_x, b_x, bit_width_a, bit_width_b, "min");
	}
	
	void logic(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width, string op){
		string netlist_address = string(NETLIST_PATH_PI) + op + "_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void logic(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b, string op){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		lmkvm* a1_x = new lmkvm(bit_width);
		lmkvm* b1_x = new lmkvm(bit_width);
		unsign_extend(a1_x, a_x, bit_width, bit_width_a);
		unsign_extend(b1_x, b_x, bit_width, bit_width_b);
		logic(y_x, a1_x, b1_x, bit_width, op);
		delete a1_x;
		delete b1_x;
	}
	
	/*y = a & b*/
	void and_(lmkvm* y_x, lmkvm* a_x, auto b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "and");
	}	
	void and_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "and");
	}
	void and_(lmkvm* y_x, lmkvm* a_x, uint64_t b, uint64_t bit_width){
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width; i++)
			if (bits[i]) y_x->copy(a_x->at(i), i, 1);	
			else y_x->copy(twopc->const_lmkvm->at(0), i, 1);
	}
	
	/*y = a | b*/
	void or_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "or");
	}	
	void or_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "or");
	}
	void or_(lmkvm* y_x, lmkvm* a_x, uint64_t b, uint64_t bit_width){
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width; i++)
			if (bits[i]) y_x->copy(twopc->const_lmkvm->at(1), i, 1);	
			else y_x->copy(a_x->at(i), i, 1);
	}
	
	/*y = a ^ b*/
	void xor_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		logic(y_x, a_x, b_x, bit_width, "xor");
	}	
	void xor_(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		logic(y_x, a_x, b_x, bit_width_a, bit_width_b, "xor");
	}
	void xor_(lmkvm* y_x, lmkvm* a_x, uint64_t b, uint64_t bit_width){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		xor_(y_x, a_x, b_x, bit_width);
		delete b_x;
	}
	
	/*y = -a*/
	void neg(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width){
		lmkvm* zero = TG_int_init(PUBLIC, bit_width, (int64_t)0);
		sub(y_x, zero, a_x, bit_width);
		delete zero;
	}

	/*y = ~a*/
	void not_(lmkvm* y_x, lmkvm* a_x, uint64_t bit_width){
		lmkvm* one = new lmkvm(bit_width);
		for (uint64_t i = 0; i < bit_width; i++){
			one->copy(twopc->const_lmkvm->at(1), i, 1);
		}
		xor_(y_x, one, a_x, bit_width);
		delete one;
	}

	/*y = a * b*/
	void mult(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		string netlist_address = string(NETLIST_PATH_PI) + "mult_" + to_string(bit_width_a) + "_" + to_string(bit_width_b) + "_" + to_string(bit_width_a+bit_width_b-1) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}	
	void mult(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		mult(y_x, a_x, b_x, bit_width, bit_width);
	}
	void mult(lmkvm* y_x, lmkvm* a_x, int64_t b, uint64_t bit_width_a, uint64_t bit_width_b){
		uint64_t bit_width_y = bit_width_a + bit_width_b - 1;
		assign(y_x, (int64_t)0, bit_width_y);
		auto a1_x = TG_int(bit_width_a);
		if(b < 0) {
			neg(a1_x, a_x, bit_width_a);
			b = -b;
		}
		else assign(a1_x, a_x, bit_width_a);
		auto a2_x = TG_int(bit_width_y);
		assign(a2_x, a1_x, bit_width_y, bit_width_a);
		bitset<64> bits(b);
		for (uint64_t i = 0; i < bit_width_b; i++){
			if (bits[i]) add(y_x, y_x, a2_x, bit_width_y);
			left_shift(a2_x, 1, bit_width_y);
		}
		delete a1_x;
		delete a2_x;
	}
	
	/*y = a / b*/
	void div(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		string netlist_address = string(NETLIST_PATH_PI) + "div_" + to_string(bit_width_a) + "_" + to_string(bit_width_b) + "_" + to_string(bit_width_a) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;
		sequential_2pc_exec(twopc, b_x, a_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);
	}
	void div(lmkvm* y_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		div(y_x, a_x, b_x, bit_width, bit_width);
	}
		
	/*y = c? a : b*/
	void ifelse(lmkvm* y_x, lmkvm* c_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width){
		lmkvm* ab_x = new lmkvm(2*bit_width);
		ab_x->copy(b_x, 0, bit_width);
		ab_x->copy(a_x, bit_width, bit_width);		
		string netlist_address = string(NETLIST_PATH_PI) + "ifelse_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;	
		sequential_2pc_exec(twopc, c_x, ab_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);	
		delete ab_x;
	}
	void ifelse(lmkvm* y_x, lmkvm* c_x, lmkvm* a_x, lmkvm* b_x, uint64_t bit_width_a, uint64_t bit_width_b){
		uint64_t bit_width = MAX(bit_width_a, bit_width_b);
		lmkvm* a1_x = new lmkvm(bit_width);
		lmkvm* b1_x = new lmkvm(bit_width);
		sign_extend(a1_x, a_x, bit_width, bit_width_a);
		sign_extend(b1_x, b_x, bit_width, bit_width_b);
		ifelse(y_x, c_x, a1_x, b1_x, bit_width);
		delete a1_x;
		delete b1_x;
	}
	void ifelse(lmkvm* y_x, lmkvm* c_x, lmkvm* a_x, int64_t b, uint64_t bit_width){
		auto b_x = TG_int_init(PUBLIC, bit_width, b);
		lmkvm* ab_x = new lmkvm(2*bit_width);
		ab_x->copy(b_x, 0, bit_width);
		ab_x->copy(a_x, bit_width, bit_width);		
		string netlist_address = string(NETLIST_PATH_PI) + "ifelse_" + to_string(bit_width) + "bit.emp.bin";	
		CircuitFile cf(netlist_address.c_str(), true);
		uint64_t cycles = 1, repeat = 1, output_mode = 2;	
		sequential_2pc_exec(twopc, c_x, ab_x, nullptr, y_x, party, io, &cf, cycles, repeat, output_mode);	
		delete ab_x;
		delete b_x;
	}

	/*a << shift*/
	void left_shift(lmkvm* a_x, uint64_t shift, uint64_t bit_width){
		for (int64_t i = bit_width - 1; i >= (int64_t)shift; i--){
			a_x->copy(a_x->at(i - shift), i, 1);
		}
		for (int64_t i = shift - 1; i >= 0; i--){
			a_x->copy(twopc->const_lmkvm->at(0), i, 1);
		}
	}		
	/*a >> shift*/
	void right_shift(lmkvm* a_x, uint64_t shift, uint64_t bit_width){	
		a_x->copy(a_x->at(shift), 0, bit_width - shift);
		for (uint64_t i = bit_width - shift; i < bit_width; i++){
			a_x->copy(a_x->at(bit_width - 1), i, 1);
		}
	}

	/*relu(a)*/		
	void relu(lmkvm* a_x, uint64_t bit_width){
		lmkvm* sign_x = new lmkvm(1);
		not_(sign_x, a_x->at(bit_width-1), 1);
		lmkvm* mask_x = new lmkvm(bit_width);
		for (uint64_t i = 0; i < bit_width-1; i++){
			mask_x->copy(sign_x, i, 1);
		}
		mask_x->copy(twopc->const_lmkvm->at(0), bit_width-1, 1);
		and_(a_x, a_x, mask_x, bit_width);
	}

};

#endif //PROGRAM_INTERFACE_H