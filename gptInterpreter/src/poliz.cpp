#include "interpreter.h"
using namespace interpretator_model;

void Interpretator::init_goto_and_label() {
	int i = T_label.size() - 1;
	int j;
	while ( i >= 0 ) {
		j = goto_counter.size() - 1;
		while (j >= 0) {
			if (T_label[i].name == goto_counter[j].name) {
				int from = goto_counter[j].lex.i;
				int to = T_label[i].lex.i;
				poliz[from].i = to; //указали метку куда прыгать
				goto_counter.erase(goto_counter.begin()+j);
			}
			j -= 1;
		}
		i -= 1;
	}
}
Lexema Interpretator::make_lexema_for_poliz (lexema_type lex, value_type op1, value_type op2) {
	op1 = to_rv(op1);
	op2 = to_rv(op2);
	if (lex == lex_semicolon)
		return Lexema(lex_semicolon,0);
	if ( (op1 == val_int && op2 == val_not_val) || (op1 == val_real && op2 == val_not_val) ) {
		switch (lex) {
			case lex_plus: return Lexema(lex_plus_unar,0);
			case lex_minus: return Lexema(lex_minus_unar,0);
			case lex_read: return Lexema(lex_read,0);
			case lex_write: return Lexema(lex_write,0);
			default: throw lex;
		}
	}
	if ( op1 == val_bool && op2 == val_not_val ) {
		switch (lex) {
			case lex_not: return Lexema(lex_not,0);
			case lex_read: return Lexema(lex_read,0);
			case lex_write: return Lexema(lex_write,0);
			default: throw lex;
		}
	}
	if ( op1 == val_string && op2 == val_not_val ) {
		switch (lex) {
			case lex_read: return Lexema(lex_read,0);
			case lex_write: return Lexema(lex_write,0);
			default: throw lex;
		}
	}
	if ( op1 == val_int && op2 == val_int ) {
		switch (lex) {
			case lex_plus: return Lexema(lex_plus_binar,0);
			case lex_minus: return Lexema(lex_minus_binar,0);
			case lex_mul: return Lexema(lex_mul,0);
			case lex_div: return Lexema(lex_div,0);
			case lex_mod: return Lexema(lex_mod,0);
			case lex_G: return Lexema(lex_G,0);
			case lex_GE: return Lexema(lex_GE,0);
			case lex_L: return Lexema(lex_L,0);
			case lex_LE: return Lexema(lex_LE,0);
			case lex_E: return Lexema(lex_E,0);
			case lex_NE: return Lexema(lex_NE,0);
			case lex_mov: return Lexema(lex_mov,0);;
			case lex_add: return Lexema(lex_add,0);
			case lex_sub: return Lexema(lex_sub,0);
			default: throw lex;
		}
	}
	if ( op1 == val_real && op2 == val_real ) {
		switch (lex) {
			case lex_plus: return Lexema(lex_plus_binar,0);
			case lex_minus: return Lexema(lex_minus_binar,0);
			case lex_mul: return Lexema(lex_mul,0);
			case lex_div: return Lexema(lex_div,0);
			case lex_G: return Lexema(lex_G,0);
			case lex_GE: return Lexema(lex_GE,0);
			case lex_L: return Lexema(lex_L,0);
			case lex_LE: return Lexema(lex_LE,0);
			case lex_E: return Lexema(lex_E,0);
			case lex_NE: return Lexema(lex_NE,0);
			case lex_mov: return Lexema(lex_mov,0);;
			case lex_add: return Lexema(lex_add,0);
			case lex_sub: return Lexema(lex_sub,0);
			default: throw lex;
		}
	}
	if ( (op1 == val_int && op2 == val_real) || (op1 == val_real && op2 == val_int) ) {
		switch (lex) {
			case lex_plus: return Lexema(lex_plus_binar_dt,0); //dt - different types
			case lex_minus: return Lexema(lex_minus_binar_dt,0);
			case lex_mul: return Lexema(lex_mul_dt,0);
			case lex_div: return Lexema(lex_div_dt,0);
			case lex_G: return Lexema(lex_G_dt,0);
			case lex_L: return Lexema(lex_L_dt,0);
			case lex_mov: return Lexema(lex_mov_dt,0);;
			case lex_add: return Lexema(lex_add_dt,0);
			case lex_sub: return Lexema(lex_sub_dt,0);
			default: throw lex;
		}
	}
	if ( op1 == val_string && op2 == val_string) {
		switch (lex) {
			case lex_plus: return Lexema(lex_plus_binar,0);
			case lex_G: return Lexema(lex_G,0);
			case lex_GE: return Lexema(lex_GE,0);
			case lex_L: return Lexema(lex_L,0);
			case lex_LE: return Lexema(lex_LE,0);
			case lex_E: return Lexema(lex_E,0);
			case lex_NE: return Lexema(lex_NE,0);
			case lex_mov: return Lexema(lex_mov,0);;
			case lex_add: return Lexema(lex_add,0);
			default: throw lex;
		}
	}
	if ( op1 == val_bool && op2 == val_bool) {
		switch (lex) {
			case lex_E: return Lexema(lex_E,0);
			case lex_NE: return Lexema(lex_NE,0);
			case lex_mov: return Lexema(lex_mov,0);
			case lex_or: return Lexema(lex_or,0);
			case lex_and: return Lexema(lex_and,0);
			default: throw lex;
		}
	}
	throw lex;
}