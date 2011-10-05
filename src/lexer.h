/*
 * lexer.h
 *
 *  Created on: 2011-10-03
 *      Author: plaguemorin
 */

#ifndef LEXER_H_
#define LEXER_H_

#include "filesystem.h"

void LE_init(filehandle_t *textFile);
void LE_skipWhiteSpace(void);
void LE_SkipRestOfLine(void);
char* LE_readToken(void);
char* LE_getCurrentToken();
float LE_readReal(void);
int LE_hasMoreData(void);
void LE_SetWhiteCharValue(char c, char value);

void LE_popLexer();
void LE_pushLexer();

#endif /* LEXER_H_ */
