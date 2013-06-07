/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
*/
/* First off, code is include which follows the "include" declaration
** in the input file. */
#include <stdio.h>
// 41 "parser.lemon"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_phalcon.h"
#include "phalcon.h"

#include "parser.h"
#include "scanner.h"
#include "volt.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"

static zval *phvolt_ret_literal_zval(int type, phvolt_parser_token *T, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", type);
	if (T) {
		add_assoc_stringl(ret, "value", T->token, T->token_len, 0);
		efree(T);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_if_statement(zval *expr, zval *true_statements, zval *false_statements, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_IF);
	add_assoc_zval(ret, "expr", expr);
	add_assoc_zval(ret, "true_statements", true_statements);

	if (false_statements) {
		add_assoc_zval(ret, "false_statements", false_statements);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_elseif_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_ELSEIF);
	add_assoc_zval(ret, "expr", expr);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_elsefor_statement(phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_ELSEFOR);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_for_statement(phvolt_parser_token *variable, phvolt_parser_token *key, zval *expr, zval *if_expr, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_FOR);

	add_assoc_stringl(ret, "variable", variable->token, variable->token_len, 0);
	efree(variable);

	if (key) {
		add_assoc_stringl(ret, "key", key->token, key->token_len, 0);
		efree(key);
	}

	add_assoc_zval(ret, "expr", expr);

	if (if_expr) {
		add_assoc_zval(ret, "if_expr", if_expr);
	}

	add_assoc_zval(ret, "block_statements", block_statements);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_cache_statement(zval *expr, phvolt_parser_token *lifetime, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, "type", PHVOLT_T_CACHE);
	add_assoc_zval(ret, "expr", expr);

	if (lifetime) {
		add_assoc_stringl(ret, "lifetime", lifetime->token, lifetime->token_len, 0);
		efree(lifetime);
	}
	add_assoc_zval(ret, "block_statements", block_statements);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_set_statement(phvolt_parser_token *variable, zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 5);
	add_assoc_long(ret, "type", PHVOLT_T_SET);

	add_assoc_stringl(ret, "variable", variable->token, variable->token_len, 0);
	efree(variable);

	add_assoc_zval(ret, "expr", expr);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_echo_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 4);
	add_assoc_long(ret, "type", PHVOLT_T_ECHO);
	add_assoc_zval(ret, "expr", expr);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_block_statement(phvolt_parser_token *name, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);

	add_assoc_long(ret, "type", PHVOLT_T_BLOCK);

	add_assoc_stringl(ret, "name", name->token, name->token_len, 0);
	efree(name);

	if (block_statements) {
		add_assoc_zval(ret, "block_statements", block_statements);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_macro_statement(phvolt_parser_token *macro_name, zval *parameters, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_MACRO);

	add_assoc_stringl(ret, "name", macro_name->token, macro_name->token_len, 0);
	efree(macro_name);

	if (parameters) {
		add_assoc_zval(ret, "parameters", parameters);
	}

	if (block_statements) {
		add_assoc_zval(ret, "block_statements", block_statements);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_macro_parameter(phvolt_parser_token *variable, zval *default_value, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 5);

	add_assoc_stringl(ret, "variable", variable->token, variable->token_len, 0);
	efree(variable);

	if (default_value) {
		add_assoc_zval(ret, "default", default_value);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_extends_statement(phvolt_parser_token *P, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 4);

	add_assoc_long(ret, "type", PHVOLT_T_EXTENDS);
	add_assoc_stringl(ret, "path", P->token, P->token_len, 0);
	efree(P);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_include_statement(zval *path, zval *params, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 4);

	add_assoc_long(ret, "type", PHVOLT_T_INCLUDE);

	add_assoc_zval(ret, "path", path);
	if (params) {
		add_assoc_zval(ret, "params", params);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_do_statement(zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 4);

	add_assoc_long(ret, "type", PHVOLT_T_DO);

	add_assoc_zval(ret, "expr", expr);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_autoescape_statement(int enable, zval *block_statements, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 5);

	add_assoc_long(ret, "type", PHVOLT_T_AUTOESCAPE);
	add_assoc_long(ret, "enable", enable);
	add_assoc_zval(ret, "block_statements", block_statements);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_empty_statement(phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 3);
	add_assoc_long(ret, "type", PHVOLT_T_EMPTY);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_break_statement(phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 3);
	add_assoc_long(ret, "type", PHVOLT_T_BREAK);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_continue_statement(phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init_size(ret, 3);
	add_assoc_long(ret, "type", PHVOLT_T_CONTINUE);

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_zval_list(zval *list_left, zval *right_list)
{

	zval *ret;
	HashPosition pos;
	HashTable *list;

	MAKE_STD_ZVAL(ret);
	array_init(ret);

	if (list_left) {

		list = Z_ARRVAL_P(list_left);
		if (zend_hash_index_exists(list, 0)) {
			zend_hash_internal_pointer_reset_ex(list, &pos);
			for (;; zend_hash_move_forward_ex(list, &pos)) {

				zval ** item;

				if (zend_hash_get_current_data_ex(list, (void**) &item, &pos) == FAILURE) {
					break;
				}

				Z_ADDREF_PP(item);
				add_next_index_zval(ret, *item);

			}
			zval_ptr_dtor(&list_left);
		} else {
			add_next_index_zval(ret, list_left);
		}
	}

	add_next_index_zval(ret, right_list);

	return ret;
}

static zval *phvolt_ret_named_item(phvolt_parser_token *name, zval *expr, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_zval(ret, "expr", expr);
	if (name != NULL) {
		add_assoc_stringl(ret, "name", name->token, name->token_len, 0);
		efree(name);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_expr(int type, zval *left, zval *right, zval *ternary, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", type);

	if (ternary) {
		add_assoc_zval(ret, "ternary", ternary);
	}

	if (left) {
		add_assoc_zval(ret, "left", left);
	}

	if (right) {
		add_assoc_zval(ret, "right", right);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_slice(zval *left, zval *start, zval *end, phvolt_scanner_state *state)
{
	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_SLICE);
	add_assoc_zval(ret, "left", left);

	if (start != NULL) {
		add_assoc_zval(ret, "start", start);
	}

	if (end != NULL) {
		add_assoc_zval(ret, "end", end);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}

static zval *phvolt_ret_func_call(zval *expr, zval *arguments, phvolt_scanner_state *state)
{

	zval *ret;

	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "type", PHVOLT_T_FCALL);
	add_assoc_zval(ret, "name", expr);

	if (arguments) {
		add_assoc_zval(ret, "arguments", arguments);
	}

	Z_ADDREF_P(state->active_file);
	add_assoc_zval(ret, "file", state->active_file);
	add_assoc_long(ret, "line", state->active_line);

	return ret;
}


// 509 "parser.c"
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    KKCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    KKNOCODE           is a number of type KKCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    KKFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    KKACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    phvolt_KTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    KKMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is phvolt_KTOKENTYPE.  The entry in the union
**                       for base tokens is called "kk0".
**    KKSTACKDEPTH       is the maximum depth of the parser's stack.
**    phvolt_ARG_SDECL     A static variable declaration for the %extra_argument
**    phvolt_ARG_PDECL     A parameter declaration for the %extra_argument
**    phvolt_ARG_STORE     Code to store %extra_argument into kkpParser
**    phvolt_ARG_FETCH     Code to extract %extra_argument from kkpParser
**    KKNSTATE           the combined number of states.
**    KKNRULE            the number of rules in the grammar
**    KKERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define KKCODETYPE unsigned char
#define KKNOCODE 99
#define KKACTIONTYPE unsigned short int
#define phvolt_KTOKENTYPE phvolt_parser_token*
typedef union {
  phvolt_KTOKENTYPE kk0;
  zval* kk92;
  int kk197;
} KKMINORTYPE;
#define KKSTACKDEPTH 100
#define phvolt_ARG_SDECL phvolt_parser_status *status;
#define phvolt_ARG_PDECL ,phvolt_parser_status *status
#define phvolt_ARG_FETCH phvolt_parser_status *status = kkpParser->status
#define phvolt_ARG_STORE kkpParser->status = status
#define KKNSTATE 273
#define KKNRULE 114
#define KKERRORSYMBOL 66
#define KKERRSYMDT kk197
#define KK_NO_ACTION      (KKNSTATE+KKNRULE+2)
#define KK_ACCEPT_ACTION  (KKNSTATE+KKNRULE+1)
#define KK_ERROR_ACTION   (KKNSTATE+KKNRULE)

/* Next are that tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < KKNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   KKNSTATE <= N < KKNSTATE+KKNRULE   Reduce by rule N-KKNSTATE.
**
**   N == KKNSTATE+KKNRULE              A syntax error has occurred.
**
**   N == KKNSTATE+KKNRULE+1            The parser accepts its input.
**
**   N == KKNSTATE+KKNRULE+2            No such action.  Denotes unused
**                                      slots in the kk_action[] table.
**
** The action table is constructed as a single large table named kk_action[].
** Given state S and lookahead X, the action is computed as
**
**      kk_action[ kk_shift_ofst[S] + X ]
**
** If the index value kk_shift_ofst[S]+X is out of range or if the value
** kk_lookahead[kk_shift_ofst[S]+X] is not equal to X or if kk_shift_ofst[S]
** is equal to KK_SHIFT_USE_DFLT, it means that the action is not in the table
** and that kk_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the kk_reduce_ofst[] array is used in place of
** the kk_shift_ofst[] array and KK_REDUCE_USE_DFLT is used in place of
** KK_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  kk_action[]        A single table containing all actions.
**  kk_lookahead[]     A table containing the lookahead for each entry in
**                     kk_action.  Used to detect hash collisions.
**  kk_shift_ofst[]    For each state, the offset into kk_action for
**                     shifting terminals.
**  kk_reduce_ofst[]   For each state, the offset into kk_action for
**                     shifting non-terminals after a reduce.
**  kk_default[]       Default action for each state.
*/
static KKACTIONTYPE kk_action[] = {
 /*     0 */    80,   88,  273,   85,   58,   50,   52,   62,   60,   64,
 /*    10 */    70,   72,   74,   76,   66,   68,   46,   44,   48,   41,
 /*    20 */    38,   54,   56,   82,   92,   78,  250,  154,   80,   88,
 /*    30 */    28,   85,   58,   50,   52,   62,   60,   64,   70,   72,
 /*    40 */    74,   76,   66,   68,   46,   44,   48,   41,   38,   54,
 /*    50 */    56,   82,   92,   78,   30,  183,   46,   44,   48,   41,
 /*    60 */    38,   54,   56,   82,   92,   78,  104,   23,  222,   82,
 /*    70 */    92,   78,   80,   88,  109,   85,   58,   50,   52,   62,
 /*    80 */    60,   64,   70,   72,   74,   76,   66,   68,   46,   44,
 /*    90 */    48,   41,   38,   54,   56,   82,   92,   78,   34,  193,
 /*   100 */    80,   88,   83,   85,   58,   50,   52,   62,   60,   64,
 /*   110 */    70,   72,   74,   76,   66,   68,   46,   44,   48,   41,
 /*   120 */    38,   54,   56,   82,   92,   78,  194,   24,  107,  103,
 /*   130 */    80,   88,  128,   85,   58,   50,   52,   62,   60,   64,
 /*   140 */    70,   72,   74,   76,   66,   68,   46,   44,   48,   41,
 /*   150 */    38,   54,   56,   82,   92,   78,  266,  261,   80,   88,
 /*   160 */   129,   85,   58,   50,   52,   62,   60,   64,   70,   72,
 /*   170 */    74,   76,   66,   68,   46,   44,   48,   41,   38,   54,
 /*   180 */    56,   82,   92,   78,  123,   37,   80,   88,  149,   85,
 /*   190 */    58,   50,   52,   62,   60,   64,   70,   72,   74,   76,
 /*   200 */    66,   68,   46,   44,   48,   41,   38,   54,   56,   82,
 /*   210 */    92,   78,   80,   88,   90,   85,   58,   50,   52,   62,
 /*   220 */    60,   64,   70,   72,   74,   76,   66,   68,   46,   44,
 /*   230 */    48,   41,   38,   54,   56,   82,   92,   78,   41,   38,
 /*   240 */    54,   56,   82,   92,   78,  257,  152,   36,   87,  231,
 /*   250 */   249,  367,   80,   88,  109,   85,   58,   50,   52,   62,
 /*   260 */    60,   64,   70,   72,   74,   76,   66,   68,   46,   44,
 /*   270 */    48,   41,   38,   54,   56,   82,   92,   78,  243,  244,
 /*   280 */   245,  246,  247,  248,   56,   82,   92,   78,  368,  147,
 /*   290 */    98,  201,  216,  133,   80,   88,  109,   85,   58,   50,
 /*   300 */    52,   62,   60,   64,   70,   72,   74,   76,   66,   68,
 /*   310 */    46,   44,   48,   41,   38,   54,   56,   82,   92,   78,
 /*   320 */   135,  163,   80,   88,  158,   85,   58,   50,   52,   62,
 /*   330 */    60,   64,   70,   72,   74,   76,   66,   68,   46,   44,
 /*   340 */    48,   41,   38,   54,   56,   82,   92,   78,  136,  196,
 /*   350 */    80,   88,  151,   85,   58,   50,   52,   62,   60,   64,
 /*   360 */    70,   72,   74,   76,   66,   68,   46,   44,   48,   41,
 /*   370 */    38,   54,   56,   82,   92,   78,  367,  199,   80,   88,
 /*   380 */   160,   85,   58,   50,   52,   62,   60,   64,   70,   72,
 /*   390 */    74,   76,   66,   68,   46,   44,   48,   41,   38,   54,
 /*   400 */    56,   82,   92,   78,  388,    1,    2,  215,    4,    5,
 /*   410 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   420 */    16,   17,   18,   19,   20,  213,  240,  368,  161,   80,
 /*   430 */    88,  167,   85,   58,   50,   52,   62,   60,   64,   70,
 /*   440 */    72,   74,   76,   66,   68,   46,   44,   48,   41,   38,
 /*   450 */    54,   56,   82,   92,   78,  165,  252,   80,   88,  166,
 /*   460 */    85,   58,   50,   52,   62,   60,   64,   70,   72,   74,
 /*   470 */    76,   66,   68,   46,   44,   48,   41,   38,   54,   56,
 /*   480 */    82,   92,   78,  168,  268,   80,   88,  172,   85,   58,
 /*   490 */    50,   52,   62,   60,   64,   70,   72,   74,   76,   66,
 /*   500 */    68,   46,   44,   48,   41,   38,   54,   56,   82,   92,
 /*   510 */    78,   88,  175,   85,   58,   50,   52,   62,   60,   64,
 /*   520 */    70,   72,   74,   76,   66,   68,   46,   44,   48,   41,
 /*   530 */    38,   54,   56,   82,   92,   78,   85,   58,   50,   52,
 /*   540 */    62,   60,   64,   70,   72,   74,   76,   66,   68,   46,
 /*   550 */    44,   48,   41,   38,   54,   56,   82,   92,   78,   50,
 /*   560 */    52,   62,   60,   64,   70,   72,   74,   76,   66,   68,
 /*   570 */    46,   44,   48,   41,   38,   54,   56,   82,   92,   78,
 /*   580 */    25,  215,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   590 */    12,   13,   14,   15,   16,   17,   18,   19,   20,  189,
 /*   600 */    31,  215,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   610 */    12,   13,   14,   15,   16,   17,   18,   19,   20,   62,
 /*   620 */    60,   64,   70,   72,   74,   76,   66,   68,   46,   44,
 /*   630 */    48,   41,   38,   54,   56,   82,   92,   78,  155,  215,
 /*   640 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   650 */    14,   15,   16,   17,   18,   19,   20,  169,  215,    4,
 /*   660 */     5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*   670 */    15,   16,   17,   18,   19,   20,  177,  215,    4,    5,
 /*   680 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   690 */    16,   17,   18,   19,   20,  184,  215,    4,    5,    6,
 /*   700 */     7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
 /*   710 */    17,   18,   19,   20,  203,  215,    4,    5,    6,    7,
 /*   720 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   730 */    18,   19,   20,  218,  215,    4,    5,    6,    7,    8,
 /*   740 */     9,   10,   11,   12,   13,   14,   15,   16,   17,   18,
 /*   750 */    19,   20,  224,  215,    4,    5,    6,    7,    8,    9,
 /*   760 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*   770 */    20,  234,  215,    4,    5,    6,    7,    8,    9,   10,
 /*   780 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*   790 */   253,  215,    4,    5,    6,    7,    8,    9,   10,   11,
 /*   800 */    12,   13,   14,   15,   16,   17,   18,   19,   20,  262,
 /*   810 */   215,    4,    5,    6,    7,    8,    9,   10,   11,   12,
 /*   820 */    13,   14,   15,   16,   17,   18,   19,   20,  269,  215,
 /*   830 */     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   840 */    14,   15,   16,   17,   18,   19,   20,    3,    4,    5,
 /*   850 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   860 */    16,   17,   18,   19,   20,   22,  173,   27,   29,   35,
 /*   870 */   148,  150,  176,  180,  159,  187,  164,   22,  173,   33,
 /*   880 */    21,   35,  148,  150,  190,  120,  159,  174,  164,  181,
 /*   890 */   202,  188,  191,   93,  197,  200,  206,  207,  209,  174,
 /*   900 */   109,  181,  121,  188,  191,  240,  197,  200,  239,  207,
 /*   910 */   209,   22,  173,  274,   21,   35,  148,  150,   93,  157,
 /*   920 */   159,  208,  164,  131,  119,  109,  118,  126,  211,   86,
 /*   930 */   130,  210,   40,  174,  132,  181,  109,  188,  191,  109,
 /*   940 */   197,  200,  214,  207,  209,   22,  173,  217,   26,   35,
 /*   950 */   148,  150,  134,  100,  159,  242,  164,  102,  171,  117,
 /*   960 */   109,  131,  211,  146,  221,  238,   43,  174,  130,  181,
 /*   970 */   109,  188,  191,  109,  197,  200,  214,  207,  209,   22,
 /*   980 */   173,  223,   32,   35,  148,  150,  100,   45,  159,  227,
 /*   990 */   164,   47,  105,  109,  109,  230,  211,  241,  109,  233,
 /*  1000 */    49,  174,  179,  181,  232,  188,  191,  109,  197,  200,
 /*  1010 */   214,  207,  209,   22,  173,  237,   51,   35,  148,  150,
 /*  1020 */   256,   53,  159,  109,  164,   55,   57,   59,  109,   61,
 /*  1030 */   211,  258,  109,  109,  109,  174,  109,  181,  186,  188,
 /*  1040 */   191,  259,  197,  200,  214,  207,  209,   22,  173,  265,
 /*  1050 */    95,   35,  148,  150,  272,   63,  159,  109,  164,   65,
 /*  1060 */    67,   69,  109,   71,  214,   73,  109,  109,  109,  174,
 /*  1070 */   109,  181,  109,  188,  191,  214,  197,  200,  205,  207,
 /*  1080 */   209,   22,  173,  214,  156,   35,  148,  150,  214,   75,
 /*  1090 */   159,   77,  164,  251,   79,   81,  109,   84,  109,   89,
 /*  1100 */   109,  109,  109,  174,  109,  181,  109,  188,  191,  214,
 /*  1110 */   197,  200,  220,  207,  209,   22,  173,  214,  170,   35,
 /*  1120 */   148,  150,   91,   95,  159,  214,  164,   97,  108,  109,
 /*  1130 */   109,  124,  211,  214,  109,  109,  143,  174,  109,  181,
 /*  1140 */   226,  188,  191,  109,  197,  200,  214,  207,  209,   22,
 /*  1150 */   173,  214,  228,   35,  148,  150,  145,  153,  159,  214,
 /*  1160 */   164,  162,  182,  109,  109,  214,  211,  192,  109,  109,
 /*  1170 */   195,  174,  229,  181,  109,  188,  191,  109,  197,  200,
 /*  1180 */   214,  207,  209,   22,  173,  214,  178,   35,  148,  150,
 /*  1190 */   198,  212,  159,  214,  164,  260,  236,  109,  109,  267,
 /*  1200 */   211,  214,  109,  214,  214,  174,  109,  181,  214,  188,
 /*  1210 */   191,  214,  197,  200,  214,  207,  209,   22,  173,  214,
 /*  1220 */   185,   35,  148,  150,  214,  255,  159,  214,  164,  214,
 /*  1230 */   214,  214,  214,  214,  211,  214,  214,  214,  214,  174,
 /*  1240 */   214,  181,  214,  188,  191,  214,  197,  200,  214,  207,
 /*  1250 */   209,   22,  173,  214,  204,   35,  148,  150,  214,  264,
 /*  1260 */   159,  214,  164,  214,  214,  214,  214,  214,  211,  214,
 /*  1270 */   214,  214,  214,  174,  214,  181,  214,  188,  191,  214,
 /*  1280 */   197,  200,  214,  207,  209,   22,  173,  214,  214,   35,
 /*  1290 */   148,  150,  219,  271,  159,  214,  164,  214,  214,  214,
 /*  1300 */   214,  214,  211,  214,  214,  214,  214,  174,  214,  181,
 /*  1310 */   214,  188,  191,  225,  197,  200,  214,  207,  209,   22,
 /*  1320 */   173,  214,  214,   35,  148,  150,  214,  214,  159,  214,
 /*  1330 */   164,  214,  214,  214,  214,  214,  214,  214,  214,  214,
 /*  1340 */   211,  174,  214,  181,  214,  188,  191,  214,  197,  200,
 /*  1350 */    99,  207,  209,  235,  214,  214,   99,  214,  214,  214,
 /*  1360 */   214,  211,  214,  214,  144,  214,   42,   39,  214,  142,
 /*  1370 */    94,   96,   42,   39,  214,  214,   94,   96,  214,  214,
 /*  1380 */   110,  214,  214,   99,  214,  214,  110,  111,  112,  113,
 /*  1390 */   114,  115,  116,  111,  112,  113,  114,  115,  116,   42,
 /*  1400 */    39,  211,  214,  139,   96,  214,  214,  214,  214,  127,
 /*  1410 */    99,  254,  214,  110,  214,  214,  214,  214,  214,  214,
 /*  1420 */   111,  112,  113,  114,  115,  116,   42,   39,  214,  214,
 /*  1430 */    94,   96,  214,  214,  214,  214,  214,   99,  214,  214,
 /*  1440 */   138,  141,  214,  214,  214,  214,  214,  137,  112,  113,
 /*  1450 */   114,  115,  116,   42,   39,  214,  214,   94,   96,  211,
 /*  1460 */   214,  214,  214,  214,   99,  214,  214,  110,  214,  214,
 /*  1470 */   214,  214,  125,  214,  111,  122,  113,  114,  115,  116,
 /*  1480 */    42,   39,  214,  214,   94,   96,  214,  214,  214,  214,
 /*  1490 */   214,   99,  214,  214,  110,  214,  214,  214,  214,  214,
 /*  1500 */   214,  111,  106,  113,  114,  115,  116,   42,   39,  214,
 /*  1510 */   214,   94,   96,  214,  214,  214,  214,  214,   99,  214,
 /*  1520 */   214,  110,  214,  101,  214,  214,  214,  214,  111,  112,
 /*  1530 */   113,  114,  115,  116,   42,   39,  214,  214,   94,   96,
 /*  1540 */   214,  214,  214,  214,  214,   99,  214,  214,  110,  140,
 /*  1550 */   214,  214,  214,  214,  214,  111,  112,  113,  114,  115,
 /*  1560 */   116,   42,   39,  214,  214,   94,   96,  214,  263,  214,
 /*  1570 */   270,  214,   99,  214,  214,  110,  214,  214,  214,  214,
 /*  1580 */   214,  214,  111,  106,  113,  114,  115,  116,   42,   39,
 /*  1590 */   214,  214,   94,   96,  214,  214,  214,  214,  214,  214,
 /*  1600 */   214,  214,  110,  214,  214,  214,  214,  214,  214,  111,
 /*  1610 */   122,  113,  114,  115,  116,  214,  211,  214,  211,  214,
 /*  1620 */   214,  214,  214,  214,  214,  214,  214,  214,  214,  214,
 /*  1630 */   214,  214,  214,
};
static KKCODETYPE kk_lookahead[] = {
 /*     0 */     3,    4,    0,    6,    7,    8,    9,   10,   11,   12,
 /*    10 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*    20 */    23,   24,   25,   26,   27,   28,   29,   30,    3,    4,
 /*    30 */    30,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*    40 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*    50 */    25,   26,   27,   28,   30,   30,   19,   20,   21,   22,
 /*    60 */    23,   24,   25,   26,   27,   28,    2,   88,   43,   26,
 /*    70 */    27,   28,    3,    4,   95,    6,    7,    8,    9,   10,
 /*    80 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*    90 */    21,   22,   23,   24,   25,   26,   27,   28,   30,   30,
 /*   100 */     3,    4,    3,    6,    7,    8,    9,   10,   11,   12,
 /*   110 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   120 */    23,   24,   25,   26,   27,   28,   57,   30,    5,   65,
 /*   130 */     3,    4,   93,    6,    7,    8,    9,   10,   11,   12,
 /*   140 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   150 */    23,   24,   25,   26,   27,   28,   29,   30,    3,    4,
 /*   160 */    65,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*   170 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*   180 */    25,   26,   27,   28,    5,   30,    3,    4,   30,    6,
 /*   190 */     7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
 /*   200 */    17,   18,   19,   20,   21,   22,   23,   24,   25,   26,
 /*   210 */    27,   28,    3,    4,    5,    6,    7,    8,    9,   10,
 /*   220 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*   230 */    21,   22,   23,   24,   25,   26,   27,   28,   22,   23,
 /*   240 */    24,   25,   26,   27,   28,    2,    3,   88,   65,   89,
 /*   250 */    90,   65,    3,    4,   95,    6,    7,    8,    9,   10,
 /*   260 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*   270 */    21,   22,   23,   24,   25,   26,   27,   28,   43,   44,
 /*   280 */    45,   46,   47,   48,   25,   26,   27,   28,   65,   88,
 /*   290 */    41,   47,   48,    5,    3,    4,   95,    6,    7,    8,
 /*   300 */     9,   10,   11,   12,   13,   14,   15,   16,   17,   18,
 /*   310 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*   320 */    93,   30,    3,    4,   30,    6,    7,    8,    9,   10,
 /*   330 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*   340 */    21,   22,   23,   24,   25,   26,   27,   28,   65,   30,
 /*   350 */     3,    4,   36,    6,    7,    8,    9,   10,   11,   12,
 /*   360 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   370 */    23,   24,   25,   26,   27,   28,    5,   30,    3,    4,
 /*   380 */    36,    6,    7,    8,    9,   10,   11,   12,   13,   14,
 /*   390 */    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,
 /*   400 */    25,   26,   27,   28,   67,   68,   69,   70,   71,   72,
 /*   410 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   420 */    83,   84,   85,   86,   87,   50,   36,    5,   39,    3,
 /*   430 */     4,   41,    6,    7,    8,    9,   10,   11,   12,   13,
 /*   440 */    14,   15,   16,   17,   18,   19,   20,   21,   22,   23,
 /*   450 */    24,   25,   26,   27,   28,   36,   30,    3,    4,   27,
 /*   460 */     6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
 /*   470 */    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,
 /*   480 */    26,   27,   28,   30,   30,    3,    4,   30,    6,    7,
 /*   490 */     8,    9,   10,   11,   12,   13,   14,   15,   16,   17,
 /*   500 */    18,   19,   20,   21,   22,   23,   24,   25,   26,   27,
 /*   510 */    28,    4,   36,    6,    7,    8,    9,   10,   11,   12,
 /*   520 */    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,
 /*   530 */    23,   24,   25,   26,   27,   28,    6,    7,    8,    9,
 /*   540 */    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
 /*   550 */    20,   21,   22,   23,   24,   25,   26,   27,   28,    8,
 /*   560 */     9,   10,   11,   12,   13,   14,   15,   16,   17,   18,
 /*   570 */    19,   20,   21,   22,   23,   24,   25,   26,   27,   28,
 /*   580 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   590 */    79,   80,   81,   82,   83,   84,   85,   86,   87,   44,
 /*   600 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   610 */    79,   80,   81,   82,   83,   84,   85,   86,   87,   10,
 /*   620 */    11,   12,   13,   14,   15,   16,   17,   18,   19,   20,
 /*   630 */    21,   22,   23,   24,   25,   26,   27,   28,   69,   70,
 /*   640 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*   650 */    81,   82,   83,   84,   85,   86,   87,   69,   70,   71,
 /*   660 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   670 */    82,   83,   84,   85,   86,   87,   69,   70,   71,   72,
 /*   680 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   690 */    83,   84,   85,   86,   87,   69,   70,   71,   72,   73,
 /*   700 */    74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   710 */    84,   85,   86,   87,   69,   70,   71,   72,   73,   74,
 /*   720 */    75,   76,   77,   78,   79,   80,   81,   82,   83,   84,
 /*   730 */    85,   86,   87,   69,   70,   71,   72,   73,   74,   75,
 /*   740 */    76,   77,   78,   79,   80,   81,   82,   83,   84,   85,
 /*   750 */    86,   87,   69,   70,   71,   72,   73,   74,   75,   76,
 /*   760 */    77,   78,   79,   80,   81,   82,   83,   84,   85,   86,
 /*   770 */    87,   69,   70,   71,   72,   73,   74,   75,   76,   77,
 /*   780 */    78,   79,   80,   81,   82,   83,   84,   85,   86,   87,
 /*   790 */    69,   70,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   800 */    79,   80,   81,   82,   83,   84,   85,   86,   87,   69,
 /*   810 */    70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*   820 */    80,   81,   82,   83,   84,   85,   86,   87,   69,   70,
 /*   830 */    71,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*   840 */    81,   82,   83,   84,   85,   86,   87,   70,   71,   72,
 /*   850 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   860 */    83,   84,   85,   86,   87,   29,   30,   31,   32,   33,
 /*   870 */    34,   35,   30,   30,   38,   30,   40,   29,   30,   31,
 /*   880 */     1,   33,   34,   35,   30,    2,   38,   51,   40,   53,
 /*   890 */    30,   55,   56,   88,   58,   59,   30,   61,   62,   51,
 /*   900 */    95,   53,   97,   55,   56,   36,   58,   59,   90,   61,
 /*   910 */    62,   29,   30,    0,    1,   33,   34,   35,   88,   37,
 /*   920 */    38,   30,   40,   36,   41,   95,   96,   97,   49,   88,
 /*   930 */    43,   30,   88,   51,   93,   53,   95,   55,   56,   95,
 /*   940 */    58,   59,   63,   61,   62,   29,   30,   30,    1,   33,
 /*   950 */    34,   35,   65,   88,   38,   91,   40,   92,   42,   94,
 /*   960 */    95,   36,   49,   88,   30,    2,   88,   51,   43,   53,
 /*   970 */    95,   55,   56,   95,   58,   59,   63,   61,   62,   29,
 /*   980 */    30,   30,    1,   33,   34,   35,   88,   88,   38,   30,
 /*   990 */    40,   88,   94,   95,   95,   30,   49,   39,   95,   30,
 /*  1000 */    88,   51,   52,   53,   41,   55,   56,   95,   58,   59,
 /*  1010 */    63,   61,   62,   29,   30,   30,   88,   33,   34,   35,
 /*  1020 */    30,   88,   38,   95,   40,   88,   88,   88,   95,   88,
 /*  1030 */    49,   36,   95,   95,   95,   51,   95,   53,   54,   55,
 /*  1040 */    56,    3,   58,   59,   63,   61,   62,   29,   30,   30,
 /*  1050 */    88,   33,   34,   35,   30,   88,   38,   95,   40,   88,
 /*  1060 */    88,   88,   95,   88,   98,   88,   95,   95,   95,   51,
 /*  1070 */    95,   53,   95,   55,   56,   98,   58,   59,   60,   61,
 /*  1080 */    62,   29,   30,   98,    1,   33,   34,   35,   98,   88,
 /*  1090 */    38,   88,   40,   88,   88,   88,   95,   88,   95,   88,
 /*  1100 */    95,   95,   95,   51,   95,   53,   95,   55,   56,   98,
 /*  1110 */    58,   59,   60,   61,   62,   29,   30,   98,    1,   33,
 /*  1120 */    34,   35,   88,   88,   38,   98,   40,   88,   88,   95,
 /*  1130 */    95,   88,   49,   98,   95,   95,   88,   51,   95,   53,
 /*  1140 */    54,   55,   56,   95,   58,   59,   63,   61,   62,   29,
 /*  1150 */    30,   98,    1,   33,   34,   35,   88,   88,   38,   98,
 /*  1160 */    40,   88,   88,   95,   95,   98,   49,   88,   95,   95,
 /*  1170 */    88,   51,   52,   53,   95,   55,   56,   95,   58,   59,
 /*  1180 */    63,   61,   62,   29,   30,   98,    1,   33,   34,   35,
 /*  1190 */    88,   88,   38,   98,   40,   88,   42,   95,   95,   88,
 /*  1200 */    49,   98,   95,   98,   98,   51,   95,   53,   98,   55,
 /*  1210 */    56,   98,   58,   59,   63,   61,   62,   29,   30,   98,
 /*  1220 */     1,   33,   34,   35,   98,   37,   38,   98,   40,   98,
 /*  1230 */    98,   98,   98,   98,   49,   98,   98,   98,   98,   51,
 /*  1240 */    98,   53,   98,   55,   56,   98,   58,   59,   63,   61,
 /*  1250 */    62,   29,   30,   98,    1,   33,   34,   35,   98,   37,
 /*  1260 */    38,   98,   40,   98,   98,   98,   98,   98,   49,   98,
 /*  1270 */    98,   98,   98,   51,   98,   53,   98,   55,   56,   98,
 /*  1280 */    58,   59,   63,   61,   62,   29,   30,   98,   98,   33,
 /*  1290 */    34,   35,    1,   37,   38,   98,   40,   98,   98,   98,
 /*  1300 */    98,   98,   49,   98,   98,   98,   98,   51,   98,   53,
 /*  1310 */    98,   55,   56,    1,   58,   59,   63,   61,   62,   29,
 /*  1320 */    30,   98,   98,   33,   34,   35,   98,   98,   38,   98,
 /*  1330 */    40,   98,   98,   98,   98,   98,   98,   98,   98,   98,
 /*  1340 */    49,   51,   98,   53,   98,   55,   56,   98,   58,   59,
 /*  1350 */     6,   61,   62,    1,   63,   98,    6,   98,   98,   98,
 /*  1360 */    98,   49,   98,   98,   20,   98,   22,   23,   98,   19,
 /*  1370 */    26,   27,   22,   23,   98,   63,   26,   27,   98,   98,
 /*  1380 */    36,   98,   98,    6,   98,   98,   36,   43,   44,   45,
 /*  1390 */    46,   47,   48,   43,   44,   45,   46,   47,   48,   22,
 /*  1400 */    23,   49,   98,   26,   27,   98,   98,   98,   98,    5,
 /*  1410 */     6,    1,   98,   36,   98,   63,   98,   98,   98,   98,
 /*  1420 */    43,   44,   45,   46,   47,   48,   22,   23,   98,   98,
 /*  1430 */    26,   27,   98,   98,   98,   98,   98,    6,   98,   98,
 /*  1440 */    36,   64,   98,   98,   98,   98,   98,   43,   44,   45,
 /*  1450 */    46,   47,   48,   22,   23,   98,   98,   26,   27,   49,
 /*  1460 */    98,   98,   98,   98,    6,   98,   98,   36,   98,   98,
 /*  1470 */    98,   98,   41,   63,   43,   44,   45,   46,   47,   48,
 /*  1480 */    22,   23,   98,   98,   26,   27,   98,   98,   98,   98,
 /*  1490 */    98,    6,   98,   98,   36,   98,   98,   98,   98,   98,
 /*  1500 */    98,   43,   44,   45,   46,   47,   48,   22,   23,   98,
 /*  1510 */    98,   26,   27,   98,   98,   98,   98,   98,    6,   98,
 /*  1520 */    98,   36,   98,   65,   98,   98,   98,   98,   43,   44,
 /*  1530 */    45,   46,   47,   48,   22,   23,   98,   98,   26,   27,
 /*  1540 */    98,   98,   98,   98,   98,    6,   98,   98,   36,   64,
 /*  1550 */    98,   98,   98,   98,   98,   43,   44,   45,   46,   47,
 /*  1560 */    48,   22,   23,   98,   98,   26,   27,   98,    1,   98,
 /*  1570 */     1,   98,    6,   98,   98,   36,   98,   98,   98,   98,
 /*  1580 */    98,   98,   43,   44,   45,   46,   47,   48,   22,   23,
 /*  1590 */    98,   98,   26,   27,   98,   98,   98,   98,   98,   98,
 /*  1600 */    98,   98,   36,   98,   98,   98,   98,   98,   98,   43,
 /*  1610 */    44,   45,   46,   47,   48,   98,   49,   98,   49,   98,
 /*  1620 */    98,   98,   98,   98,   98,   98,   98,   98,   98,   98,
 /*  1630 */    63,   98,   63,
};
#define KK_SHIFT_USE_DFLT (-4)
static short kk_shift_ofst[] = {
 /*     0 */   879,    2,  913,   -4,   -4,   -4,   -4,   -4,   -4,   -4,
 /*    10 */    -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,
 /*    20 */    -4, 1290, 1512,   97,  879,  947,  836,    0,   -4,   24,
 /*    30 */   879,  981,  848,   68,   -4, 1512,  155,   -4, 1512, 1512,
 /*    40 */   259, 1512, 1512,  259, 1344,  216, 1350,  216, 1512,  216,
 /*    50 */  1512,  609, 1512,  609, 1512,  259, 1512,   43, 1512,  551,
 /*    60 */  1512,   37, 1377,   37, 1512,   37, 1512,   37, 1512,   37,
 /*    70 */  1512,   37, 1512,   37, 1512,   37, 1512,   37, 1512,   -4,
 /*    80 */  1512,  507,   99, 1512,   43, 1404,  183,   -4, 1512,  209,
 /*    90 */  1512,  530, 1431,  482, 1512,   43, 1512,  249,   -4, 1458,
 /*   100 */   482,   -4,   64,   -4, 1539,   -4,  123, 1512,  482,   -4,
 /*   110 */    -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,  883,   -4,
 /*   120 */  1566,   -4,  179, 1512,  482,   -4,   -4,  925,   95,   -4,
 /*   130 */   186,  223,  288,  887,   -4,  283,   -4,  371,  422, 1485,
 /*   140 */    -4,   -4, 1512,  216, 1512,  216,  259,  259,  158,   -4,
 /*   150 */   316,  243, 1512,   -3,  879, 1083,  882,  294,   -4,  344,
 /*   160 */   389, 1512,  291,   -4,  419,  432,  390,  453,  879, 1117,
 /*   170 */   916,  457,   -4,   -4,  476,  842, 1151, 1185,  950,  843,
 /*   180 */    -4, 1512,   25,  879, 1219,  984,  845,   -4,  555,  854,
 /*   190 */    -4, 1512,   69,   -4, 1512,  319,   -4, 1512,  347,   -4,
 /*   200 */   244,  860,  879, 1253, 1018,  866,   -4,  891,   -4,  901,
 /*   210 */    -4, 1512,  375,   -4,   -4,   -4,  917,  879, 1291, 1052,
 /*   220 */   934,   -4,  951,  879, 1312, 1086,  959,   -4, 1120,  965,
 /*   230 */    -4,  963,  969,  879, 1352, 1154,  985,   -4,  869,   -4,
 /*   240 */   958,  235,   -4,   -4,   -4,   -4,   -4,   -4,   -4,   -4,
 /*   250 */  1512,  426,  879, 1410, 1188,  990,   -4,  995, 1038, 1512,
 /*   260 */   127,  879, 1567, 1222, 1019,   -4, 1512,  454,  879, 1569,
 /*   270 */  1256, 1024,   -4,
};
#define KK_REDUCE_USE_DFLT (-22)
static short kk_reduce_ofst[] = {
 /*     0 */   337,  -22,  777,  -22,  -22,  -22,  -22,  -22,  -22,  -22,
 /*    10 */   -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,
 /*    20 */   -22,  -22,  -21,  -22,  511,  777,  -22,  -22,  -22,  -22,
 /*    30 */   531,  777,  -22,  -22,  -22,  159,  -22,  -22,  201,  844,
 /*    40 */   -22,  875,  878,  -22,  899,  -22,  903,  -22,  912,  -22,
 /*    50 */   928,  -22,  933,  -22,  937,  -22,  938,  -22,  939,  -22,
 /*    60 */   941,  -22,  967,  -22,  971,  -22,  972,  -22,  973,  -22,
 /*    70 */   975,  -22,  977,  -22, 1001,  -22, 1003,  -22, 1006,  -22,
 /*    80 */  1007,  -22,  -22, 1009,  -22,  841,  -22,  -22, 1011,  -22,
 /*    90 */  1034,  -22,  830,  -22, 1035,  -22, 1039,  -22,  -22,  865,
 /*   100 */   -22,  -22,  -22,  -22,  898,  -22,  -22, 1040,  -22,  -22,
 /*   110 */   -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,
 /*   120 */   805,  -22,  -22, 1043,  -22,  -22,  -22,   39,  -22,  -22,
 /*   130 */   -22,  -22,  -22,  227,  -22,  -22,  -22,  -22,  -22,  962,
 /*   140 */   -22,  -22, 1048,  -22, 1068,  -22,  -22,  -22,  -22,  -22,
 /*   150 */   -22,  -22, 1069,  -22,  569,  777,  -22,  -22,  -22,  -22,
 /*   160 */   -22, 1073,  -22,  -22,  -22,  -22,  160,  -22,  588,  777,
 /*   170 */   -22,  -22,  -22,  -22,  -22,  -22,  607,  777,  -22,  -22,
 /*   180 */   -22, 1074,  -22,  626,  777,  -22,  -22,  -22,  -22,  -22,
 /*   190 */   -22, 1079,  -22,  -22, 1082,  -22,  -22, 1102,  -22,  -22,
 /*   200 */   -22,  -22,  645,  777,  -22,  -22,  -22,  -22,  -22,  -22,
 /*   210 */   -22, 1103,  -22,  -22,  -22,  -22,  -22,  664,  777,  -22,
 /*   220 */   -22,  -22,  -22,  683,  777,  -22,  -22,  -22,  -22,  -22,
 /*   230 */   -22,  -22,  -22,  702,  777,  -22,  -22,  -22,  818,  -22,
 /*   240 */   -22,  864,  -22,  -22,  -22,  -22,  -22,  -22,  -22,  -22,
 /*   250 */  1005,  -22,  721,  777,  -22,  -22,  -22,  -22,  -22, 1107,
 /*   260 */   -22,  740,  777,  -22,  -22,  -22, 1111,  -22,  759,  777,
 /*   270 */   -22,  -22,  -22,
};
static KKACTIONTYPE kk_default[] = {
 /*     0 */   387,  387,  387,  275,  277,  278,  279,  280,  281,  282,
 /*    10 */   283,  284,  285,  286,  287,  288,  289,  290,  291,  292,
 /*    20 */   293,  387,  387,  387,  387,  387,  387,  387,  294,  387,
 /*    30 */   387,  387,  387,  387,  295,  387,  387,  296,  387,  387,
 /*    40 */   330,  387,  387,  331,  387,  334,  387,  336,  387,  338,
 /*    50 */   387,  339,  387,  340,  387,  341,  387,  342,  387,  343,
 /*    60 */   387,  344,  387,  347,  387,  348,  387,  349,  387,  350,
 /*    70 */   387,  351,  387,  352,  387,  353,  387,  354,  387,  355,
 /*    80 */   387,  356,  387,  387,  357,  387,  387,  362,  387,  387,
 /*    90 */   387,  363,  387,  378,  387,  358,  387,  387,  359,  387,
 /*   100 */   372,  360,  387,  361,  387,  369,  382,  387,  371,  373,
 /*   110 */   380,  381,  382,  383,  384,  385,  386,  370,  387,  374,
 /*   120 */   387,  376,  382,  387,  379,  375,  377,  387,  387,  364,
 /*   130 */   387,  387,  387,  387,  365,  387,  366,  381,  380,  387,
 /*   140 */   345,  346,  387,  337,  387,  335,  333,  332,  387,  297,
 /*   150 */   387,  387,  387,  387,  387,  387,  387,  387,  298,  387,
 /*   160 */   387,  387,  387,  302,  387,  387,  387,  387,  387,  387,
 /*   170 */   387,  387,  303,  315,  387,  387,  387,  387,  387,  387,
 /*   180 */   317,  387,  387,  387,  387,  387,  387,  319,  387,  387,
 /*   190 */   321,  387,  387,  322,  387,  387,  323,  387,  387,  324,
 /*   200 */   387,  387,  387,  387,  387,  387,  325,  387,  327,  387,
 /*   210 */   328,  387,  387,  316,  329,  276,  387,  387,  387,  387,
 /*   220 */   387,  326,  387,  387,  387,  387,  387,  320,  387,  387,
 /*   230 */   318,  387,  387,  387,  387,  387,  387,  304,  387,  305,
 /*   240 */   307,  387,  308,  309,  310,  311,  312,  313,  314,  306,
 /*   250 */   387,  387,  387,  387,  387,  387,  299,  387,  387,  387,
 /*   260 */   387,  387,  387,  387,  387,  300,  387,  387,  387,  387,
 /*   270 */   387,  387,  301,
};
#define KK_SZ_ACTTAB (sizeof(kk_action)/sizeof(kk_action[0]))

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammer, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef KKFALLBACK
static const KKCODETYPE kkFallback[] = {
};
#endif /* KKFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct kkStackEntry {
  int stateno;       /* The state-number */
  int major;         /* The major token value.  This is the code
                     ** number for the token at this stack level */
  KKMINORTYPE minor; /* The user-supplied minor token value.  This
                     ** is the value of the token  */
};
typedef struct kkStackEntry kkStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct kkParser {
  int kkidx;                    /* Index of top element in stack */
  int kkerrcnt;                 /* Shifts left before out of the error */
  phvolt_ARG_SDECL                /* A place to hold %extra_argument */
  kkStackEntry kkstack[KKSTACKDEPTH];  /* The parser's stack */
};
typedef struct kkParser kkParser;

#ifndef NDEBUG
#include <stdio.h>
static FILE *kkTraceFILE = 0;
static char *kkTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
void phvolt_Trace(FILE *TraceFILE, char *zTracePrompt){
  kkTraceFILE = TraceFILE;
  kkTracePrompt = zTracePrompt;
  if( kkTraceFILE==0 ) kkTracePrompt = 0;
  else if( kkTracePrompt==0 ) kkTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *kkTokenName[] = { 
  "$",             "OPEN_DELIMITER",  "COMMA",         "IN",          
  "QUESTION",      "COLON",         "SBRACKET_OPEN",  "RANGE",       
  "AND",           "OR",            "IS",            "EQUALS",      
  "NOTEQUALS",     "LESS",          "GREATER",       "GREATEREQUAL",
  "LESSEQUAL",     "IDENTICAL",     "NOTIDENTICAL",  "DIVIDE",      
  "TIMES",         "MOD",           "PLUS",          "MINUS",       
  "CONCAT",        "PIPE",          "NOT",           "PARENTHESES_OPEN",
  "DOT",           "IF",            "CLOSE_DELIMITER",  "ENDIF",       
  "ELSE",          "ELSEIF",        "ELSEFOR",       "FOR",         
  "IDENTIFIER",    "ENDFOR",        "SET",           "ASSIGN",      
  "MACRO",         "PARENTHESES_CLOSE",  "ENDMACRO",      "INTEGER",     
  "STRING",        "DOUBLE",        "NULL",          "FALSE",       
  "TRUE",          "OPEN_EDELIMITER",  "CLOSE_EDELIMITER",  "BLOCK",       
  "ENDBLOCK",      "CACHE",         "ENDCACHE",      "EXTENDS",     
  "INCLUDE",       "WITH",          "DO",            "AUTOESCAPE",  
  "ENDAUTOESCAPE",  "BREAK",         "CONTINUE",      "RAW_FRAGMENT",
  "DEFINED",       "SBRACKET_CLOSE",  "error",         "program",     
  "volt_language",  "statement_list",  "statement",     "raw_fragment",
  "if_statement",  "elseif_statement",  "elsefor_statement",  "for_statement",
  "set_statement",  "echo_statement",  "block_statement",  "cache_statement",
  "extends_statement",  "include_statement",  "do_statement",  "autoescape_statement",
  "break_statement",  "continue_statement",  "macro_statement",  "empty_statement",
  "expr",          "macro_parameters",  "macro_parameter",  "macro_parameter_default",
  "array_list",    "slice_offset",  "array_item",    "function_call",
  "argument_list",  "argument_item",
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *kkRuleName[] = {
 /*   0 */ "program ::= volt_language",
 /*   1 */ "volt_language ::= statement_list",
 /*   2 */ "statement_list ::= statement_list statement",
 /*   3 */ "statement_list ::= statement",
 /*   4 */ "statement ::= raw_fragment",
 /*   5 */ "statement ::= if_statement",
 /*   6 */ "statement ::= elseif_statement",
 /*   7 */ "statement ::= elsefor_statement",
 /*   8 */ "statement ::= for_statement",
 /*   9 */ "statement ::= set_statement",
 /*  10 */ "statement ::= echo_statement",
 /*  11 */ "statement ::= block_statement",
 /*  12 */ "statement ::= cache_statement",
 /*  13 */ "statement ::= extends_statement",
 /*  14 */ "statement ::= include_statement",
 /*  15 */ "statement ::= do_statement",
 /*  16 */ "statement ::= autoescape_statement",
 /*  17 */ "statement ::= break_statement",
 /*  18 */ "statement ::= continue_statement",
 /*  19 */ "statement ::= macro_statement",
 /*  20 */ "statement ::= empty_statement",
 /*  21 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  22 */ "if_statement ::= OPEN_DELIMITER IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ELSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDIF CLOSE_DELIMITER",
 /*  23 */ "elseif_statement ::= OPEN_DELIMITER ELSEIF expr CLOSE_DELIMITER",
 /*  24 */ "elsefor_statement ::= OPEN_DELIMITER ELSEFOR CLOSE_DELIMITER",
 /*  25 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER IN expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  26 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER IN expr IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  27 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER COMMA IDENTIFIER IN expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  28 */ "for_statement ::= OPEN_DELIMITER FOR IDENTIFIER COMMA IDENTIFIER IN expr IF expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDFOR CLOSE_DELIMITER",
 /*  29 */ "set_statement ::= OPEN_DELIMITER SET IDENTIFIER ASSIGN expr CLOSE_DELIMITER",
 /*  30 */ "macro_statement ::= OPEN_DELIMITER MACRO IDENTIFIER PARENTHESES_OPEN PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDMACRO CLOSE_DELIMITER",
 /*  31 */ "macro_statement ::= OPEN_DELIMITER MACRO IDENTIFIER PARENTHESES_OPEN macro_parameters PARENTHESES_CLOSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDMACRO CLOSE_DELIMITER",
 /*  32 */ "macro_parameters ::= macro_parameters COMMA macro_parameter",
 /*  33 */ "macro_parameters ::= macro_parameter",
 /*  34 */ "macro_parameter ::= IDENTIFIER",
 /*  35 */ "macro_parameter ::= IDENTIFIER ASSIGN macro_parameter_default",
 /*  36 */ "macro_parameter_default ::= INTEGER",
 /*  37 */ "macro_parameter_default ::= STRING",
 /*  38 */ "macro_parameter_default ::= DOUBLE",
 /*  39 */ "macro_parameter_default ::= NULL",
 /*  40 */ "macro_parameter_default ::= FALSE",
 /*  41 */ "macro_parameter_default ::= TRUE",
 /*  42 */ "empty_statement ::= OPEN_DELIMITER CLOSE_DELIMITER",
 /*  43 */ "echo_statement ::= OPEN_EDELIMITER expr CLOSE_EDELIMITER",
 /*  44 */ "block_statement ::= OPEN_DELIMITER BLOCK IDENTIFIER CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDBLOCK CLOSE_DELIMITER",
 /*  45 */ "block_statement ::= OPEN_DELIMITER BLOCK IDENTIFIER CLOSE_DELIMITER OPEN_DELIMITER ENDBLOCK CLOSE_DELIMITER",
 /*  46 */ "cache_statement ::= OPEN_DELIMITER CACHE expr CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCACHE CLOSE_DELIMITER",
 /*  47 */ "cache_statement ::= OPEN_DELIMITER CACHE expr INTEGER CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDCACHE CLOSE_DELIMITER",
 /*  48 */ "extends_statement ::= OPEN_DELIMITER EXTENDS STRING CLOSE_DELIMITER",
 /*  49 */ "include_statement ::= OPEN_DELIMITER INCLUDE expr CLOSE_DELIMITER",
 /*  50 */ "include_statement ::= OPEN_DELIMITER INCLUDE expr WITH expr CLOSE_DELIMITER",
 /*  51 */ "do_statement ::= OPEN_DELIMITER DO expr CLOSE_DELIMITER",
 /*  52 */ "autoescape_statement ::= OPEN_DELIMITER AUTOESCAPE FALSE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDAUTOESCAPE CLOSE_DELIMITER",
 /*  53 */ "autoescape_statement ::= OPEN_DELIMITER AUTOESCAPE TRUE CLOSE_DELIMITER statement_list OPEN_DELIMITER ENDAUTOESCAPE CLOSE_DELIMITER",
 /*  54 */ "break_statement ::= OPEN_DELIMITER BREAK CLOSE_DELIMITER",
 /*  55 */ "continue_statement ::= OPEN_DELIMITER CONTINUE CLOSE_DELIMITER",
 /*  56 */ "raw_fragment ::= RAW_FRAGMENT",
 /*  57 */ "expr ::= MINUS expr",
 /*  58 */ "expr ::= PLUS expr",
 /*  59 */ "expr ::= expr MINUS expr",
 /*  60 */ "expr ::= expr PLUS expr",
 /*  61 */ "expr ::= expr TIMES expr",
 /*  62 */ "expr ::= expr TIMES TIMES expr",
 /*  63 */ "expr ::= expr DIVIDE expr",
 /*  64 */ "expr ::= expr DIVIDE DIVIDE expr",
 /*  65 */ "expr ::= expr MOD expr",
 /*  66 */ "expr ::= expr AND expr",
 /*  67 */ "expr ::= expr OR expr",
 /*  68 */ "expr ::= expr CONCAT expr",
 /*  69 */ "expr ::= expr PIPE expr",
 /*  70 */ "expr ::= expr RANGE expr",
 /*  71 */ "expr ::= expr EQUALS expr",
 /*  72 */ "expr ::= expr IS NOT DEFINED",
 /*  73 */ "expr ::= expr IS DEFINED",
 /*  74 */ "expr ::= expr IS expr",
 /*  75 */ "expr ::= expr NOTEQUALS expr",
 /*  76 */ "expr ::= expr IDENTICAL expr",
 /*  77 */ "expr ::= expr NOTIDENTICAL expr",
 /*  78 */ "expr ::= expr LESS expr",
 /*  79 */ "expr ::= expr GREATER expr",
 /*  80 */ "expr ::= expr GREATEREQUAL expr",
 /*  81 */ "expr ::= expr LESSEQUAL expr",
 /*  82 */ "expr ::= expr DOT expr",
 /*  83 */ "expr ::= expr IN expr",
 /*  84 */ "expr ::= expr NOT IN expr",
 /*  85 */ "expr ::= NOT expr",
 /*  86 */ "expr ::= PARENTHESES_OPEN expr PARENTHESES_CLOSE",
 /*  87 */ "expr ::= SBRACKET_OPEN SBRACKET_CLOSE",
 /*  88 */ "expr ::= SBRACKET_OPEN array_list SBRACKET_CLOSE",
 /*  89 */ "expr ::= expr SBRACKET_OPEN expr SBRACKET_CLOSE",
 /*  90 */ "expr ::= expr QUESTION expr COLON expr",
 /*  91 */ "expr ::= expr SBRACKET_OPEN COLON slice_offset SBRACKET_CLOSE",
 /*  92 */ "expr ::= expr SBRACKET_OPEN slice_offset COLON SBRACKET_CLOSE",
 /*  93 */ "expr ::= expr SBRACKET_OPEN slice_offset COLON slice_offset SBRACKET_CLOSE",
 /*  94 */ "slice_offset ::= INTEGER",
 /*  95 */ "slice_offset ::= IDENTIFIER",
 /*  96 */ "array_list ::= array_list COMMA array_item",
 /*  97 */ "array_list ::= array_item",
 /*  98 */ "array_item ::= STRING COLON expr",
 /*  99 */ "array_item ::= expr",
 /* 100 */ "expr ::= function_call",
 /* 101 */ "function_call ::= expr PARENTHESES_OPEN argument_list PARENTHESES_CLOSE",
 /* 102 */ "function_call ::= expr PARENTHESES_OPEN PARENTHESES_CLOSE",
 /* 103 */ "argument_list ::= argument_list COMMA argument_item",
 /* 104 */ "argument_list ::= argument_item",
 /* 105 */ "argument_item ::= expr",
 /* 106 */ "argument_item ::= STRING COLON expr",
 /* 107 */ "expr ::= IDENTIFIER",
 /* 108 */ "expr ::= INTEGER",
 /* 109 */ "expr ::= STRING",
 /* 110 */ "expr ::= DOUBLE",
 /* 111 */ "expr ::= NULL",
 /* 112 */ "expr ::= FALSE",
 /* 113 */ "expr ::= TRUE",
};
#endif /* NDEBUG */

/*
** This function returns the symbolic name associated with a token
** value.
*/
const char *phvolt_TokenName(int tokenType){
#ifndef NDEBUG
  if( tokenType>0 && tokenType<(sizeof(kkTokenName)/sizeof(kkTokenName[0])) ){
    return kkTokenName[tokenType];
  }else{
    return "Unknown";
  }
#else
  return "";
#endif
}

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to phvolt_ and phvolt_Free.
*/
void *phvolt_Alloc(void *(*mallocProc)(size_t)){
  kkParser *pParser;
  pParser = (kkParser*)(*mallocProc)( (size_t)sizeof(kkParser) );
  if( pParser ){
    pParser->kkidx = -1;
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "kkmajor" is the symbol code, and "kkpminor" is a pointer to
** the value.
*/
static void kk_destructor(KKCODETYPE kkmajor, KKMINORTYPE *kkpminor){
  switch( kkmajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
// 590 "parser.lemon"
{
	if ((kkpminor->kk0)) {
		if ((kkpminor->kk0)->free_flag) {
			efree((kkpminor->kk0)->token);
		}
		efree((kkpminor->kk0));
	}
}
// 1412 "parser.c"
      break;
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
// 607 "parser.lemon"
{ zval_ptr_dtor(&(kkpminor->kk92)); }
// 1444 "parser.c"
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int kk_pop_parser_stack(kkParser *pParser){
  KKCODETYPE kkmajor;
  kkStackEntry *kktos = &pParser->kkstack[pParser->kkidx];

  if( pParser->kkidx<0 ) return 0;
#ifndef NDEBUG
  if( kkTraceFILE && pParser->kkidx>=0 ){
    fprintf(kkTraceFILE,"%sPopping %s\n",
      kkTracePrompt,
      kkTokenName[kktos->major]);
  }
#endif
  kkmajor = kktos->major;
  kk_destructor( kkmajor, &kktos->minor);
  pParser->kkidx--;
  return kkmajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from phvolt_Alloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
void phvolt_Free(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  kkParser *pParser = (kkParser*)p;
  if( pParser==0 ) return;
  while( pParser->kkidx>=0 ) kk_pop_parser_stack(pParser);
  (*freeProc)((void*)pParser);
}

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is KKNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return KK_NO_ACTION.
*/
static int kk_find_shift_action(
  kkParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->kkstack[pParser->kkidx].stateno;
 
  /* if( pParser->kkidx<0 ) return KK_NO_ACTION;  */
  i = kk_shift_ofst[stateno];
  if( i==KK_SHIFT_USE_DFLT ){
    return kk_default[stateno];
  }
  if( iLookAhead==KKNOCODE ){
    return KK_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=KK_SZ_ACTTAB || kk_lookahead[i]!=iLookAhead ){
#ifdef KKFALLBACK
    int iFallback;            /* Fallback token */
    if( iLookAhead<sizeof(kkFallback)/sizeof(kkFallback[0])
           && (iFallback = kkFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
      if( kkTraceFILE ){
        fprintf(kkTraceFILE, "%sFALLBACK %s => %s\n",
           kkTracePrompt, kkTokenName[iLookAhead], kkTokenName[iFallback]);
      }
#endif
      return kk_find_shift_action(pParser, iFallback);
    }
#endif
    return kk_default[stateno];
  }else{
    return kk_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is KKNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return KK_NO_ACTION.
*/
static int kk_find_reduce_action(
  kkParser *pParser,        /* The parser */
  int iLookAhead            /* The look-ahead token */
){
  int i;
  int stateno = pParser->kkstack[pParser->kkidx].stateno;
 
  i = kk_reduce_ofst[stateno];
  if( i==KK_REDUCE_USE_DFLT ){
    return kk_default[stateno];
  }
  if( iLookAhead==KKNOCODE ){
    return KK_NO_ACTION;
  }
  i += iLookAhead;
  if( i<0 || i>=KK_SZ_ACTTAB || kk_lookahead[i]!=iLookAhead ){
    return kk_default[stateno];
  }else{
    return kk_action[i];
  }
}

/*
** Perform a shift action.
*/
static void kk_shift(
  kkParser *kkpParser,          /* The parser to be shifted */
  int kkNewState,               /* The new state to shift in */
  int kkMajor,                  /* The major token to shift in */
  KKMINORTYPE *kkpMinor         /* Pointer ot the minor token to shift in */
){
  kkStackEntry *kktos;
  kkpParser->kkidx++;
  if( kkpParser->kkidx>=KKSTACKDEPTH ){
     phvolt_ARG_FETCH;
     kkpParser->kkidx--;
#ifndef NDEBUG
     if( kkTraceFILE ){
       fprintf(kkTraceFILE,"%sStack Overflow!\n",kkTracePrompt);
     }
#endif
     while( kkpParser->kkidx>=0 ) kk_pop_parser_stack(kkpParser);
     /* Here code is inserted which will execute if the parser
     ** stack every overflows */
     phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument var */
     return;
  }
  kktos = &kkpParser->kkstack[kkpParser->kkidx];
  kktos->stateno = kkNewState;
  kktos->major = kkMajor;
  kktos->minor = *kkpMinor;
#ifndef NDEBUG
  if( kkTraceFILE && kkpParser->kkidx>0 ){
    int i;
    fprintf(kkTraceFILE,"%sShift %d\n",kkTracePrompt,kkNewState);
    fprintf(kkTraceFILE,"%sStack:",kkTracePrompt);
    for(i=1; i<=kkpParser->kkidx; i++)
      fprintf(kkTraceFILE," %s",kkTokenName[kkpParser->kkstack[i].major]);
    fprintf(kkTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static struct {
  KKCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} kkRuleInfo[] = {
  { 67, 1 },
  { 68, 1 },
  { 69, 2 },
  { 69, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 70, 1 },
  { 72, 8 },
  { 72, 12 },
  { 73, 4 },
  { 74, 3 },
  { 75, 10 },
  { 75, 12 },
  { 75, 12 },
  { 75, 14 },
  { 76, 6 },
  { 86, 10 },
  { 86, 11 },
  { 89, 3 },
  { 89, 1 },
  { 90, 1 },
  { 90, 3 },
  { 91, 1 },
  { 91, 1 },
  { 91, 1 },
  { 91, 1 },
  { 91, 1 },
  { 91, 1 },
  { 87, 2 },
  { 77, 3 },
  { 78, 8 },
  { 78, 7 },
  { 79, 8 },
  { 79, 9 },
  { 80, 4 },
  { 81, 4 },
  { 81, 6 },
  { 82, 4 },
  { 83, 8 },
  { 83, 8 },
  { 84, 3 },
  { 85, 3 },
  { 71, 1 },
  { 88, 2 },
  { 88, 2 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 4 },
  { 88, 3 },
  { 88, 4 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 4 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 3 },
  { 88, 4 },
  { 88, 2 },
  { 88, 3 },
  { 88, 2 },
  { 88, 3 },
  { 88, 4 },
  { 88, 5 },
  { 88, 5 },
  { 88, 5 },
  { 88, 6 },
  { 93, 1 },
  { 93, 1 },
  { 92, 3 },
  { 92, 1 },
  { 94, 3 },
  { 94, 1 },
  { 88, 1 },
  { 95, 4 },
  { 95, 3 },
  { 96, 3 },
  { 96, 1 },
  { 97, 1 },
  { 97, 3 },
  { 88, 1 },
  { 88, 1 },
  { 88, 1 },
  { 88, 1 },
  { 88, 1 },
  { 88, 1 },
  { 88, 1 },
};

static void kk_accept(kkParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void kk_reduce(
  kkParser *kkpParser,         /* The parser */
  int kkruleno                 /* Number of the rule by which to reduce */
){
  int kkgoto;                     /* The next state */
  int kkact;                      /* The next action */
  KKMINORTYPE kkgotominor;        /* The LHS of the rule reduced */
  kkStackEntry *kkmsp;            /* The top of the parser's stack */
  int kksize;                     /* Amount to pop the stack */
  phvolt_ARG_FETCH;
  kkmsp = &kkpParser->kkstack[kkpParser->kkidx];
#ifndef NDEBUG
  if( kkTraceFILE && kkruleno>=0 
        && kkruleno<sizeof(kkRuleName)/sizeof(kkRuleName[0]) ){
    fprintf(kkTraceFILE, "%sReduce [%s].\n", kkTracePrompt,
      kkRuleName[kkruleno]);
  }
#endif /* NDEBUG */

  switch( kkruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  // <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  // <lineno> <thisfile>
  **     break;
  */
      case 0:
// 599 "parser.lemon"
{
	status->ret = kkmsp[0].minor.kk92;
}
// 1775 "parser.c"
        break;
      case 1:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 100:
// 603 "parser.lemon"
{
	kkgotominor.kk92 = kkmsp[0].minor.kk92;
}
// 1800 "parser.c"
        break;
      case 2:
// 609 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_zval_list(kkmsp[-1].minor.kk92, kkmsp[0].minor.kk92);
}
// 1807 "parser.c"
        break;
      case 3:
      case 33:
      case 97:
      case 104:
// 613 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_zval_list(NULL, kkmsp[0].minor.kk92);
}
// 1817 "parser.c"
        break;
      case 21:
// 689 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_if_statement(kkmsp[-5].minor.kk92, kkmsp[-3].minor.kk92, NULL, status->scanner_state);
  kk_destructor(1,&kkmsp[-7].minor);
  kk_destructor(29,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(31,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1830 "parser.c"
        break;
      case 22:
// 693 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_if_statement(kkmsp[-9].minor.kk92, kkmsp[-7].minor.kk92, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-11].minor);
  kk_destructor(29,&kkmsp[-10].minor);
  kk_destructor(30,&kkmsp[-8].minor);
  kk_destructor(1,&kkmsp[-6].minor);
  kk_destructor(32,&kkmsp[-5].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(31,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1846 "parser.c"
        break;
      case 23:
// 699 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_elseif_statement(kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-3].minor);
  kk_destructor(33,&kkmsp[-2].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1856 "parser.c"
        break;
      case 24:
// 705 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_elsefor_statement(status->scanner_state);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(34,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1866 "parser.c"
        break;
      case 25:
// 711 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_for_statement(kkmsp[-7].minor.kk0, NULL, kkmsp[-5].minor.kk92, NULL, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-9].minor);
  kk_destructor(35,&kkmsp[-8].minor);
  kk_destructor(3,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(37,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1880 "parser.c"
        break;
      case 26:
// 715 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_for_statement(kkmsp[-9].minor.kk0, NULL, kkmsp[-7].minor.kk92, kkmsp[-5].minor.kk92, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-11].minor);
  kk_destructor(35,&kkmsp[-10].minor);
  kk_destructor(3,&kkmsp[-8].minor);
  kk_destructor(29,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(37,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1895 "parser.c"
        break;
      case 27:
// 719 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_for_statement(kkmsp[-7].minor.kk0, kkmsp[-9].minor.kk0, kkmsp[-5].minor.kk92, NULL, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-11].minor);
  kk_destructor(35,&kkmsp[-10].minor);
  kk_destructor(2,&kkmsp[-8].minor);
  kk_destructor(3,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(37,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1910 "parser.c"
        break;
      case 28:
// 723 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_for_statement(kkmsp[-9].minor.kk0, kkmsp[-11].minor.kk0, kkmsp[-7].minor.kk92, kkmsp[-5].minor.kk92, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-13].minor);
  kk_destructor(35,&kkmsp[-12].minor);
  kk_destructor(2,&kkmsp[-10].minor);
  kk_destructor(3,&kkmsp[-8].minor);
  kk_destructor(29,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(37,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1926 "parser.c"
        break;
      case 29:
// 729 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_set_statement(kkmsp[-3].minor.kk0, kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-5].minor);
  kk_destructor(38,&kkmsp[-4].minor);
  kk_destructor(39,&kkmsp[-2].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1937 "parser.c"
        break;
      case 30:
// 735 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_macro_statement(kkmsp[-7].minor.kk0, NULL, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-9].minor);
  kk_destructor(40,&kkmsp[-8].minor);
  kk_destructor(27,&kkmsp[-6].minor);
  kk_destructor(41,&kkmsp[-5].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(42,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1952 "parser.c"
        break;
      case 31:
// 739 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_macro_statement(kkmsp[-8].minor.kk0, kkmsp[-6].minor.kk92, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-10].minor);
  kk_destructor(40,&kkmsp[-9].minor);
  kk_destructor(27,&kkmsp[-7].minor);
  kk_destructor(41,&kkmsp[-5].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(42,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 1967 "parser.c"
        break;
      case 32:
      case 96:
      case 103:
// 745 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_zval_list(kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92);
  kk_destructor(2,&kkmsp[-1].minor);
}
// 1977 "parser.c"
        break;
      case 34:
// 755 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_macro_parameter(kkmsp[0].minor.kk0, NULL, status->scanner_state);
}
// 1984 "parser.c"
        break;
      case 35:
// 759 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_macro_parameter(kkmsp[-2].minor.kk0, kkmsp[0].minor.kk92, status->scanner_state);
  kk_destructor(39,&kkmsp[-1].minor);
}
// 1992 "parser.c"
        break;
      case 36:
      case 94:
      case 108:
// 763 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_INTEGER, kkmsp[0].minor.kk0, status->scanner_state);
}
// 2001 "parser.c"
        break;
      case 37:
      case 109:
// 767 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_STRING, kkmsp[0].minor.kk0, status->scanner_state);
}
// 2009 "parser.c"
        break;
      case 38:
      case 110:
// 771 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_DOUBLE, kkmsp[0].minor.kk0, status->scanner_state);
}
// 2017 "parser.c"
        break;
      case 39:
      case 111:
// 775 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_NULL, NULL, status->scanner_state);
  kk_destructor(46,&kkmsp[0].minor);
}
// 2026 "parser.c"
        break;
      case 40:
      case 112:
// 779 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_FALSE, NULL, status->scanner_state);
  kk_destructor(47,&kkmsp[0].minor);
}
// 2035 "parser.c"
        break;
      case 41:
      case 113:
// 783 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_TRUE, NULL, status->scanner_state);
  kk_destructor(48,&kkmsp[0].minor);
}
// 2044 "parser.c"
        break;
      case 42:
// 789 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_empty_statement(status->scanner_state);
  kk_destructor(1,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2053 "parser.c"
        break;
      case 43:
// 795 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_echo_statement(kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(49,&kkmsp[-2].minor);
  kk_destructor(50,&kkmsp[0].minor);
}
// 2062 "parser.c"
        break;
      case 44:
// 801 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_block_statement(kkmsp[-5].minor.kk0, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-7].minor);
  kk_destructor(51,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(52,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2075 "parser.c"
        break;
      case 45:
// 805 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_block_statement(kkmsp[-4].minor.kk0, NULL, status->scanner_state);
  kk_destructor(1,&kkmsp[-6].minor);
  kk_destructor(51,&kkmsp[-5].minor);
  kk_destructor(30,&kkmsp[-3].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(52,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2088 "parser.c"
        break;
      case 46:
// 811 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_cache_statement(kkmsp[-5].minor.kk92, NULL, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-7].minor);
  kk_destructor(53,&kkmsp[-6].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(54,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2101 "parser.c"
        break;
      case 47:
// 815 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_cache_statement(kkmsp[-6].minor.kk92, kkmsp[-5].minor.kk0, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-8].minor);
  kk_destructor(53,&kkmsp[-7].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(54,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2114 "parser.c"
        break;
      case 48:
// 821 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_extends_statement(kkmsp[-1].minor.kk0, status->scanner_state);
  kk_destructor(1,&kkmsp[-3].minor);
  kk_destructor(55,&kkmsp[-2].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2124 "parser.c"
        break;
      case 49:
// 827 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_include_statement(kkmsp[-1].minor.kk92, NULL, status->scanner_state);
  kk_destructor(1,&kkmsp[-3].minor);
  kk_destructor(56,&kkmsp[-2].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2134 "parser.c"
        break;
      case 50:
// 831 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_include_statement(kkmsp[-3].minor.kk92, kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-5].minor);
  kk_destructor(56,&kkmsp[-4].minor);
  kk_destructor(57,&kkmsp[-2].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2145 "parser.c"
        break;
      case 51:
// 837 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_do_statement(kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-3].minor);
  kk_destructor(58,&kkmsp[-2].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2155 "parser.c"
        break;
      case 52:
// 843 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_autoescape_statement(0, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-7].minor);
  kk_destructor(59,&kkmsp[-6].minor);
  kk_destructor(47,&kkmsp[-5].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(60,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2169 "parser.c"
        break;
      case 53:
// 847 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_autoescape_statement(1, kkmsp[-3].minor.kk92, status->scanner_state);
  kk_destructor(1,&kkmsp[-7].minor);
  kk_destructor(59,&kkmsp[-6].minor);
  kk_destructor(48,&kkmsp[-5].minor);
  kk_destructor(30,&kkmsp[-4].minor);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(60,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2183 "parser.c"
        break;
      case 54:
// 853 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_break_statement(status->scanner_state);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(61,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2193 "parser.c"
        break;
      case 55:
// 859 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_continue_statement(status->scanner_state);
  kk_destructor(1,&kkmsp[-2].minor);
  kk_destructor(62,&kkmsp[-1].minor);
  kk_destructor(30,&kkmsp[0].minor);
}
// 2203 "parser.c"
        break;
      case 56:
// 865 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_RAW_FRAGMENT, kkmsp[0].minor.kk0, status->scanner_state);
}
// 2210 "parser.c"
        break;
      case 57:
// 871 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_MINUS, NULL, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(23,&kkmsp[-1].minor);
}
// 2218 "parser.c"
        break;
      case 58:
// 875 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_PLUS, NULL, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(22,&kkmsp[-1].minor);
}
// 2226 "parser.c"
        break;
      case 59:
// 879 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_SUB, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(23,&kkmsp[-1].minor);
}
// 2234 "parser.c"
        break;
      case 60:
// 883 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_ADD, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(22,&kkmsp[-1].minor);
}
// 2242 "parser.c"
        break;
      case 61:
// 887 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_MUL, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(20,&kkmsp[-1].minor);
}
// 2250 "parser.c"
        break;
      case 62:
// 891 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_POW, kkmsp[-3].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(20,&kkmsp[-2].minor);
  kk_destructor(20,&kkmsp[-1].minor);
}
// 2259 "parser.c"
        break;
      case 63:
// 895 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_DIV, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(19,&kkmsp[-1].minor);
}
// 2267 "parser.c"
        break;
      case 64:
// 899 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_MOD, kkmsp[-3].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(19,&kkmsp[-2].minor);
  kk_destructor(19,&kkmsp[-1].minor);
}
// 2276 "parser.c"
        break;
      case 65:
// 903 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_MOD, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(21,&kkmsp[-1].minor);
}
// 2284 "parser.c"
        break;
      case 66:
// 907 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_AND, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(8,&kkmsp[-1].minor);
}
// 2292 "parser.c"
        break;
      case 67:
// 911 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_OR, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(9,&kkmsp[-1].minor);
}
// 2300 "parser.c"
        break;
      case 68:
// 915 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_CONCAT, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(24,&kkmsp[-1].minor);
}
// 2308 "parser.c"
        break;
      case 69:
// 919 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_PIPE, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(25,&kkmsp[-1].minor);
}
// 2316 "parser.c"
        break;
      case 70:
// 923 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_RANGE, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(7,&kkmsp[-1].minor);
}
// 2324 "parser.c"
        break;
      case 71:
// 927 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_EQUALS, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(11,&kkmsp[-1].minor);
}
// 2332 "parser.c"
        break;
      case 72:
// 931 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_NOT_ISSET, kkmsp[-3].minor.kk92, NULL, NULL, status->scanner_state);
  kk_destructor(10,&kkmsp[-2].minor);
  kk_destructor(26,&kkmsp[-1].minor);
  kk_destructor(64,&kkmsp[0].minor);
}
// 2342 "parser.c"
        break;
      case 73:
// 935 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_ISSET, kkmsp[-2].minor.kk92, NULL, NULL, status->scanner_state);
  kk_destructor(10,&kkmsp[-1].minor);
  kk_destructor(64,&kkmsp[0].minor);
}
// 2351 "parser.c"
        break;
      case 74:
// 939 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_IS, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(10,&kkmsp[-1].minor);
}
// 2359 "parser.c"
        break;
      case 75:
// 943 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_NOTEQUALS, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(12,&kkmsp[-1].minor);
}
// 2367 "parser.c"
        break;
      case 76:
// 947 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_IDENTICAL, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(17,&kkmsp[-1].minor);
}
// 2375 "parser.c"
        break;
      case 77:
// 951 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_NOTIDENTICAL, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(18,&kkmsp[-1].minor);
}
// 2383 "parser.c"
        break;
      case 78:
// 955 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_LESS, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(13,&kkmsp[-1].minor);
}
// 2391 "parser.c"
        break;
      case 79:
// 959 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_GREATER, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(14,&kkmsp[-1].minor);
}
// 2399 "parser.c"
        break;
      case 80:
// 963 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_GREATEREQUAL, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(15,&kkmsp[-1].minor);
}
// 2407 "parser.c"
        break;
      case 81:
// 967 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_LESSEQUAL, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(16,&kkmsp[-1].minor);
}
// 2415 "parser.c"
        break;
      case 82:
// 971 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_DOT, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(28,&kkmsp[-1].minor);
}
// 2423 "parser.c"
        break;
      case 83:
// 975 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_IN, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(3,&kkmsp[-1].minor);
}
// 2431 "parser.c"
        break;
      case 84:
// 979 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_NOT_IN, kkmsp[-3].minor.kk92, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(26,&kkmsp[-2].minor);
  kk_destructor(3,&kkmsp[-1].minor);
}
// 2440 "parser.c"
        break;
      case 85:
// 983 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_NOT, NULL, kkmsp[0].minor.kk92, NULL, status->scanner_state);
  kk_destructor(26,&kkmsp[-1].minor);
}
// 2448 "parser.c"
        break;
      case 86:
// 987 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_ENCLOSED, kkmsp[-1].minor.kk92, NULL, NULL, status->scanner_state);
  kk_destructor(27,&kkmsp[-2].minor);
  kk_destructor(41,&kkmsp[0].minor);
}
// 2457 "parser.c"
        break;
      case 87:
// 991 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_ARRAY, NULL, NULL, NULL, status->scanner_state);
  kk_destructor(6,&kkmsp[-1].minor);
  kk_destructor(65,&kkmsp[0].minor);
}
// 2466 "parser.c"
        break;
      case 88:
// 995 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_ARRAY, kkmsp[-1].minor.kk92, NULL, NULL, status->scanner_state);
  kk_destructor(6,&kkmsp[-2].minor);
  kk_destructor(65,&kkmsp[0].minor);
}
// 2475 "parser.c"
        break;
      case 89:
// 999 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_ARRAYACCESS, kkmsp[-3].minor.kk92, kkmsp[-1].minor.kk92, NULL, status->scanner_state);
  kk_destructor(6,&kkmsp[-2].minor);
  kk_destructor(65,&kkmsp[0].minor);
}
// 2484 "parser.c"
        break;
      case 90:
// 1003 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_expr(PHVOLT_T_TERNARY, kkmsp[-2].minor.kk92, kkmsp[0].minor.kk92, kkmsp[-4].minor.kk92, status->scanner_state);
  kk_destructor(4,&kkmsp[-3].minor);
  kk_destructor(5,&kkmsp[-1].minor);
}
// 2493 "parser.c"
        break;
      case 91:
// 1007 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_slice(kkmsp[-4].minor.kk92, NULL, kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(6,&kkmsp[-3].minor);
  kk_destructor(5,&kkmsp[-2].minor);
  kk_destructor(65,&kkmsp[0].minor);
}
// 2503 "parser.c"
        break;
      case 92:
// 1011 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_slice(kkmsp[-4].minor.kk92, kkmsp[-2].minor.kk92, NULL, status->scanner_state);
  kk_destructor(6,&kkmsp[-3].minor);
  kk_destructor(5,&kkmsp[-1].minor);
  kk_destructor(65,&kkmsp[0].minor);
}
// 2513 "parser.c"
        break;
      case 93:
// 1015 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_slice(kkmsp[-5].minor.kk92, kkmsp[-3].minor.kk92, kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(6,&kkmsp[-4].minor);
  kk_destructor(5,&kkmsp[-2].minor);
  kk_destructor(65,&kkmsp[0].minor);
}
// 2523 "parser.c"
        break;
      case 95:
      case 107:
// 1025 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_literal_zval(PHVOLT_T_IDENTIFIER, kkmsp[0].minor.kk0, status->scanner_state);
}
// 2531 "parser.c"
        break;
      case 98:
      case 106:
// 1041 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_named_item(kkmsp[-2].minor.kk0, kkmsp[0].minor.kk92, status->scanner_state);
  kk_destructor(5,&kkmsp[-1].minor);
}
// 2540 "parser.c"
        break;
      case 99:
      case 105:
// 1045 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_named_item(NULL, kkmsp[0].minor.kk92, status->scanner_state);
}
// 2548 "parser.c"
        break;
      case 101:
// 1055 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_func_call(kkmsp[-3].minor.kk92, kkmsp[-1].minor.kk92, status->scanner_state);
  kk_destructor(27,&kkmsp[-2].minor);
  kk_destructor(41,&kkmsp[0].minor);
}
// 2557 "parser.c"
        break;
      case 102:
// 1059 "parser.lemon"
{
	kkgotominor.kk92 = phvolt_ret_func_call(kkmsp[-2].minor.kk92, NULL, status->scanner_state);
  kk_destructor(27,&kkmsp[-1].minor);
  kk_destructor(41,&kkmsp[0].minor);
}
// 2566 "parser.c"
        break;
  };
  kkgoto = kkRuleInfo[kkruleno].lhs;
  kksize = kkRuleInfo[kkruleno].nrhs;
  kkpParser->kkidx -= kksize;
  kkact = kk_find_reduce_action(kkpParser,kkgoto);
  if( kkact < KKNSTATE ){
    kk_shift(kkpParser,kkact,kkgoto,&kkgotominor);
  }else if( kkact == KKNSTATE + KKNRULE + 1 ){
    kk_accept(kkpParser);
  }
}

/*
** The following code executes when the parse fails
*/
static void kk_parse_failed(
  kkParser *kkpParser           /* The parser */
){
  phvolt_ARG_FETCH;
#ifndef NDEBUG
  if( kkTraceFILE ){
    fprintf(kkTraceFILE,"%sFail!\n",kkTracePrompt);
  }
#endif
  while( kkpParser->kkidx>=0 ) kk_pop_parser_stack(kkpParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following code executes when a syntax error first occurs.
*/
static void kk_syntax_error(
  kkParser *kkpParser,           /* The parser */
  int kkmajor,                   /* The major type of the error token */
  KKMINORTYPE kkminor            /* The minor type of the error token */
){
  phvolt_ARG_FETCH;
#define KTOKEN (kkminor.kk0)
// 542 "parser.lemon"

	if (status->scanner_state->start_length) {
		{

			char *token_name = NULL;
			const phvolt_token_names *tokens = phvolt_tokens;
			int token_found = 0;
			int active_token = status->scanner_state->active_token;

			if (active_token) {

				do {
					if (tokens->code == active_token) {
						token_found = 1;
						token_name = tokens->name;
						break;
					}
					++tokens;
				} while (tokens[0].code != 0);

			}

			if (!token_name) {
				token_found = 0;
				token_name = estrndup("UNKNOWN", strlen("UNKNOWN"));
			}

			status->syntax_error_len = 64 + strlen(token_name) + Z_STRLEN_P(status->scanner_state->active_file);
			status->syntax_error = emalloc(sizeof(char) * status->syntax_error_len);
			snprintf(status->syntax_error, status->syntax_error_len - 1, "Syntax error, unexpected token %s in %s on line %d", token_name, Z_STRVAL_P(status->scanner_state->active_file), status->scanner_state->active_line);
			status->syntax_error[status->syntax_error_len - 1] = '\0';

			if (!token_found) {
				if (token_name) {
					efree(token_name);
				}
			}
		}
	} else {
		status->syntax_error_len = 48 + Z_STRLEN_P(status->scanner_state->active_file);
		status->syntax_error = emalloc(sizeof(char) * status->syntax_error_len);
		snprintf(status->syntax_error, status->syntax_error_len - 1, "Syntax error, unexpected EOF in %s", Z_STRVAL_P(status->scanner_state->active_file));
		status->syntax_error[status->syntax_error_len - 1] = '\0';
	}

	status->status = PHVOLT_PARSING_FAILED;

// 2656 "parser.c"
  phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void kk_accept(
  kkParser *kkpParser           /* The parser */
){
  phvolt_ARG_FETCH;
#ifndef NDEBUG
  if( kkTraceFILE ){
    fprintf(kkTraceFILE,"%sAccept!\n",kkTracePrompt);
  }
#endif
  while( kkpParser->kkidx>=0 ) kk_pop_parser_stack(kkpParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  phvolt_ARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "phvolt_Alloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
void phvolt_(
  void *kkp,                   /* The parser */
  int kkmajor,                 /* The major token code number */
  phvolt_KTOKENTYPE kkminor       /* The value for the token */
  phvolt_ARG_PDECL               /* Optional %extra_argument parameter */
){
  KKMINORTYPE kkminorunion;
  int kkact;            /* The parser action. */
  int kkendofinput;     /* True if we are at the end of input */
  int kkerrorhit = 0;   /* True if kkmajor has invoked an error */
  kkParser *kkpParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  kkpParser = (kkParser*)kkp;
  if( kkpParser->kkidx<0 ){
    if( kkmajor==0 ) return;
    kkpParser->kkidx = 0;
    kkpParser->kkerrcnt = -1;
    kkpParser->kkstack[0].stateno = 0;
    kkpParser->kkstack[0].major = 0;
  }
  kkminorunion.kk0 = kkminor;
  kkendofinput = (kkmajor==0);
  phvolt_ARG_STORE;

#ifndef NDEBUG
  if( kkTraceFILE ){
    fprintf(kkTraceFILE,"%sInput %s\n",kkTracePrompt,kkTokenName[kkmajor]);
  }
#endif

  do{
    kkact = kk_find_shift_action(kkpParser,kkmajor);
    if( kkact<KKNSTATE ){
      kk_shift(kkpParser,kkact,kkmajor,&kkminorunion);
      kkpParser->kkerrcnt--;
      if( kkendofinput && kkpParser->kkidx>=0 ){
        kkmajor = 0;
      }else{
        kkmajor = KKNOCODE;
      }
    }else if( kkact < KKNSTATE + KKNRULE ){
      kk_reduce(kkpParser,kkact-KKNSTATE);
    }else if( kkact == KK_ERROR_ACTION ){
      int kkmx;
#ifndef NDEBUG
      if( kkTraceFILE ){
        fprintf(kkTraceFILE,"%sSyntax Error!\n",kkTracePrompt);
      }
#endif
#ifdef KKERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( kkpParser->kkerrcnt<0 ){
        kk_syntax_error(kkpParser,kkmajor,kkminorunion);
      }
      kkmx = kkpParser->kkstack[kkpParser->kkidx].major;
      if( kkmx==KKERRORSYMBOL || kkerrorhit ){
#ifndef NDEBUG
        if( kkTraceFILE ){
          fprintf(kkTraceFILE,"%sDiscard input token %s\n",
             kkTracePrompt,kkTokenName[kkmajor]);
        }
#endif
        kk_destructor(kkmajor,&kkminorunion);
        kkmajor = KKNOCODE;
      }else{
         while(
          kkpParser->kkidx >= 0 &&
          kkmx != KKERRORSYMBOL &&
          (kkact = kk_find_shift_action(kkpParser,KKERRORSYMBOL)) >= KKNSTATE
        ){
          kk_pop_parser_stack(kkpParser);
        }
        if( kkpParser->kkidx < 0 || kkmajor==0 ){
          kk_destructor(kkmajor,&kkminorunion);
          kk_parse_failed(kkpParser);
          kkmajor = KKNOCODE;
        }else if( kkmx!=KKERRORSYMBOL ){
          KKMINORTYPE u2;
          u2.KKERRSYMDT = 0;
          kk_shift(kkpParser,kkact,KKERRORSYMBOL,&u2);
        }
      }
      kkpParser->kkerrcnt = 3;
      kkerrorhit = 1;
#else  /* KKERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( kkpParser->kkerrcnt<=0 ){
        kk_syntax_error(kkpParser,kkmajor,kkminorunion);
      }
      kkpParser->kkerrcnt = 3;
      kk_destructor(kkmajor,&kkminorunion);
      if( kkendofinput ){
        kk_parse_failed(kkpParser);
      }
      kkmajor = KKNOCODE;
#endif
    }else{
      kk_accept(kkpParser);
      kkmajor = KKNOCODE;
    }
  }while( kkmajor!=KKNOCODE && kkpParser->kkidx>=0 );
  return;
}

/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

const phvolt_token_names phvolt_tokens[] =
{
  { PHVOLT_T_INTEGER,       	"INTEGER" },
  { PHVOLT_T_DOUBLE,        	"DOUBLE" },
  { PHVOLT_T_STRING,        	"STRING" },
  { PHVOLT_T_IDENTIFIER,    	"IDENTIFIER" },
  { PHVOLT_T_MINUS,         	"MINUS" },
  { PHVOLT_T_ADD,           	"+" },
  { PHVOLT_T_SUB,           	"-" },
  { PHVOLT_T_MUL,           	"*" },
  { PHVOLT_T_DIV,           	"/" },
  { PHVOLT_T_MOD,           	"%%" },
  { PHVOLT_T_NOT,           	"!" },
  { PHVOLT_T_CONCAT,           	"~" },
  { PHVOLT_T_AND,           	"AND" },
  { PHVOLT_T_OR,            	"OR" },
  { PHVOLT_T_DOT,           	"DOT" },
  { PHVOLT_T_COMMA,         	"COMMA" },
  { PHVOLT_T_EQUALS,        	"EQUALS" },
  { PHVOLT_T_NOTEQUALS,     	"NOT EQUALS" },
  { PHVOLT_T_IDENTICAL,     	"IDENTICAL" },
  { PHVOLT_T_NOTIDENTICAL,  	"NOT IDENTICAL" },
  { PHVOLT_T_NOT,           	"NOT" },
  { PHVOLT_T_RANGE,           	"RANGE" },
  { PHVOLT_T_COLON,       		"COLON" },
  { PHVOLT_T_QUESTION,       	"QUESTION MARK" },
  { PHVOLT_T_LESS,          	"<" },
  { PHVOLT_T_LESSEQUAL,     	"<=" },
  { PHVOLT_T_GREATER,       	">" },
  { PHVOLT_T_GREATEREQUAL,  	">=" },
  { PHVOLT_T_PARENTHESES_OPEN,  "(" },
  { PHVOLT_T_PARENTHESES_CLOSE, ")" },
  { PHVOLT_T_SBRACKET_OPEN,  	"[" },
  { PHVOLT_T_SBRACKET_CLOSE, 	"]" },
  { PHVOLT_T_OPEN_DELIMITER, 	"{%" },
  { PHVOLT_T_CLOSE_DELIMITER, 	"%}" },
  { PHVOLT_T_OPEN_EDELIMITER, 	"{{" },
  { PHVOLT_T_CLOSE_EDELIMITER, 	"}}" },
  { PHVOLT_T_IF,           		"IF" },
  { PHVOLT_T_ELSE,           	"ELSE" },
  { PHVOLT_T_ELSEIF,           	"ELSEIF" },
  { PHVOLT_T_ELSEFOR,           "ELSEFOR" },
  { PHVOLT_T_ENDIF,           	"ENDIF" },
  { PHVOLT_T_FOR,           	"FOR" },
  { PHVOLT_T_IN, 	          	"IN" },
  { PHVOLT_T_ENDFOR,           	"ENDFOR" },
  { PHVOLT_T_SET,           	"SET" },
  { PHVOLT_T_ASSIGN,           	"ASSIGN" },
  { PHVOLT_T_BLOCK,           	"BLOCK" },
  { PHVOLT_T_ENDBLOCK,          "ENDBLOCK" },
  { PHVOLT_T_CACHE,           	"CACHE" },
  { PHVOLT_T_ENDCACHE,        	"ENDCACHE" },
  { PHVOLT_T_EXTENDS,			"EXTENDS" },
  { PHVOLT_T_IS,				"IS" },
  { PHVOLT_T_DEFINED,			"DEFINED" },
  { PHVOLT_T_INCLUDE,			"INCLUDE" },
  { PHVOLT_T_DO,				"DO" },
  { PHVOLT_T_IGNORE,			"WHITESPACE" },
  { PHVOLT_T_AUTOESCAPE,		"AUTOESCAPE" },
  { PHVOLT_T_ENDAUTOESCAPE,		"ENDAUTOESCAPE" },
  { PHVOLT_T_CONTINUE,			"CONTINUE" },
  { PHVOLT_T_BREAK,				"BREAK" },
  { PHVOLT_T_WITH,				"WITH" },
  {  0, NULL }
};

/**
 * Wrapper to alloc memory within the parser
 */
static void *phvolt_wrapper_alloc(size_t bytes){
	return emalloc(bytes);
}

/**
 * Wrapper to free memory within the parser
 */
static void phvolt_wrapper_free(void *pointer){
	efree(pointer);
}

/**
 * Creates a parser_token to be passed to the parser
 */
static void phvolt_parse_with_token(void* phvolt_parser, int opcode, int parsercode, phvolt_scanner_token *token, phvolt_parser_status *parser_status){

	phvolt_parser_token *pToken;

	pToken = emalloc(sizeof(phvolt_parser_token));
	pToken->opcode = opcode;
	pToken->token = token->value;
	pToken->token_len = token->len;
	pToken->free_flag = 1;

	phvolt_(phvolt_parser, parsercode, pToken, parser_status);
}

/**
 * Creates an error message
 */
static void phvolt_create_error_msg(phvolt_parser_status *parser_status, char *message){

	unsigned int length = (128 + Z_STRLEN_P(parser_status->scanner_state->active_file));
	char *str = emalloc(sizeof(char) * length);

	snprintf(str, length, "%s in %s on line %d", message, Z_STRVAL_P(parser_status->scanner_state->active_file), parser_status->scanner_state->active_line);
	str[length - 1] = '\0';

	parser_status->syntax_error = estrndup(str, strlen(str));
	efree(str);
}

/**
 * Creates an error message when it's triggered by the scanner
 */
static void phvolt_scanner_error_msg(phvolt_parser_status *parser_status, zval **error_msg TSRMLS_DC){

	char *error, *error_part;
	int length;
	phvolt_scanner_state *state = parser_status->scanner_state;

	PHALCON_INIT_VAR(*error_msg);
	if (state->start) {
		error = emalloc(sizeof(char) * 72 + state->start_length +  Z_STRLEN_P(state->active_file));
		if (state->start_length > 16) {
			length = 72 + Z_STRLEN_P(state->active_file);
			error_part = estrndup(state->start, 16);
			snprintf(error, length, "Parsing error before '%s...' in %s on line %d", error_part, Z_STRVAL_P(state->active_file), state->active_line);
			efree(error_part);
		} else {
			length = 48 + state->start_length + Z_STRLEN_P(state->active_file);
			snprintf(error, length, "Parsing error before '%s' in %s on line %d", state->start, Z_STRVAL_P(state->active_file), state->active_line);
		}
	} else {
		error = emalloc(sizeof(char) * (32 + Z_STRLEN_P(state->active_file)));
		length = 32 + Z_STRLEN_P(state->active_file);
		snprintf(error, length, "Parsing error near to EOF in %s", Z_STRVAL_P(state->active_file));
	}

	error[length - 1] = '\0';
	ZVAL_STRING(*error_msg, error, 1);
	efree(error);
}

/**
 * Receives the volt code tokenizes and parses it
 */
int phvolt_parse_view(zval *result, zval *view_code, zval *template_path TSRMLS_DC){

	zval *error_msg = NULL;

	ZVAL_NULL(result);

	if (Z_TYPE_P(view_code) != IS_STRING) {
		phalcon_throw_exception_string(phalcon_mvc_view_exception_ce, SL("View code must be a string") TSRMLS_CC);
		return FAILURE;
	}

	if (phvolt_internal_parse_view(&result, view_code, template_path, &error_msg TSRMLS_CC) == FAILURE) {
		phalcon_throw_exception_string(phalcon_mvc_view_exception_ce, Z_STRVAL_P(error_msg), Z_STRLEN_P(error_msg) TSRMLS_CC);
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Checks whether the token has only blank characters
 */
int phvolt_is_blank_string(phvolt_scanner_token *token){

	char *marker = token->value;
	unsigned int ch, i;

	for (i = 0; i < token->len; i++) {
		ch = *marker;
		if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != 11) {
			return 0;
		}
		marker++;
	}

	return 1;
}

/**
 * Parses a volt template returning an intermediate array representation
 */
int phvolt_internal_parse_view(zval **result, zval *view_code, zval *template_path, zval **error_msg TSRMLS_DC) {

	char *error;
	phvolt_scanner_state *state;
	phvolt_scanner_token token;
	int scanner_status, status = SUCCESS;
	phvolt_parser_status *parser_status = NULL;
	void* phvolt_parser;

	/** Check if the view has code */
	if (!Z_STRVAL_P(view_code)) {
		PHALCON_INIT_VAR(*error_msg);
		ZVAL_STRING(*error_msg, "View code cannot be null", 1);
		return FAILURE;
	}

	if (!Z_STRLEN_P(view_code)) {
		array_init(*result);
		return SUCCESS;
	}

	/** Start the reentrant parser */
	phvolt_parser = phvolt_Alloc(phvolt_wrapper_alloc);

	parser_status = emalloc(sizeof(phvolt_parser_status));
	state = emalloc(sizeof(phvolt_scanner_state));

	parser_status->status = PHVOLT_PARSING_OK;
	parser_status->scanner_state = state;
	parser_status->ret = NULL;
	parser_status->syntax_error = NULL;

	/** Initialize the scanner state */
	state->active_token = 0;
	state->start = Z_STRVAL_P(view_code);
	state->mode = PHVOLT_MODE_RAW;
	state->raw_buffer = emalloc(sizeof(char) * PHVOLT_RAW_BUFFER_SIZE);
	state->raw_buffer_size = PHVOLT_RAW_BUFFER_SIZE;
	state->raw_buffer_cursor = 0;
	state->active_file = template_path;
	state->active_line = 1;
	state->statement_position = 0;
	state->extends_mode = 0;
	state->block_level = 0;
	state->start_length = 0;
	state->old_if_level = 0;
	state->if_level = 0;
	state->for_level = 0;
	state->whitespace_control = 0;

	state->end = state->start;

	while(0 <= (scanner_status = phvolt_get_token(state, &token))) {

		state->active_token = token.opcode;

		state->start_length = (Z_STRVAL_P(view_code) + Z_STRLEN_P(view_code) - state->start);

		switch(token.opcode){

			case PHVOLT_T_IGNORE:
				break;

			case PHVOLT_T_ADD:
				phvolt_(phvolt_parser, PHVOLT_PLUS, NULL, parser_status);
				break;
			case PHVOLT_T_SUB:
				phvolt_(phvolt_parser, PHVOLT_MINUS, NULL, parser_status);
				break;
			case PHVOLT_T_MUL:
				phvolt_(phvolt_parser, PHVOLT_TIMES, NULL, parser_status);
				break;
			case PHVOLT_T_DIV:
				phvolt_(phvolt_parser, PHVOLT_DIVIDE, NULL, parser_status);
				break;
			case PHVOLT_T_MOD:
				phvolt_(phvolt_parser, PHVOLT_MOD, NULL, parser_status);
				break;
			case PHVOLT_T_AND:
				phvolt_(phvolt_parser, PHVOLT_AND, NULL, parser_status);
				break;
			case PHVOLT_T_OR:
				phvolt_(phvolt_parser, PHVOLT_OR, NULL, parser_status);
				break;
			case PHVOLT_T_IS:
				phvolt_(phvolt_parser, PHVOLT_IS, NULL, parser_status);
				break;
			case PHVOLT_T_EQUALS:
				phvolt_(phvolt_parser, PHVOLT_EQUALS, NULL, parser_status);
				break;
			case PHVOLT_T_NOTEQUALS:
				phvolt_(phvolt_parser, PHVOLT_NOTEQUALS, NULL, parser_status);
				break;
			case PHVOLT_T_LESS:
				phvolt_(phvolt_parser, PHVOLT_LESS, NULL, parser_status);
				break;
			case PHVOLT_T_GREATER:
				phvolt_(phvolt_parser, PHVOLT_GREATER, NULL, parser_status);
				break;
			case PHVOLT_T_GREATEREQUAL:
				phvolt_(phvolt_parser, PHVOLT_GREATEREQUAL, NULL, parser_status);
				break;
			case PHVOLT_T_LESSEQUAL:
				phvolt_(phvolt_parser, PHVOLT_LESSEQUAL, NULL, parser_status);
				break;
			case PHVOLT_T_IDENTICAL:
				phvolt_(phvolt_parser, PHVOLT_IDENTICAL, NULL, parser_status);
				break;
			case PHVOLT_T_NOTIDENTICAL:
				phvolt_(phvolt_parser, PHVOLT_NOTIDENTICAL, NULL, parser_status);
				break;
			case PHVOLT_T_NOT:
				phvolt_(phvolt_parser, PHVOLT_NOT, NULL, parser_status);
				break;
			case PHVOLT_T_DOT:
				phvolt_(phvolt_parser, PHVOLT_DOT, NULL, parser_status);
				break;
			case PHVOLT_T_CONCAT:
				phvolt_(phvolt_parser, PHVOLT_CONCAT, NULL, parser_status);
				break;
			case PHVOLT_T_RANGE:
				phvolt_(phvolt_parser, PHVOLT_RANGE, NULL, parser_status);
				break;
			case PHVOLT_T_PIPE:
				phvolt_(phvolt_parser, PHVOLT_PIPE, NULL, parser_status);
				break;
			case PHVOLT_T_COMMA:
				phvolt_(phvolt_parser, PHVOLT_COMMA, NULL, parser_status);
				break;
			case PHVOLT_T_COLON:
				phvolt_(phvolt_parser, PHVOLT_COLON, NULL, parser_status);
				break;
			case PHVOLT_T_QUESTION:
				phvolt_(phvolt_parser, PHVOLT_QUESTION, NULL, parser_status);
				break;

			case PHVOLT_T_PARENTHESES_OPEN:
				phvolt_(phvolt_parser, PHVOLT_PARENTHESES_OPEN, NULL, parser_status);
				break;
			case PHVOLT_T_PARENTHESES_CLOSE:
				phvolt_(phvolt_parser, PHVOLT_PARENTHESES_CLOSE, NULL, parser_status);
				break;
			case PHVOLT_T_SBRACKET_OPEN:
				phvolt_(phvolt_parser, PHVOLT_SBRACKET_OPEN, NULL, parser_status);
				break;
			case PHVOLT_T_SBRACKET_CLOSE:
				phvolt_(phvolt_parser, PHVOLT_SBRACKET_CLOSE, NULL, parser_status);
				break;

			case PHVOLT_T_OPEN_DELIMITER:
				phvolt_(phvolt_parser, PHVOLT_OPEN_DELIMITER, NULL, parser_status);
				break;
			case PHVOLT_T_CLOSE_DELIMITER:
				phvolt_(phvolt_parser, PHVOLT_CLOSE_DELIMITER, NULL, parser_status);
				break;

			case PHVOLT_T_OPEN_EDELIMITER:
				if (state->extends_mode == 1 && state->block_level == 0) {
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				}
				phvolt_(phvolt_parser, PHVOLT_OPEN_EDELIMITER, NULL, parser_status);
				break;
			case PHVOLT_T_CLOSE_EDELIMITER:
				phvolt_(phvolt_parser, PHVOLT_CLOSE_EDELIMITER, NULL, parser_status);
				break;

			case PHVOLT_T_NULL:
				phvolt_(phvolt_parser, PHVOLT_NULL, NULL, parser_status);
				break;
			case PHVOLT_T_TRUE:
				phvolt_(phvolt_parser, PHVOLT_TRUE, NULL, parser_status);
				break;
			case PHVOLT_T_FALSE:
				phvolt_(phvolt_parser, PHVOLT_FALSE, NULL, parser_status);
				break;

			case PHVOLT_T_INTEGER:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_INTEGER, PHVOLT_INTEGER, &token, parser_status);
				break;
			case PHVOLT_T_DOUBLE:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_DOUBLE, PHVOLT_DOUBLE, &token, parser_status);
				break;
			case PHVOLT_T_STRING:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_STRING, PHVOLT_STRING, &token, parser_status);
				break;
			case PHVOLT_T_IDENTIFIER:
				phvolt_parse_with_token(phvolt_parser, PHVOLT_T_IDENTIFIER, PHVOLT_IDENTIFIER, &token, parser_status);
				break;

			case PHVOLT_T_IF:
				if (state->extends_mode == 1 && state->block_level == 0){
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->if_level++;
					state->block_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_IF, NULL, parser_status);
				break;

			case PHVOLT_T_ELSE:
				if (state->if_level == 0 && state->for_level > 0) {
					phvolt_(phvolt_parser, PHVOLT_ELSEFOR, NULL, parser_status);
				} else {
					phvolt_(phvolt_parser, PHVOLT_ELSE, NULL, parser_status);
				}
				break;

			case PHVOLT_T_ELSEFOR:
				phvolt_(phvolt_parser, PHVOLT_ELSEFOR, NULL, parser_status);
				break;

			case PHVOLT_T_ELSEIF:
				if (state->if_level == 0) {
					phvolt_create_error_msg(parser_status, "Unexpected ENDIF");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				}
				phvolt_(phvolt_parser, PHVOLT_ELSEIF, NULL, parser_status);
				break;

			case PHVOLT_T_ENDIF:
				state->block_level--;
				state->if_level--;
				phvolt_(phvolt_parser, PHVOLT_ENDIF, NULL, parser_status);
				break;

			case PHVOLT_T_FOR:
				if (state->extends_mode == 1 && state->block_level == 0){
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->old_if_level = state->if_level;
					state->if_level = 0;
					state->for_level++;
					state->block_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_FOR, NULL, parser_status);
				break;

			case PHVOLT_T_IN:
				phvolt_(phvolt_parser, PHVOLT_IN, NULL, parser_status);
				break;

			case PHVOLT_T_ENDFOR:
				state->block_level--;
				state->for_level--;
				state->if_level = state->old_if_level;
				phvolt_(phvolt_parser, PHVOLT_ENDFOR, NULL, parser_status);
				break;

			case PHVOLT_T_RAW_FRAGMENT:
				if (token.len > 0) {
					if (state->extends_mode == 1 && state->block_level == 0){
						if(!phvolt_is_blank_string(&token)){
							phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
							parser_status->status = PHVOLT_PARSING_FAILED;
						}
						efree(token.value);
						break;
					} else {
						if (!phvolt_is_blank_string(&token)) {
							state->statement_position++;
						}
					}
					phvolt_parse_with_token(phvolt_parser, PHVOLT_T_RAW_FRAGMENT, PHVOLT_RAW_FRAGMENT, &token, parser_status);
				} else {
					efree(token.value);
				}
				break;

			case PHVOLT_T_SET:
				if (state->extends_mode == 1 && state->block_level == 0){
					phvolt_create_error_msg(parser_status, "Child templates only may contain blocks");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				}
				phvolt_(phvolt_parser, PHVOLT_SET, NULL, parser_status);
				break;
			case PHVOLT_T_ASSIGN:
				phvolt_(phvolt_parser, PHVOLT_ASSIGN, NULL, parser_status);
				break;

			case PHVOLT_T_BLOCK:
				if (state->block_level > 0) {
					phvolt_create_error_msg(parser_status, "Embedding blocks into other blocks is not supported");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->block_level++;
				}
				phvolt_(phvolt_parser, PHVOLT_BLOCK, NULL, parser_status);
				break;
			case PHVOLT_T_ENDBLOCK:
				state->block_level--;
				phvolt_(phvolt_parser, PHVOLT_ENDBLOCK, NULL, parser_status);
				break;

			case PHVOLT_T_MACRO:
				phvolt_(phvolt_parser, PHVOLT_MACRO, NULL, parser_status);
				break;
			case PHVOLT_T_ENDMACRO:
				phvolt_(phvolt_parser, PHVOLT_ENDMACRO, NULL, parser_status);
				break;

			case PHVOLT_T_CACHE:
				phvolt_(phvolt_parser, PHVOLT_CACHE, NULL, parser_status);
				break;
			case PHVOLT_T_ENDCACHE:
				phvolt_(phvolt_parser, PHVOLT_ENDCACHE, NULL, parser_status);
				break;

			case PHVOLT_T_INCLUDE:
				phvolt_(phvolt_parser, PHVOLT_INCLUDE, NULL, parser_status);
				break;

			case PHVOLT_T_WITH:
				phvolt_(phvolt_parser, PHVOLT_WITH, NULL, parser_status);
				break;

			case PHVOLT_T_DEFINED:
				phvolt_(phvolt_parser, PHVOLT_DEFINED, NULL, parser_status);
				break;

			case PHVOLT_T_DO:
				phvolt_(phvolt_parser, PHVOLT_DO, NULL, parser_status);
				break;

			case PHVOLT_T_AUTOESCAPE:
				phvolt_(phvolt_parser, PHVOLT_AUTOESCAPE, NULL, parser_status);
				break;

			case PHVOLT_T_ENDAUTOESCAPE:
				phvolt_(phvolt_parser, PHVOLT_ENDAUTOESCAPE, NULL, parser_status);
				break;

			case PHVOLT_T_BREAK:
				phvolt_(phvolt_parser, PHVOLT_BREAK, NULL, parser_status);
				break;

			case PHVOLT_T_CONTINUE:
				phvolt_(phvolt_parser, PHVOLT_CONTINUE, NULL, parser_status);
				break;

			case PHVOLT_T_EXTENDS:
				if (state->statement_position != 1) {
					phvolt_create_error_msg(parser_status, "Extends statement must be placed at the first line in the template");
					parser_status->status = PHVOLT_PARSING_FAILED;
					break;
				} else {
					state->extends_mode = 1;
				}
				phvolt_(phvolt_parser, PHVOLT_EXTENDS, NULL, parser_status);
				break;

			default:
				parser_status->status = PHVOLT_PARSING_FAILED;
				if (!*error_msg) {
					error = emalloc(sizeof(char) * (48 + Z_STRLEN_P(state->active_file)));
					snprintf(error, 48 + Z_STRLEN_P(state->active_file) + state->active_line, "Scanner: unknown opcode %d on in %s line %d", token.opcode, Z_STRVAL_P(state->active_file), state->active_line);
					PHALCON_INIT_VAR(*error_msg);
					ZVAL_STRING(*error_msg, error, 1);
					efree(error);
				}
				break;
		}

		if (parser_status->status != PHVOLT_PARSING_OK) {
			status = FAILURE;
			break;
		}

		state->end = state->start;
	}

	if (status != FAILURE) {
		switch (scanner_status) {
			case PHVOLT_SCANNER_RETCODE_ERR:
			case PHVOLT_SCANNER_RETCODE_IMPOSSIBLE:
				if (!*error_msg) {
					phvolt_scanner_error_msg(parser_status, error_msg TSRMLS_CC);
				}
				status = FAILURE;
				break;
			default:
				phvolt_(phvolt_parser, 0, NULL, parser_status);
		}
	}

	state->active_token = 0;
	state->start = NULL;
	efree(state->raw_buffer);

	if (parser_status->status != PHVOLT_PARSING_OK) {
		status = FAILURE;
		if (parser_status->syntax_error) {
			if (!*error_msg) {
				PHALCON_INIT_VAR(*error_msg);
				ZVAL_STRING(*error_msg, parser_status->syntax_error, 1);
			}
			efree(parser_status->syntax_error);
		}
	}

	phvolt_Free(phvolt_parser, phvolt_wrapper_free);

	if (status != FAILURE) {
		if (parser_status->status == PHVOLT_PARSING_OK) {
			if (parser_status->ret) {
				ZVAL_ZVAL(*result, parser_status->ret, 0, 0);
				ZVAL_NULL(parser_status->ret);
				zval_ptr_dtor(&parser_status->ret);
			} else {
				array_init(*result);
			}
		}
	}

	efree(parser_status);
	efree(state);

	return status;
}