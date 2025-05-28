#include "interpreter.h"
using namespace interpretator_model;

value_type Interpretator::VALUE_ID() {
	value_type t1;
	if ( clex == lex_id ) {
		t1 = IDENTIF();
		if (t1 == val_not_val)
			throw "unexpected label";
		return t1;
	}
	if ( is_constant(clex) ) {
		poliz.push_back( find_in_T_const(clex,t1) );
		next();
		return t1;
	}
	if ( clex == lex_branch_open ) {
		next();
		t1 = VALUE();
		cond_th(clex != lex_branch_close);
		next();
		return t1;
	}
	throw clex;
}
value_type Interpretator::VALUE_NOT() {
	value_type t1;
	if (clex == lex_plus || clex == lex_minus || clex == lex_not) {
		lexema_type lx = clex;
		next();
		t1 = VALUE_ID();
		poliz.push_back( make_lexema_for_poliz(lx,t1) );
		return to_rv(t1);
	}
	return VALUE_ID();
}
value_type Interpretator::VALUE_MUL_DIV() {
	value_type t1 = VALUE_NOT(),t2;
	if ( clex != lex_mul && clex != lex_div && clex != lex_mod )
		return t1;
	lexema_type lx;
	while ( clex == lex_mul || clex == lex_div || clex == lex_mod ) {
		lx = clex;
		next();
		t2 = VALUE_NOT();
		poliz.push_back(make_lexema_for_poliz(lx,t1,t2));
		t1 = val_result(t1,t2);
	}
	return to_rv(t1);
}
value_type Interpretator::VALUE_PLUS_MINUS() {
	value_type t1 = VALUE_MUL_DIV(),t2;
	if ( clex != lex_plus && clex != lex_minus )
		return t1;
	lexema_type lx;
	while ( clex == lex_plus || clex == lex_minus ) {
		lx = clex;
		next();
		t2 = VALUE_MUL_DIV();
		poliz.push_back(make_lexema_for_poliz(lx,t1,t2));
		t1 = val_result(t1,t2);
	}
	return to_rv(t1);
}
value_type Interpretator::VALUE_LE() {
	value_type t1 = VALUE_PLUS_MINUS(),t2;
	if ( clex == lex_G || clex == lex_GE || clex == lex_L || clex == lex_LE || clex == lex_E || clex == lex_NE ) {
		lexema_type lx = clex;
		next();
		t2 = VALUE_PLUS_MINUS();
		poliz.push_back(make_lexema_for_poliz(lx,t1,t2));
		return val_bool;
	}
	return t1;
}
value_type Interpretator::VALUE_AND() {
	value_type t1 = VALUE_LE(),t2;
	if ( clex != lex_and )
		return t1;
	while ( clex == lex_and ) {
		next();
		t2 = VALUE_LE();
		poliz.push_back(make_lexema_for_poliz(lex_and,t1,t2));
	}
	return val_bool;
}
value_type Interpretator::VALUE_OR() {
	value_type t1 = VALUE_AND(),t2;
	if ( clex != lex_or )
		return t1;
	while ( clex == lex_or ) {
		next();
		t2 = VALUE_AND();
		poliz.push_back(make_lexema_for_poliz(lex_or,t1,t2));
	}
	return val_bool;
}
value_type Interpretator::VALUE() {
	value_type t2, t1 = VALUE_OR();
	if ( clex == lex_mov || clex == lex_add || clex == lex_sub ) {
		if ( is_value_rv(t1) )
			throw "assignment to RValue";
		lexema_type lx = clex;
		next();
		t2 = VALUE();
		poliz.push_back(make_lexema_for_poliz(lx,t1,t2));
		t1 = t2;
	}
	return t1;
}
void Interpretator::GOTO() {
	next();
	cond_th( clex != lex_id );
	poliz.push_back( Lexema(lex_adress,-100) );
	Lexema lex(lex_adress,poliz.size()-1);
	poliz.push_back( Lexema(lex_jmp,0) );
	goto_counter.push_back(Id(bufer,lex));
	init_goto_and_label();
	next();
	cond_th(clex != lex_semicolon);
	next();
}
void Interpretator::CONTINUE_BREAK() {
	poliz.push_back( Lexema(lex_adress,0) );
	continue_break_counter.push(Lexema(clex,poliz.size()-1));
	poliz.push_back( Lexema(lex_jmp,0) );
	next();
	cond_th(clex != lex_semicolon);
	poliz.push_back(make_lexema_for_poliz(lex_semicolon));
	next();
}
value_type Interpretator::IDENTIF() {
	value_type t;
	string __s = bufer;
	lexema_type __lex = clex;
	next();
	if (clex == lex_colon) {
		bufer = __s;
		clex = __lex;
		make_new_label();
		init_goto_and_label();
		next();
		return val_not_val;
	} else {
		swap(__s,bufer);
		swap(__lex,clex);
		/* ПЕРЕМЕННАЯ */
		Lexema lex = find_in_T_var(t);
		poliz.push_back(lex);
        swap(__s,bufer);
		swap(__lex,clex);
		return to_lv(t);
	}
}
void Interpretator::MOVE(value_type t1) {
	value_type t2;
	lexema_type lx;
	cond_th(clex != lex_mov && clex != lex_add && clex != lex_sub );
	lx = clex;
	next();
	t2 = VALUE();
	poliz.push_back(make_lexema_for_poliz(lx,t1,t2));
	cond_th(clex != lex_semicolon);
	poliz.push_back(make_lexema_for_poliz(lex_semicolon));
	next();
	return;
}
void Interpretator::IF() {
	next();
	cond_th( clex != lex_branch_open );
	next();
	value_type t = VALUE();
	if (!(t == val_bool || t == val_bool) )
		throw " unexpected type in IF condition";
	cond_th( clex != lex_branch_close );
	next();
	int k = poliz.size();//запоминаем текущий размер (позицию)
	poliz.push_back( Lexema(lex_adress,-100) );//кладем в полиз заведомо ложный адрес
	poliz.push_back( Lexema(lex_jF,0) );//и лексему перехода по JF
	COMMAND();//рекурсивно обрабатываем следующие команды
	if ( clex == lex_else ) {
		next();
		int kk = poliz.size();//аналогично для else
		poliz.push_back(Lexema(lex_adress,-100));
		poliz.push_back(Lexema(lex_jmp,0));
		poliz[k].i = poliz.size()-1;//теперь, после рекурсивной обработки всех команд мы знаем размер стэка и его конец и можем положить хороший адрес
		COMMAND();
		poliz[kk].i = poliz.size()-1;
	} else {
		poliz[k].i = poliz.size()-1;
	}
}
void Interpretator::FOR() {
	next();
	cond_th( clex != lex_branch_open );
	next();
	if ( clex != lex_semicolon )
		VALUE();
	cond_th( clex != lex_semicolon );
	poliz.push_back(make_lexema_for_poliz(lex_semicolon));
	next();
	// labels and jmps
	int label_1 = poliz.size()-1;
	value_type t = to_rv( VALUE() );
	if (!(t == val_bool || t == val_int) )
		throw " unexpected type in FOR condition";
	int save_1;
	int save_2;
	poliz.push_back( Lexema(lex_adress,-100) );
	save_1 = poliz.size()-1;
	poliz.push_back( Lexema(lex_jF,0) );
	poliz.push_back( Lexema(lex_adress,-100) );
	save_2 = poliz.size()-1;
	poliz.push_back( Lexema(lex_jmp,0) );
	int label_2 = poliz.size()-1;
	cond_th(clex != lex_semicolon);
	next();
	if ( clex != lex_branch_close )
		VALUE();
	cond_th( clex != lex_branch_close );
	poliz.push_back(make_lexema_for_poliz(lex_semicolon));
	next();
	poliz.push_back( Lexema(lex_adress,label_1) );
	poliz.push_back( Lexema(lex_jmp,0) );
	poliz[save_2].i = poliz.size()-1;
	continue_break_counter.push( Lexema(lex_NULL,-1) );
	COMMAND();
	poliz.push_back( Lexema(lex_adress,label_2) );
	poliz.push_back( Lexema(lex_jmp,0) );
	poliz[save_1].i = poliz.size()-1;
	while ( continue_break_counter.top().type != lex_NULL ) {
		Lexema lex = continue_break_counter.top();
		continue_break_counter.pop();
		if (lex.type == lex_continue )
			poliz[lex.i].i = label_2;
		else
			poliz[lex.i].i = poliz.size()-1;
	}
	continue_break_counter.pop();
}
void Interpretator::WHILE() {
	next();
	cond_th( clex != lex_branch_open );
	next();
	int label_1 = poliz.size()-1;
	value_type t = VALUE();
	if (!(t == val_bool || t == val_int) )
		throw " unexpected type in WHILE condition";
	cond_th( clex != lex_branch_close );
	next();
	int kk = poliz.size();
	poliz.push_back(Lexema(lex_adress,-100));
	poliz.push_back(Lexema(lex_jF,0));
	continue_break_counter.push( Lexema(lex_NULL,-100) );
	COMMAND();
	poliz.push_back(Lexema(lex_adress,label_1));
	poliz.push_back(Lexema(lex_jmp,0));
	poliz[kk].i = poliz.size()-1;
	while ( continue_break_counter.top().type != lex_NULL ) {
		Lexema lex = continue_break_counter.top();
		continue_break_counter.pop();
		if (lex.type == lex_continue )
			poliz[lex.i].i = label_1;
		else
			poliz[lex.i].i = poliz.size()-1;
	}
	continue_break_counter.pop();
}
void Interpretator::READ_WRITE() {
	lexema_type rw = clex;
	next();
	cond_th(clex != lex_branch_open);
	next();
	value_type t;
	if ( rw == lex_read ) {
		while (1) {
			cond_th(clex != lex_id);
			// poliz.push_back( find_in_T_var(t) );
			// poliz.push_back( make_lexema_for_poliz(lex_read,t) );
			// next();
			t = IDENTIF();
			poliz.push_back( make_lexema_for_poliz(lex_read,t) );
			/**/
			if (clex == lex_comma) {
				next();
				continue;
			}
			if (clex == lex_branch_close)
				break;
		}
	} else {
		while (1) {
			t = VALUE();
			poliz.push_back( make_lexema_for_poliz(lex_write,t) );
			if (clex == lex_comma) {
				next();
				continue;
			}
			if (clex == lex_branch_close)
				break;
		}
	}
	next();
	cond_th(clex != lex_semicolon);
	poliz.push_back( Lexema(lex_semicolon,0) );
	next();
}
void Interpretator::COMMAND() {
    if ( clex == lex_id ) {
        value_type t = IDENTIF();
        if (t == val_not_val)
            return;
        MOVE(t);
        return;
	}
	if ( clex == lex_read || clex == lex_write ) {
		READ_WRITE();
		return;
	}
	if ( clex == lex_semicolon ) {
        next();
		return;
	}
    if ( clex == lex_if ) {
		IF();
		return;
	}
    if ( clex == lex_begin ) {
		next();
		COMMANDS();
		next();
		return;
	}
   	if ( clex == lex_while ) {
		WHILE();
		return;
	}
   	if ( clex == lex_for ) {
		FOR();
		return;
	}
    if ( clex == lex_continue || clex == lex_break ) {
		CONTINUE_BREAK();
		return;
	}
	if ( clex == lex_goto ) {
		GOTO();
		return;
	}
	throw clex;
}
void Interpretator::COMMANDS() {
	while ( clex != lex_end )
		COMMAND();
}
void Interpretator::DECLAR() {
	while ( clex == lex_int || clex == lex_bool || clex == lex_string || clex == lex_real || clex == lex_struct ) {
		value_type t1 = lex_to_val(clex), t2;
		Lexema lex1, lex2;
		do {
			next();
			if ( clex != lex_id ) throw clex;
			lex1 = make_new_var(t1);
			next();
			if ( clex == lex_mov ) {
				next();
				if ( !is_constant(clex) ) throw clex;
				lex2 = find_in_T_const(clex,t2);
				if ( t1 != t2 ) throw "different types of var and const";
				T_var[lex1.i].is_valued = true;
				T_var[lex1.i].v = T_const[lex2.i].v;
				next();
			}
			if ( clex == lex_comma )
				continue;
		} while ( clex != lex_semicolon );
		next();
	}
}
void Interpretator::semantic_analis() {
	// первое слово
    next(); // Lexical Analysis
	if ( clex != lex_program ) throw clex;
	next();
	if ( clex != lex_begin ) throw clex;
	next();
	DECLAR();
	COMMANDS();
	if ( clex != lex_end ) throw clex;
	next();
    if ( !goto_counter.empty() )
		throw "GOTO with no label";
	if ( !continue_break_counter.empty() )
		throw "BREAK or CONTINUE not in cycle";
	if ( clex == lex_NULL ) {
		poliz.push_back(Lexema(lex_semicolon, 0));
		poliz.push_back(Lexema(lex_NULL, 0));
        return;
    }
	throw clex;
}
void Interpretator::make_new_label() {
	int i = T_label.size() - 1;
	while ( i >= 0 ) {
		if (T_label[i].name == bufer)
			throw string("two labels '") + bufer + "' in one context";
		i -= 1;
	}
	Lexema lex(lex_t_label,poliz.size()-1);
	T_label.push_back(Id(bufer,lex));
}
Lexema Interpretator::make_new_var(value_type t) {
	int i = T_id.size() - 1;
	// пробег по таблице: если identif уже объявлен - ошибка
    while ( i >= 0 ) {
		if (T_id[i].name == bufer)
			throw std::string("variable " + T_id[i].name + " declarated twice");
		i -= 1;
	}
    // выделяем место на таблице переменных
	T_var.push_back(Var(t,false));
	Lexema lex(lex_t_var,T_var.size()-1);
    // в таблицу идентификатор оставляем ссылку на Эту запись в таблице переменных
	T_id.push_back(Id(bufer,lex));
	return lex;
}
Lexema Interpretator::find_in_T_var(value_type &t) {
	int i = T_id.size() - 1;
	while ( i >= 0 ) {
		if (T_id[i].name == bufer) {
			Lexema lex = T_id[i].lex;
			t = T_var[lex.i].type;
			return lex;
		}
		i -= 1;
	}
	throw string("undeclarated variable: " + bufer);
}
Lexema Interpretator::find_in_T_const(lexema_type lex, value_type &t) {
	switch(lex) {
	case lex_true: case lex_false:
		t = val_bool; return find_in_T_const_bool();
	case lex_int_const:
		t = val_int; return find_in_T_const_int();
	case lex_real_const:
		t = val_real; return find_in_T_const_real();
	case lex_string_const:
		t = val_string; return find_in_T_const_string();
	default: throw "CODE ERROR #7";
	}
}
Lexema Interpretator::find_in_T_const_int() {
	int i = T_const.size() - 1;
	long l = strtol(bufer.c_str(),NULL,10);
	while ( i >= 0 ) {
		if (T_const[i].type == val_int)
			if (T_const[i].v.i == l )
				return Lexema(lex_t_const,i);
		i -= 1;
	}
	T_const.push_back(Var(val_int,true));
	T_const[T_const.size()-1].v.i = l;
	return Lexema (lex_t_const,T_const.size()-1);
}
Lexema Interpretator::find_in_T_const_string() {
	int i = T_const.size() - 1;
	while ( i >= 0 ) {
		if (T_const[i].type == val_string)
			if (T_const[i].v.s == bufer )
				return Lexema(lex_t_const,i);
		i -= 1;
	}
	T_const.push_back(Var(val_string,true));
	T_const[T_const.size()-1].v.s = bufer;
	return Lexema (lex_t_const,T_const.size()-1);
}
Lexema Interpretator::find_in_T_const_real() {
	double k = strtod(bufer.c_str(),NULL);
	int i = T_const.size() - 1;
	while ( i >= 0 ) {
		if (T_const[i].type == val_real)
			if (T_const[i].v.r == k )
				return Lexema(lex_t_const,i);
		i -= 1;
	}
	T_const.push_back(Var(val_real,true));
	T_const[T_const.size()-1].v.r = k;
	return Lexema (lex_t_const,T_const.size()-1);
}
Lexema Interpretator::find_in_T_const_bool() {
	bool k = bufer == "true";
	int i = T_const.size() - 1;
	while ( i >= 0 ) {
		if (T_const[i].type == val_bool)
			if (T_const[i].v.b == k)
				return Lexema(lex_t_const,i);
		i -= 1;
	}
	T_const.push_back(Var(val_bool,true));
	T_const[T_const.size()-1].v.b = k;
	return Lexema (lex_t_const,T_const.size()-1);
}
void Interpretator::analyze() {
	make_alphabet();
	try {
		// cout << "\nlexan\n";
		// lexema_type lex;
		// 	do {
		// 		lex = get_lexem();
		// 		cout << lex_to_str(lex) << "\t--" << bufer << "--\n";
		// 	} while (lex != lex_NULL);
		// cout << "\nlexan\n";
		semantic_analis();
		cout << "compilated successfully\n";
		for (int i = 0; i < (int)poliz.size(); i++)
            cout << i << "\t" << lex_to_str(poliz[i].type) << '\t' << poliz[i].i << "\n";
        cout << "Вывод программы:" << "\n";
        execute();       
	}
	catch (char c) {
		printf("str_%d unexpected symbol: '%c'\n",in_what_string_am_i,c);
	}
	catch (const char * source) {
		printf("str_%d %s\n",in_what_string_am_i,source);
	}
	catch (std::string &source) {
		printf("str_%d %s\n",in_what_string_am_i,source.c_str());
	}
	catch (lexema_type source) {
		printf("str_%d unexpected: '%s' ",in_what_string_am_i,lex_to_str(source));
        if (source == lex_id || is_constant(source))
            cout << bufer;
        cout << "\n";
	}
}