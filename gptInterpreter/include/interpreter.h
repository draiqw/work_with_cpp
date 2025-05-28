#pragma once
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <stack>
using namespace std;


namespace interpretator_model {
	enum lexema_type {
		/**/
		lex_program,
		lex_if, lex_else,
		lex_for, lex_while,
		lex_goto, lex_continue, lex_break,
		lex_int, lex_bool, lex_real, lex_string,
		lex_id, lex_t_struct,
		/**/
		// begin == {
		// branch_open == (
		lex_begin, lex_end,
		lex_branch_open, lex_branch_close,
		lex_comma, lex_dot, lex_struct,
		lex_colon, lex_t_id, lex_t_label,
		/**/
		lex_jmp, lex_jF,
		lex_t_var, lex_t_const, lex_adress,
		lex_NULL,
		/**/
		lex_semicolon, // ; точка с запятой
		lex_read, lex_write,
		lex_plus, lex_minus,
		lex_plus_unar, lex_minus_unar,
		lex_plus_binar, lex_minus_binar,
		lex_plus_binar_dt, lex_minus_binar_dt,
		lex_mul, lex_div, lex_mod,
		lex_mul_dt, lex_div_dt,
		lex_G, lex_GE, lex_L, lex_LE, lex_E, lex_NE,
		lex_G_dt, lex_L_dt,
		lex_and, lex_or, lex_not,
		lex_mov, lex_add, lex_sub,
		lex_mov_dt, lex_add_dt, lex_sub_dt,
		/*s*/
		lex_true, lex_false, lex_int_const,
		lex_string_const, lex_real_const
	};
	enum value_type {
		val_int, val_lv_int,
		val_bool, val_lv_bool,
		val_real, val_lv_real,
		val_string, val_lv_string,
        val_not_val
	};
inline value_type to_rv(value_type val) {
		switch(val) {
		case val_lv_int: return val_int;
		case val_lv_bool: return val_bool;
		case val_lv_real: return val_real;
		case val_lv_string: return val_string;
		default: return val;
		}
	}
inline value_type to_lv(value_type val) {
		switch(val) {
		case val_int: return val_lv_int;
		case val_bool: return val_lv_bool;
		case val_real: return val_lv_real;
		case val_string: return val_lv_string;
		default: return val;
		}
	}
inline bool is_value_lv(value_type val) {
		switch(val) {
		case val_lv_int: return true;
		case val_lv_bool: return true;
		case val_lv_real: return true;
		case val_lv_string: return true;
		default: return false;
		}
	}
inline bool is_value_rv(value_type val) {
		switch(val) {
		case val_int: return true;
		case val_bool: return true;
		case val_real: return true;
		case val_string: return true;
		default: return false;
		}
	}
inline value_type lex_to_val(lexema_type lex) {
		switch(lex) {
		case lex_int: return val_int;
		case lex_bool: return val_bool;
		case lex_real: return val_real;
		case lex_string: return val_string;
		default: throw "CODE ERROR #5";
		}
	}
inline bool is_constant(lexema_type lex) {
		switch(lex) {
			case lex_int_const: case lex_string_const:
			case lex_real_const:
			case lex_true: case lex_false:
			return true;
			default: return false;
		}
	}
inline value_type val_result(value_type t1, value_type t2) {
		t1 = to_rv(t1);
		t2 = to_rv(t2);
		if (t1 == t2)
			return t1;
		if ((t1 == val_real && t2 == val_int) || (t1 == val_int && t2 == val_real))
			return val_real;
		throw "CODE ERROR #6";
	}
inline bool is_oper(lexema_type lex) {
		switch(lex) {
			case lex_semicolon: case lex_read: case lex_write:
			case lex_plus_unar: case lex_minus_unar:
			case lex_plus_binar: case lex_minus_binar:
			case lex_plus_binar_dt: case lex_minus_binar_dt:
			case lex_mul: case lex_div: case lex_mod:
			case lex_mul_dt: case lex_div_dt:
			case lex_G: case lex_GE: case lex_L:
			case lex_LE: case lex_E: case lex_NE:
			case lex_G_dt: case lex_L_dt:
			case lex_and: case lex_or: case lex_not:
			case lex_mov: case lex_add: case lex_sub:
			case lex_mov_dt: case lex_add_dt: case lex_sub_dt:
			return true;
			default: return false;
		}
	}
inline bool is_oneparametr(lexema_type lex) {
		switch(lex) {
		case lex_read: case lex_write:
		case lex_plus_unar: case lex_minus_unar:
		case lex_not:
			return true;
		default:
			return false;
		}
	}

