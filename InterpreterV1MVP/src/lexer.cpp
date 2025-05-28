#include "interpreter.h"
using namespace interpretator_model;

const char* Interpretator::lex_to_str(lexema_type t) {
	switch(t) {
		case lex_program: return "program";
		case lex_if: return "if";
		case lex_else: return "else";
		case lex_for: return "for";
		case lex_while: return "while";
		case lex_goto: return "goto";
		case lex_continue: return "continue";
		case lex_break: return "break";
		case lex_int: return "int";
		case lex_bool: return "bool";
		case lex_real: return "real";
		case lex_string: return "string";
		case lex_id: return "id";
		case lex_begin: return "{";
		case lex_end: return "}";
		case lex_branch_open: return "(";
		case lex_branch_close: return ")";
		case lex_comma: return ",";
		case lex_dot: return ".";
		case lex_struct: return "struct";
		case lex_colon: return ":";
		case lex_t_id: return "__id";
		case lex_t_label: return "__label";
		case lex_jmp: return "__jmp";
		case lex_jF: return "__jF";
		case lex_t_var: return "__var";
		case lex_t_const: return "__const";
		case lex_adress: return "__addr";
		case lex_NULL: return "__END_PROGRAM";
		case lex_semicolon: return ";";
		case lex_read: return "read";
		case lex_write: return "write";
		case lex_plus: return "+";
		case lex_plus_unar: return "+un";
		case lex_plus_binar: return "+bin";
		case lex_plus_binar_dt: return "+bin";
		case lex_minus: return "-";
		case lex_minus_unar: return "-un";
		case lex_minus_binar: return "-bin";
		case lex_minus_binar_dt: return "-bin";
		case lex_mul: return "*";
		case lex_mul_dt: return "*";
		case lex_div: return "/";
		case lex_div_dt: return "/";
		case lex_mod: return "%";
		case lex_G: return ">";
		case lex_G_dt: return ">";
		case lex_GE: return ">=";
		case lex_L: return "<";
		case lex_L_dt: return "<";
		case lex_LE: return "<=";
		case lex_E: return "==";
		case lex_NE: return "!=";
		case lex_and: return "&&";
		case lex_or: return "||";
		case lex_not: return "!";
		case lex_mov: return "=";
		case lex_mov_dt: return "=";
		case lex_add: return "+=";
		case lex_add_dt: return "+=";
		case lex_sub: return "-=";
		case lex_sub_dt: return "-=";
		case lex_true: return "true";
		case lex_false: return "false";
		case lex_int_const: return "__const_int";
		case lex_string_const: return "__const_string";
		case lex_real_const: return "__const_real";
		case lex_t_struct: return "__strct";
		default: throw "CODE ERROR #4";
	}
}
lexema_type Interpretator::read_operator() {
	char c1 = cur_chr, c2;
	fscanf(file,"%c",&cur_chr);
	/* однозначно односимвольные лексемы */
	switch (c1) {
		case ':': return lex_colon;
		case ',': return lex_comma;
		case '.': return lex_dot;
		case ';': return lex_semicolon;
		case '{': return lex_begin;
		case '}': return lex_end;
		case '(': return lex_branch_open;
		case ')': return lex_branch_close;
		case '*': return lex_mul;
		case '/': return lex_div;
		case '%': return lex_mod;
		default: break;
	}
	/* перебор односимвольных комбинаций */
	c2 = cur_chr;
	if ( c1 == '+' && c2 != '=' )
		return lex_plus;
	if ( c1 == '-' && c2 != '=' )
		return lex_minus;
	if ( c1 == '=' && c2 != '=' )
		return lex_mov;
	if ( c1 == '<' && c2 != '=' )
		return lex_L;
	if ( c1 == '>' && c2 != '=' )
		return lex_G;
	if ( c1 == '!' && c2 != '=' )
		return lex_not;
	if ( !is_symbol_operator(c2) )
		throw "unknown lexema";
	/* двусимвольные комбинации */
	fscanf(file,"%c",&cur_chr);
	if ( c1 == '&' && c2 == '&' )
		return lex_and;
	if ( c1 == '|' && c2 == '|' )
		return lex_or;
	if ( c1 == '+' && c2 == '=' )
		return lex_add;
	if ( c1 == '-' && c2 == '=' )
		return lex_sub;
	if ( c1 == '=' && c2 == '=' )
		return lex_E;
	if ( c1 == '<' && c2 == '=' )
		return lex_LE;
	if ( c1 == '>' && c2 == '=' )
		return lex_GE;
	if ( c1 == '!' && c2 == '=' )
		return lex_NE;
	throw "unknown lexema";
}
lexema_type Interpretator::read_number() {
	int was_dot = 0;
	bufer.push_back(cur_chr);
	// допустимо: 12, 148, 17.6 но НЕ 1.2.3 и НЕ 0.
	while (1) {
		fscanf(file,"%c",&cur_chr);
		if ( is_digit(cur_chr) ) {
			bufer.push_back(cur_chr);
			continue;
		}
		if ( cur_chr == '.' ) {
			if (was_dot)
				throw "two DOTs in one number";
			was_dot = 1;
			bufer.push_back('.');
			fscanf(file,"%c",&cur_chr);
			if ( !is_digit(cur_chr) )
				throw "unexpected DOT after digit, bun not a real number";
			bufer.push_back(cur_chr);
			continue;
		}
		if ( is_space(cur_chr) || is_symbol_operator(cur_chr) ) {
			if (was_dot)
				return lex_real_const;
			else
				return lex_int_const;
		}
		if ( is_letter(cur_chr) )
			throw "alpha after digit";
		throw cur_chr;
	}
}
lexema_type Interpretator::read_word() {
	bufer.push_back(cur_chr);
	while (1) {
		fscanf(file,"%c",&cur_chr);
		if ( is_letter(cur_chr) || is_digit(cur_chr) ) {
			// идентификатор продолжается
			bufer.push_back(cur_chr);
			continue;
		}
		if ( is_space(cur_chr) || is_symbol_operator(cur_chr) )
			goto key_words_label; // идентификатор закончился
		throw cur_chr;
	}
key_words_label:
	if ( bufer == "program" )
		return lex_program;
	if ( bufer == "read" )
		return lex_read;
	if ( bufer == "write" )
		return lex_write;
	if ( bufer == "if" )
		return lex_if;
	if ( bufer == "else" )
		return lex_else;
	if ( bufer == "for" )
		return lex_for;
	if ( bufer == "while" )
		return lex_while;
	if ( bufer == "goto" )
		return lex_goto;
	if ( bufer == "continue" )
		return lex_continue;
	if ( bufer == "break" )
		return lex_break;
	if ( bufer == "true" )
		return lex_true;
	if ( bufer == "false" )
		return lex_false;
	if ( bufer == "int" )
		return lex_int;
	if ( bufer == "bool" )
		return lex_bool;
	if ( bufer == "string" )
		return lex_string;
	if ( bufer == "real" )
		return lex_real;
	if ( bufer == "struct" )
		return lex_struct;
	return lex_id;
}
lexema_type Interpretator::get_lexem() {
	bufer.erase();
	if (cur_chr == '\n')
		in_what_string_am_i += 1;
	while ( is_space(cur_chr) ) {
		if ( fscanf(file,"%c",&cur_chr) == EOF )
			return lex_NULL; // корректная программа всегда завершается lex_NULL
		// КОММЕНТАРИЙ
		if ( cur_chr == '#' )
			while (cur_chr != '\n')
				fscanf(file,"%c",&cur_chr);
		if (cur_chr == '\n')
			in_what_string_am_i += 1;
	}
	// слово, цифра, оператор, ...
	if ( is_letter(cur_chr) ) // если буква или _
		return read_word();
	else if ( is_symbol_operator(cur_chr) ) // если + = - * /
		return read_operator();
	else if ( is_digit(cur_chr) ) // если цифра
		return read_number();
	else if (cur_chr == '"') {
		/* константная строка */ /* "hello world" */
		do {
			fscanf(file,"%c",&cur_chr);
			if (cur_chr == '\n')
				throw "string cut by ENTER";
			bufer.push_back(cur_chr);
		} while ( cur_chr != '"' );
		cur_chr = ' ';
		bufer.resize(bufer.size()-1); // убрать закрывающую "
		return lex_string_const;
	} else
		throw cur_chr; // unknown symbol
}