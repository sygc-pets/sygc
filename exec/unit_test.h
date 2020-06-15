#ifndef PROGRAM_INTERFACE_EXEC_H
#define PROGRAM_INTERFACE_EXEC_H

#include "sygc/program_interface.h"
#include "exec/exec_common.h"

void unit_test(NetIO* io, int party) {
	
	SYGCPI* TGPI = new SYGCPI(io, party);
	io->flush();

	uint64_t a_bits = 19, b_bits = 24, m_bits = MAX(a_bits, b_bits);
	int64_t a = rand_L_U(), b = rand_L_U();
	cout << "testing with a = " << a << ", b = " << b << endl; 
	
	auto a_x = TGPI->TG_int_init(ALICE, a_bits, a);
	auto b_x = TGPI->TG_int_init(BOB, b_bits, b);
	TGPI->gen_input_labels();
	TGPI->retrieve_input_labels(a_x, ALICE, a_bits);
	TGPI->retrieve_input_labels(b_x, BOB, b_bits);
	TGPI->clear_input_labels();
	int64_t a_chk = TGPI->reveal(a_x, a_bits);
	int64_t b_chk = TGPI->reveal(b_x, b_bits);
	if (TGPI->party == BOB) 
		verify_n_report("register inputs, retrieve labels and reveal", (vector<int64_t>){a_chk, b_chk}, (vector<int64_t>){a, b});
	
	auto t0_x = TGPI->TG_int_init(PUBLIC, a_bits, a);
	auto t1_x = TGPI->TG_int(a_bits);
	TGPI->assign(t1_x, b_x, a_bits, b_bits);
	int64_t t0_chk = TGPI->reveal(t0_x, a_bits);	
	int64_t t1_chk = TGPI->reveal(t1_x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("assign public or secret values", (vector<int64_t>){t0_chk, t1_chk}, (vector<int64_t>){a, b});

	
	auto t2_x = TGPI->TG_int(m_bits);
	auto t2__x = TGPI->TG_int(a_bits);
	TGPI->add(t2_x, a_x, b_x, a_bits, b_bits);
	TGPI->add(t2__x, a_x, b, a_bits);
	int64_t t2_chk = TGPI->reveal(t2_x, m_bits);
	int64_t t2__chk = TGPI->reveal(t2__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("addition of public or secret values", (vector<int64_t>){t2_chk, t2__chk}, (vector<int64_t>){a + b, a + b});

	auto t3_x = TGPI->TG_int_init(PUBLIC, m_bits, (int64_t)0);
	TGPI->add(t3_x, t3_x, a_x, a_bits);
	TGPI->add(t3_x, t3_x, b_x, a_bits, b_bits);
	TGPI->add(t3_x, t3_x, t0_x, a_bits);
	TGPI->add(t3_x, t3_x, t1_x, a_bits);
	TGPI->add(t3_x, t3_x, t2_x, a_bits);
	int64_t t3_chk = TGPI->reveal(t3_x, m_bits);
	if (TGPI->party == BOB)
		verify_n_report("accumulation", t3_chk, a + b + t0_chk + t1_chk + t2_chk);

	auto t4_x = TGPI->TG_int(m_bits);
	auto t4__x = TGPI->TG_int(a_bits);
	TGPI->sub(t4_x, a_x, b_x, a_bits, b_bits);
	TGPI->sub(t4__x, a_x, b, a_bits);
	int64_t t4_chk = TGPI->reveal(t4_x, m_bits);
	int64_t t4__chk = TGPI->reveal(t4__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("subtraction of public or secret values", (vector<int64_t>){t4_chk, t4__chk}, (vector<int64_t>){a - b, a - b});

	auto t5_x = TGPI->TG_int(1);
	auto t5__x = TGPI->TG_int(1);
	TGPI->lt(t5_x, a_x, b_x, a_bits, b_bits);
	TGPI->lt(t5__x, a_x, b, a_bits);
	int64_t t5_chk = TGPI->reveal(t5_x, 1, false);
	int64_t t5__chk = TGPI->reveal(t5__x, 1, false);
	if (TGPI->party == BOB) 
		verify_n_report("comparison of public or secret values", (vector<int64_t>){t5_chk, t5__chk}, (vector<int64_t>){(int64_t)(a < b), (int64_t)(a < b)});

	auto t6_x = TGPI->TG_int(m_bits);
	auto t6__x = TGPI->TG_int(a_bits);
	TGPI->max(t6_x, a_x, b_x, a_bits, b_bits);
	TGPI->max(t6__x, a_x, b, a_bits);
	int64_t t6_chk = TGPI->reveal(t6_x, m_bits);
	int64_t t6__chk = TGPI->reveal(t6__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("maximum of public or secret values", (vector<int64_t>){t6_chk, t6__chk}, (vector<int64_t>){MAX(a, b), MAX(a, b)});

	auto t7_x = TGPI->TG_int(m_bits);
	auto t7__x = TGPI->TG_int(a_bits);
	TGPI->min(t7_x, a_x, b_x, a_bits, b_bits);
	TGPI->min(t7__x, a_x, b, a_bits);
	int64_t t7_chk = TGPI->reveal(t7_x, m_bits);
	int64_t t7__chk = TGPI->reveal(t7__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("minimum of public or secret values", (vector<int64_t>){t7_chk, t7__chk}, (vector<int64_t>){MIN(a, b), MIN(a, b)});


	auto t8_x = TGPI->TG_int(a_bits);
	TGPI->neg(t8_x, a_x, a_bits);
	int64_t t8_chk = TGPI->reveal(t8_x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("negate", t8_chk, -a);

	auto t9_x = TGPI->TG_int(a_bits + b_bits - 1);
	auto t9__x = TGPI->TG_int(a_bits + b_bits - 1);
	TGPI->mult(t9_x, a_x, b_x, a_bits, b_bits);
	TGPI->mult(t9__x, a_x, b, a_bits, b_bits);
	int64_t t9_chk = TGPI->reveal(t9_x, a_bits + b_bits - 1);
	int64_t t9__chk = TGPI->reveal(t9__x, a_bits + b_bits - 1);
	if (TGPI->party == BOB) 
		verify_n_report("multiplication of public or secret values", (vector<int64_t>){t9_chk, t9__chk}, (vector<int64_t>){a * b, a * b});

	auto t10_x = TGPI->TG_int(a_bits);
	TGPI->div(t10_x, a_x, b_x, a_bits, b_bits);
	int64_t t10_chk = TGPI->reveal(t10_x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("division", t10_chk, a/b);

	auto t11_x = TGPI->TG_int(m_bits);
	auto t11__x = TGPI->TG_int(a_bits);
	TGPI->ifelse(t11_x, t5_x, a_x, b_x, a_bits, b_bits);
	TGPI->ifelse(t11__x, t5_x, a_x, b, a_bits);
	int64_t t11_chk = TGPI->reveal(t11_x, m_bits);
	int64_t t11__chk = TGPI->reveal(t11__x, a_bits);
	if (TGPI->party == BOB) 
		verify_n_report("if else public or secret values", (vector<int64_t>){t11_chk, t11__chk}, (vector<int64_t>){MIN(a, b), MIN(a, b)});

	int64_t t12 = rand_L_U(), t13 = rand_L_U();
	cout << "testing shift with a = " << t12 << ", b = " << t13 << endl; 
	int64_t t12_ref = t12 << 3, t13_ref = t13 >> 3;
	auto t12_x = TGPI->TG_int_init(PUBLIC, m_bits, t12);
	auto t13_x = TGPI->TG_int_init(PUBLIC, m_bits, t13);
	TGPI->left_shift(t12_x, 3, m_bits);
	TGPI->right_shift(t13_x, 3, m_bits);
	int64_t t12_chk = TGPI->reveal(t12_x, m_bits);
	int64_t t13_chk = TGPI->reveal(t13_x, m_bits);
	if (TGPI->party == BOB) 
		verify_n_report("left and right shift", (vector<int64_t>){t12_chk, t13_chk}, (vector<int64_t>){t12_ref, t13_ref});

	uint64_t t14 = urand_U(), t15 = urand_U();
	auto t14_x = TGPI->TG_int_init(PUBLIC, a_bits, t14);
	auto t15_x = TGPI->TG_int_init(PUBLIC, b_bits, t15);
	cout << "testing logical op with a = " << t14 << ", b = " << t15 << endl; 

    auto t16_x = TGPI->TG_int(m_bits);
	auto t16__x = TGPI->TG_int(a_bits);
	TGPI->and_(t16_x, t14_x, t15_x, a_bits, b_bits);
	TGPI->and_(t16__x, t14_x, t15, a_bits);
	uint64_t t16_chk = TGPI->reveal(t16_x, m_bits, false);
	uint64_t t16__chk = TGPI->reveal(t16__x, a_bits, false);
	if (TGPI->party == BOB) 
		verify_n_report("logical and of public or secret values", (vector<uint64_t>){t16_chk, t16__chk}, (vector<uint64_t>){t14 & t15, t14 & t15});

	auto t17_x = TGPI->TG_int(m_bits);
	auto t17__x = TGPI->TG_int(a_bits);
	TGPI->or_(t17_x, t14_x, t15_x, a_bits, b_bits);
	TGPI->or_(t17__x, t14_x, t15, a_bits);
	uint64_t t17_chk = TGPI->reveal(t17_x, m_bits, false);
	uint64_t t17__chk = TGPI->reveal(t17__x, a_bits, false);
	if (TGPI->party == BOB) 
		verify_n_report("logical or of public or secret values", (vector<uint64_t>){t17_chk, t17__chk}, (vector<uint64_t>){t14 | t15, t14 | t15});

	auto t18_x = TGPI->TG_int(m_bits);
	auto t18__x = TGPI->TG_int(a_bits);
	TGPI->xor_(t18_x, t14_x, t15_x, a_bits, b_bits);
	TGPI->xor_(t18__x, t14_x, t15, a_bits);
	uint64_t t18_chk = TGPI->reveal(t18_x, m_bits, false);
	uint64_t t18__chk = TGPI->reveal(t18__x, a_bits, false);
	if (TGPI->party == BOB) 
		verify_n_report("logical xor of public or secret values", (vector<uint64_t>){t18_chk, t18__chk}, (vector<uint64_t>){t14 ^ t15, t14 ^ t15});

	auto t19_x = TGPI->TG_int(a_bits);
	TGPI->not_(t19_x, t14_x, a_bits);
	int64_t t19_chk = TGPI->reveal(t19_x, a_bits, false);
	uint64_t not_t14 = ~t14;
	for (uint64_t i = a_bits; i < 64; i++)
		not_t14 &= ~(1UL << i); // set the bits beyond a_bits to 0
	if (TGPI->party == BOB) 
		verify_n_report("logical not", t19_chk, not_t14);

	int64_t t20 = -urand_U(), t21 = urand_U();
	cout << "testing relu with a = " << t20 << ", b = " << t21 << endl; 	
	int64_t t20_ref = t20 > 0 ? t20 : 0, t21_ref = t21 > 0 ? t21 : 0;	
	auto t20_x = TGPI->TG_int_init(PUBLIC, a_bits, t20);	
	auto t21_x = TGPI->TG_int_init(PUBLIC, a_bits, t21);	
	TGPI->relu(t20_x, a_bits);	
	TGPI->relu(t21_x, a_bits);	
	int64_t t20_chk = TGPI->reveal(t20_x, a_bits);	
	int64_t t21_chk = TGPI->reveal(t21_x, a_bits);	
	if (TGPI->party == BOB) 	
		verify_n_report("relu", (vector<int64_t>){t20_chk, t21_chk}, (vector<int64_t>){t20_ref, t21_ref});

	TGPI->clear_TG_int (a_x);
	TGPI->clear_TG_int (b_x);
	TGPI->clear_TG_int (t0_x);
	TGPI->clear_TG_int (t1_x);
	TGPI->clear_TG_int (t2_x);
	TGPI->clear_TG_int (t2__x);
	TGPI->clear_TG_int (t3_x);
	TGPI->clear_TG_int (t4_x);
	TGPI->clear_TG_int (t4__x);
	TGPI->clear_TG_int (t5_x);
	TGPI->clear_TG_int (t5__x);
	TGPI->clear_TG_int (t6_x);
	TGPI->clear_TG_int (t6__x);
	TGPI->clear_TG_int (t7_x);
	TGPI->clear_TG_int (t7__x);
	TGPI->clear_TG_int (t8_x);
	TGPI->clear_TG_int (t9_x);
	TGPI->clear_TG_int (t9__x);
	TGPI->clear_TG_int (t10_x);
	TGPI->clear_TG_int (t11_x);
	TGPI->clear_TG_int (t11__x);
	TGPI->clear_TG_int (t12_x);
	TGPI->clear_TG_int (t13_x);
	TGPI->clear_TG_int (t16_x);
	TGPI->clear_TG_int (t16__x);
	TGPI->clear_TG_int (t17_x);
	TGPI->clear_TG_int (t17__x);
	TGPI->clear_TG_int (t18_x);
	TGPI->clear_TG_int (t18__x);
	TGPI->clear_TG_int (t19_x);
	TGPI->clear_TG_int (t20_x);
	TGPI->clear_TG_int (t21_x);
	
	delete TGPI;

	return;
}

#endif //PROGRAM_INTERFACE_EXEC_H