	enum __gl_symbols_types {
		__not_a_symbol, __letter, __digit, __special, __spaces
	};
inline __gl_symbols_types __gl_symbols[256];
inline void make_alphabet() {
		int i;
		for (i = 0; i < 256; i++)
			__gl_symbols[i] = __not_a_symbol;
		for (i = 'a'; i <= 'z'; i++)
			__gl_symbols[i] = __letter;
		for (i = 'A'; i <= 'Z'; i++)
			__gl_symbols[i] = __letter;
		for (i = '0'; i <= '9'; i++)
			__gl_symbols[i] = __digit;
		__gl_symbols['_'] = __letter;
		__gl_symbols_types *a = __gl_symbols;
		a['+'] = a['-'] = a['{'] = a['}'] = a['('] = __special;
		a['*'] = a['/'] = a['%'] = a['<'] = a['>'] = __special;
		a['&'] = a['|'] = a[','] = a[';'] = a['!'] = __special;
		a[')'] = a['='] = a['.'] = a[':'] = __special;
		__gl_symbols[' '] = __spaces;
		__gl_symbols['\t'] = __spaces;
		__gl_symbols['\n'] = __spaces;
	}
inline int is_space(int i) {
		if (i >= 0 && i < 256)
			return __gl_symbols[i] == __spaces;
		else
			return 0;
	}
inline int is_letter(int i) {
		if (i >= 0 && i < 256)
			return __gl_symbols[i] == __letter;
		else
			return 0;
	}
inline int is_digit(int i) {
		if (i >= 0 && i < 256)
			return __gl_symbols[i] == __digit;
		else
			return 0;
	}
inline int is_symbol_operator(int i) {
		if (i >= 0 && i < 256)
			return __gl_symbols[i] == __special;
		else
			return 0;
	}
inline int is_unknown(int i) {
		if (i >= 0 && i < 256)
			return __gl_symbols[i] == __not_a_symbol;
		else
			return 0;
	}



    struct Lexema {
        // именно эти лексемы будут исходным кодом
		lexema_type type;
		int i;
        // если lex_t_var, то i для индекса в T_var
        // если lex_t_const, то i для индекса в T_const
        // для других lexema_type i используется иначе
		Lexema(){}
		Lexema(lexema_type t, int index){ type = t; i = index; }
	};
	struct Id {
		string name;
		// если встретили в тексте некий идентификатор, то надо найти его в T_id, за это отвечает name
        // во время объявления - создаём новую запись, без объявления - смотрим, был ли объявлен ранее
        // объявлен - привязать к T_var, не объявлен - прервать компиляцию
        //~ program {
            //~ int i, j;
            //~ // T_id ["i"] ["j"]
            //~ i = 5;
            //~ // T_id[0]
            //~ k = 4;
            //~ // не найдено в таблице - прервать парсинг
        //~ }
        Lexema lex;
		Id(){}
		Id(const string &s, Lexema l) : name(s), lex(l) {}
	};
    struct Field {
		// база для Var
        bool b;
		long i;
		double r;
		string s;
		Field() :s("") { b = false; i = 0; r = 0; }
		void operator += (const Field &f) { i += f.i; r += f.r; s += f.s; }
		void operator -= (const Field &f) { i -= f.i; r -= f.r; }
	};
    struct Var {
        // это есть переменная
		
        // тип переменной
        value_type type;
        
        // проинициализирована ли переменная, имеет ли значение
		bool is_valued;
        
        // значение переменной
		Field v;
		Var(){ is_valued = false; }
		Var( value_type t, bool is_v = false ) {
			type = t;
			is_valued = is_v;
		}
	};
}

using namespace interpretator_model;


class Interpretator {
private:
inline void next() { clex = get_lexem(); /*cout << lex_to_str(clex) << "\n";*/ }
inline void cond_th(bool b) { if (b) { throw clex; } }
	char *filename;
	FILE *file;
	lexema_type clex;
	int in_what_string_am_i;
	// открыть файл
public:
	Interpretator(const char *s) {
		filename = strdup(s);
		cur_chr = ' ';
		file = fopen(filename,"r");
		if (file == NULL)
			throw "unknown filename";
		in_what_string_am_i = 1;
	}
	~Interpretator() {
		free(filename);
		fclose(file);
	}
public:
	void analyze();
private:
	char cur_chr; // текущий символ
	string bufer; // текущее слово
	lexema_type get_lexem();//определить на какой мы лексеме
	lexema_type read_number();//функция чтения если на стоим на цифре
	lexema_type read_word();//функция чтения если на стоим на слове
	lexema_type read_operator();//функция чтения если на стоим на операторе
	const char* lex_to_str(lexema_type); //функция для удобного вывода
    
