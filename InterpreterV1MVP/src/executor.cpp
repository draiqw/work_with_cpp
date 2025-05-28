#include "interpreter.h"
using namespace interpretator_model;

void Interpretator::oneparam_oper_execution(stack<Lexema> &st, Lexema lex) {
	Var vr = st.top().type == lex_t_var ? T_var[st.top().i] : T_const[st.top().i];
	Lexema what_oper_it_was = st.top();
	st.pop();
	if ( lex.type != lex_read )
		if ( ! vr.is_valued )
			throw "read from not-valued memory";
	switch (lex.type) {
		case lex_read:
			switch (vr.type) {
				case val_int: cin >> vr.v.i; break;
				case val_real: cin >> vr.v.r; break;
				case val_bool: cin >> vr.v.b; break;
				case val_string: cin >> vr.v.s; break;
				default: throw "CODE ERROR #8";
			}
			break;
		case lex_write:
			switch (vr.type) {
				case val_int: cout << vr.v.i << "\n"; break;;
				case val_real: cout << vr.v.r << "\n"; break;
				case val_bool: cout << vr.v.b << "\n"; break;
				case val_string: cout << vr.v.s << "\n"; break;
				default: throw "CODE ERROR #9";
			}
			break;
		case lex_not:
			vr.v.b = !vr.v.b;
			break;
		case lex_plus_unar:
			break;
		case lex_minus_unar:
			vr.v.i = -vr.v.i;
			vr.v.r = -vr.v.r;
			break;
		default: throw "CODE ERROR #10";
	}
	if ( lex.type == lex_read ) {
		vr.is_valued = true;
		T_var[what_oper_it_was.i] = vr;
	} else {
		T_var.push_back(vr);
		st.push(Lexema(lex_t_var,T_var.size()-1));
	}
}
void Interpretator::twoparam_oper_execution(stack<Lexema> &st, Lexema lex) {
	Var vr2 = st.top().type == lex_t_var ? T_var[st.top().i] : T_const[st.top().i];
	st.pop();
	if ( ! vr2.is_valued )
		throw "read from not-valued memory";
	Var vr1 = st.top().type == lex_t_var ? T_var[st.top().i] : T_const[st.top().i];
	Lexema var_index = st.top();
	st.pop();
	if ( ! (lex.type == lex_mov || lex.type == lex_mov_dt) )
		if ( ! vr1.is_valued )
			throw "read from not-valued memory";
	switch (lex.type) {
		case lex_mov:
			vr1.v = vr2.v;
			vr1.is_valued = true;
			break;
		case lex_mov_dt:
			vr1.v.i = vr2.v.r;
			vr1.v.r = vr2.v.i;
			break;
		case lex_add: case lex_plus_binar:
			vr1.v += vr2.v;
			break;
		case lex_add_dt:
			vr1.v.i += vr2.v.r;
			vr1.v.r += vr2.v.i;
			break;
		case lex_sub: case lex_minus_binar:
			vr1.v -= vr2.v;
			break;
		case lex_sub_dt:
			vr1.v.i -= vr2.v.r;
			vr1.v.r -= vr2.v.i;
			break;
		case lex_mul:
			vr1.v.i = vr1.v.i * vr2.v.i;
			vr1.v.r = vr1.v.r * vr2.v.r;
			break;
		case lex_plus_binar_dt:
			if (vr1.type == val_int) {
				vr1.type = val_real;
				vr1.v.r = vr1.v.i + vr2.v.r;
			} else {
				vr1.v.r = vr1.v.r + vr2.v.i;
			}
			break;
		case lex_minus_binar_dt:
			if (vr1.type == val_int) {
				vr1.type = val_real;
				vr1.v.r = vr1.v.i - vr2.v.r;
			} else {
				vr1.v.r = vr1.v.r - vr2.v.i;
			}
			break;
		case lex_mul_dt:
			vr1.v.i = vr1.v.i * vr2.v.r;
			vr1.v.r = vr1.v.r * vr2.v.i;
			break;
		case lex_div:
			if ( vr1.type == val_int )
				vr1.v.i = vr1.v.i / vr2.v.i;
			else
				vr1.v.r = vr1.v.r / vr2.v.r;
			break;
		case lex_div_dt:
			if ( vr1.type == val_int )
				vr1.v.i = vr1.v.i / vr2.v.r;
			else
				vr1.v.r = vr1.v.r / vr2.v.i;
			break;
		case lex_mod:
			vr1.v.i = vr1.v.i % vr2.v.i;
			break;
		case lex_G:
			switch(vr1.type) {
				case val_int: vr1.v.b = vr1.v.i > vr2.v.i; break;
				case val_string: vr1.v.b = vr1.v.s > vr2.v.s; break;
				case val_real: vr1.v.b = vr1.v.r > vr2.v.r; break;
				default: throw "CODE ERROR #11";
			}
			break;
		case lex_G_dt:
			if ( vr1.type == val_int )
				vr1.v.b = vr1.v.i > vr2.v.r;
			else
				vr1.v.b = vr1.v.r > vr2.v.i;
			break;
		case lex_GE:
			switch(vr1.type) {
				case val_int: vr1.v.b = vr1.v.i >= vr2.v.i; break;
				case val_string: vr1.v.b = vr1.v.s >= vr2.v.s; break;
				case val_real: vr1.v.b = vr1.v.r >= vr2.v.r; break;
				default: throw "CODE ERROR #12";
			}
			break;
		case lex_L:
			switch(vr1.type) {
				case val_int: vr1.v.b = vr1.v.i < vr2.v.i; break;
				case val_string: vr1.v.b = vr1.v.s < vr2.v.s; break;
				case val_real: vr1.v.b = vr1.v.r < vr2.v.r; break;
				default: throw "CODE ERROR #13";
			}
			break;
		case lex_L_dt:
			if ( vr1.type == val_int )
				vr1.v.b = vr1.v.i < vr2.v.r;
			else
				vr1.v.b = vr1.v.r < vr2.v.i;
			break;
		case lex_LE:
			switch(vr1.type) {
				case val_int: vr1.v.b = vr1.v.i <= vr2.v.i; break;
				case val_string: vr1.v.b = vr1.v.s <= vr2.v.s; break;
				case val_real: vr1.v.b = vr1.v.r <= vr2.v.r; break;
				default: throw "CODE ERROR #14";
			}
			break;
		case lex_E:
			switch(vr1.type) {
				case val_int: vr1.v.b = vr1.v.i == vr2.v.i; break;
				case val_string: vr1.v.b = vr1.v.s == vr2.v.s; break;
				case val_real: vr1.v.b = vr1.v.r == vr2.v.r; break;
				case val_bool: vr1.v.b = vr1.v.b == vr2.v.b; break;
				default: throw "CODE ERROR #15";
			}
			break;
		case lex_NE:
			switch(vr1.type) {
				case val_int: vr1.v.b = vr1.v.i != vr2.v.i; break;
				case val_string: vr1.v.b = vr1.v.s != vr2.v.s; break;
				case val_real: vr1.v.b = vr1.v.r != vr2.v.r; break;
				case val_bool: vr1.v.b = vr1.v.b != vr2.v.b; break;
				default: throw "CODE ERROR #16";
			}
			break;
		case lex_and:
			vr1.v.b = vr1.v.b && vr2.v.b;
			break;
		case lex_or:
			vr1.v.b = vr1.v.b || vr2.v.b;
			break;
		default: throw "CODE ERROR #17";
	}
	switch(lex.type) {
		case lex_G: case lex_GE: case lex_L: case lex_LE:
		case lex_E: case lex_NE:
		case lex_G_dt: case lex_L_dt:
		vr1.type = val_bool;
		default: ;
	}
	if ( ! (lex.type == lex_mov || lex.type == lex_mov_dt
			|| lex.type == lex_add || lex.type == lex_add_dt
			|| lex.type == lex_sub || lex.type == lex_sub_dt ) ) {
		T_var.push_back(vr1);
		st.push(Lexema(lex_t_var,T_var.size()-1));
	} else {
		st.push(var_index);
		T_var[var_index.i].v = vr1.v;
		T_var[var_index.i].is_valued = true;
	}
}
void Interpretator::execute() {
	int size_T_var = T_var.size();
	Lexema lex;
	Var vr;
	stack <Lexema> args;
	int index = 0,kk;
    bool jmp_cond;
	while ( poliz[index].type != lex_NULL ) {
		lex = poliz[index];
		switch(lex.type) {
		case lex_t_var: case lex_t_const: case lex_adress:
			args.push(lex);
			break;
		case lex_semicolon:
			while ( !args.empty() )
				args.pop();
			while ((int)T_var.size() != size_T_var)
				T_var.pop_back();
			break;
		case lex_jmp:
			index = args.top().i;
			args.pop();
			break;
		case lex_jF:
			kk = args.top().i;
			args.pop();
			vr = args.top().type == lex_t_var ?
				T_var[args.top().i] : T_const[args.top().i];
			args.pop();
			jmp_cond = vr.v.b;
            if (vr.type == val_int || vr.type == val_lv_int)
                jmp_cond = vr.v.i;
            if ( !jmp_cond )
				index = kk;
			break;
		default:
			if ( is_oper(lex.type) ) {
				if (is_oneparametr(lex.type)) // unar+ unar- not read write
					oneparam_oper_execution(args,lex);
				else
					twoparam_oper_execution(args,lex); // other
				break;
			}
            cout << lex_to_str(lex.type) << "\n";
			throw "CODE ERROR #1";
		}
		index += 1;
	}
	if ( size_T_var != (int)T_var.size() )
		throw "CODE ERROR #2";
	if ( !args.empty() )
		throw "CODE ERROR #3";
}