    vector <Var> T_var;
	vector <Var> T_const;
	vector <Id> T_id;

    //2 таблицы т.к. мы хотим избежать неравномерного хранения имен
    //программы в T_var
    //t_var непрерывная область памяти, а t_id разбросана.

    /*
        ГРАММАТИКА

        VALUE = VALUE_OR = VALUE | VALUE_OR
        VALUE_OR = VALUE_AND or VALUE_OR | VALUE_AND
        VALUE_AND = VALUE_LE and VALUE_AND
        VALUE_LE = VALUE_PLUS_MINUS >= VALUE_LE | VALUE_PLUS_MINUS
        VALUE_PLUS_MINUS = VALUE_MUL_DIV + VALUE_PLUS_MINUS | VALUE_MUL_DIV
        VALUE_MUL_DIV = VALUE_NOT * VALUE_MUL_DIV| VALUE_NOT 
        VALUE_NOT = not SIMPLE_VALUE | SIMPLE_VALUE
        SIMPLE_VALUE = 1 2 3 4 "stroka" | a b c i j k | (VALUE)
    */
    
    
private:
    void semantic_analis();//функция запускающая лексический, синтаксический и семантический анализ
	void DECLAR();//- функция объявления переменных: считывает 
		          //int i,j,k
		          //string s;
		          //...
		          //пока clex у нас lex_type работает эта ф-ия,
                  //берем следующее слово, должно быть имя идентификатора,
                  //иначе ошибка, а далее make_new_var. Следующее слово и далее 			присваивание в цикле.
	void COMMANDS();//функция пока не конец -> COMMAND()
	void COMMAND();//проход и сравнения лексемы со стандартными функциями,
                   //иначе - ошибка
	void READ_WRITE();//стандартная функция чтения/записи

	value_type VALUE();//стандартные функции грамматики
	value_type VALUE_OR();
	value_type VALUE_AND();
	value_type VALUE_LE();
	value_type VALUE_PLUS_MINUS();
	value_type VALUE_MUL_DIV();
	value_type VALUE_NOT();
	value_type VALUE_ID();
	value_type IDENTIF();

    Lexema make_lexema_for_poliz(lexema_type lex, value_type v1 = val_not_val, value_type v2 = val_not_val);//функция проверки корректности использования того
	//или иного типа в выражении, в случае правильности вернет константу - лексему операции которую необходимо провести с операндами, далее запушим это в стэк

	void MOVE(value_type);//функция присвоения (смотрит правильность аргументов)
	void IF();//функция if (смотрит правильность аргументов)
	void WHILE();//функция while (смотрит правильность аргументов)
	void FOR();//функция for (смотрит правильность аргументов)
	void CONTINUE_BREAK();//функция реализующая continue и break
                          //(смотрит правильность аргументов)
	void GOTO();//функция goto (смотрит правильность аргументов)
    

private:
	Lexema find_in_T_const(lexema_type, value_type &);//поиск по таблице констант
	Lexema find_in_T_const_string();//поиск строки в таблице констант.
	Lexema find_in_T_const_real();//поиск real в таблице констант
	Lexema find_in_T_const_int();//поиск int в таблице констант
	Lexema find_in_T_const_bool();//поиск bool в таблице констант
	Lexema make_new_var(value_type t);//создать новую переменную,
                                      //если такая уже есть - ошибка
	Lexema find_in_T_var(value_type &t); //поиск по таблице, если не нашел - 
                                         //ошибка, достает лексему из ТИД
                                         //определяет ее тип

//
	vector <Lexema> poliz;//вектор содержащий лексемы, хранящий полиз
	stack <Lexema> continue_break_counter;//сюда пушим каждый раз когда встретили, в начале каждого цикла пушится пустышка, чтобы различать вложенность
	vector <Id> goto_counter;//массив меток которых встретили с goto, для дальнейшей проверки правильности и заролнения адресов в полиз
	vector <Id> T_label;//таблица, аналогичная по сути T_id, хранит метки
	void init_goto_and_label();//функция для заполнения пустых адресов jmp в полизе 
	void make_new_label();//функция инициализации метки

	//интерпретация
	void oneparam_oper_execution(stack<Lexema> &st, Lexema lex);//обрабатывает команды из полиз с 1 операндом
	void twoparam_oper_execution(stack<Lexema> &st, Lexema lex);//обрабатывает команды из полиз с 2 операторами
	void execute();//последовательный вызов функций исполнения
};

// if ( a and b + (c + d) )

//обрабатываем лексемы слов, цифр, букв