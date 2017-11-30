// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pl0.h"
#include "set.h"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

  //////////////////////////////////////////////////////////////////////
  /*getch()函数从源程序获取字符*/
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

  //////////////////////////////////////////////////////////////////////
  // gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];//作为满足条件的字符串的最长前缀的临时存储

						 //跳过空格和制表符
	while (ch == ' ' || ch == '\t')
		getch();

	/*若首部为字母则进入标识符及保留字的判断流程*/
	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	/*若首部为数字则进入是否为数字的判断流程*/
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;//k记录数字的位数
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == '=')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_EQU; // ==
			getch();
		}
		else
		{
			sym = SYM_BECOMES;       // =
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else  if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;//&&
			getch();
		}
		else
		{
			sym = SYM_BAND;//&
		}
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		else
		{
			sym = SYM_BOR;//|
		}
	}
	else if (ch == '/')
	{
		getch();
		if(ch=='/')//"//"
		{
			while (cc != ll)
			{
				getch();
			}
			getch();
			getsym();
		}
		else if (ch == '*')
		{
			while (1)
			{
				getch();
				if (ch == '*')
				{
					getch();
					if (ch == '/')
					{
						break;
					}
				}
			}
			getch();
			getsym();
		}
		else
		{
			sym = SYM_SLASH;// '/'
		}
	}
	/*判断字符是否为保存于csym中的无需超前搜索即可判断的运算符*/
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else//若上述要求当前字符均不符合，则可判定为非法字符
		{
			printf("Fatal Error: Unknown character.\n");
			//exit(1);
		}
	}
} // getsym

  //////////////////////////////////////////////////////////////////////
  // generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

  //////////////////////////////////////////////////////////////////////
  // tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;
	//sym doesn't belongs to s1
	if (!inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while (!inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

  //////////////////////////////////////////////////////////////////////
int dx;  // data allocation index
int formal=0;//the number of formal parameter

 // enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT://常数
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE://变量
		mk = (mask*)&table[tx];
		mk->level = level;//变量所在的层次
		mk->address = dx++;
		break;
	case ID_PROCEDURE://过程
		mk = (mask*)&table[tx];
		mk->level = level;
		break;
	case ID_DEFAULTPRO://内建方法
		mk = (mask*)&table[tx];
		mk->level = 0;
		mk->config[0] = fun_code;
		mk->config[2] = formal_para;
		break;
	case ID_ARRAY://数组
		mk = (mask*)&table[tx];
		mk->level = level;
		mk->address = dx;
		break;
	} // switch
} // enter

  //////////////////////////////////////////////////////////////////////
void DEFAULTPRO()
{
	mask* mk;
	strcpy(id, "print");
	fun_code = FUN_PRINT;
	formal_para = 0;
	enter(ID_DEFAULTPRO);
	mk = (mask*)&table[tx];
	mk->config[1] = 0;//表示参数位无效
	strcpy(id, "random");
	fun_code = FUN_RANDOM;
	formal_para = 0;
	enter(ID_DEFAULTPRO);
	mk = (mask*)&table[tx];
	mk->config[1] = 1;//表示参数位有效
	strcpy(id, "callstack");
	fun_code = FUN_CALLSTACK;
	formal_para = 0;
	enter(ID_DEFAULTPRO);
	mk = (mask*)&table[tx];
	mk->config[1] = 1;
	strcpy(id, "input");
	fun_code = FUN_INPUT;
	formal_para = 0;
	enter(ID_DEFAULTPRO);
	mk = (mask*)&table[tx];
	mk->config[1] = 0;//表示参数位无效
	formal_para = 0;
}

  //////////////////////////////////////////////////////////////////////
  // locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

  //////////////////////////////////////////////////////////////////////
void constdeclaration()//const赋值语句
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)//=||==
		{
			if (sym == SYM_EQU)
				error(1); // Found '==' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	}
	else	error(4);
	// There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

  //////////////////////////////////////////////////////////////////////
void vardeclaration(void)//变量声明语句
{
	mask* mk;
	int i = 1;
	int j , k;
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if(sym==SYM_LSQUARE)//"["
		{
			enter(ID_ARRAY);
			mk = (mask*)&table[tx];
			while (sym == SYM_LSQUARE)
			{
				getsym();
				if (sym == SYM_NUMBER)
				{
					mk->config[i] = num;
					i++;
					getsym();
					if (sym == SYM_RSQUARE)
					{
						getsym();
					}
					else
					{
						error(29);//Missing ']'
					}
				}
				else if (sym == SYM_IDENTIFIER)
				{
					j = position(id);
					if (table[j].kind == ID_CONSTANT)
					{
						mk->config[i] = table[j].value;
						i++;
						getsym();
						if (sym == SYM_RSQUARE)
						{
							getsym();
						}
						else
						{
							error(29);//Missing ']'
						}
					}
					else
					{
						error(28);
					}
				}
				else
				{
					error(28);//Illegal array declarations
				}
			}
			mk->config[0] = i - 1;//store the dim of the array
			mk->config[i] = 1;
			k = 1;
			for (j = 1; j <= i-1; j++)
			{
				k *= mk->config[j];
			}
			dx += k;//allocate the memory for the array
		}
		else
		{
			enter(ID_VARIABLE);
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

  //////////////////////////////////////////////////////////////////////
//将生成的指令列出来
void listcode(int from, int to)
{
	int i;

	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

int true_list[20], false_list[20];
int true_index = 0;
int false_index = 0;
   //////////////////////////////////////////////////////////////////////
//短路运算代码优化
void reversal(int i)
{
	switch (code[i].f)
	{
	case JG:
		code[i].f = JBE;
		break;
	case JB:
		code[i].f = JGE;
		break;
	case JGE:
		code[i].f = JB;
		break;
	case JBE:
		code[i].f = JG;
		break;
	}
}

void code_adjust()
{
	int i;
	if (true_list[true_index - 1] == cx - 1)
	{
		true_list[--true_index] = 0;
		cx--;
	}
	else if (true_list[true_index - 1] == cx - 2)
	{
		reversal(cx - 2);
		true_list[--true_index] = 0;
		false_list[false_index - 1] = cx - 2;
		cx--;
	}
}


  //////////////////////////////////////////////////////////////////////
//项的判定
void factor(symset fsys)
{
	void expression(symset fsys);
	int i,offset=0;
	int parameter = 0;
	symset set;
	mask* mk2;
	int temp[20];

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
            //符号表中未出现
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					getsym();
					break;
				case ID_VARIABLE:
					mk = (mask*)&table[i];
					gen(LOD, level - mk->level, mk->address);
					getsym();
					break;
				case ID_PROCEDURE:
					mk = (mask*)&table[i];
					getsym();
					if(sym==SYM_LPAREN)
					{
						getsym();
						while (sym == SYM_IDENTIFIER||sym == SYM_NUMBER)
						{
							set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
							expression(set);
							parameter++;
							if (sym == SYM_COMMA)//','
							{
								getsym();
							}
						}
						if (sym == SYM_RPAREN)//')'
						{
							mk = (mask*)&table[i];
							gen(CAL, level - mk->level, mk->address);
							getsym();
						}
						else
						{
							error(22);//Missing ')'.
						}
						if (parameter != mk->config[0])
						{
							error(27);//"The number of the parameter of the procedure is wrong."
						}
					}
					else
					{
						error(21);//"Procedure identifier can not be in an expression without bracket pair."
					}
					break;
				case ID_DEFAULTPRO:
					mk = (mask*)&table[i];
					getsym();
					if (sym == SYM_LPAREN)
					{
						if (strcmp("input", mk->name) != 0)
						{
							getsym();
							while (sym == SYM_IDENTIFIER || sym == SYM_NUMBER)
							{
								set = createset(SYM_COMMA, SYM_RPAREN, SYM_NULL);
								expression(set);
								parameter++;
								if (sym == SYM_COMMA)//','
								{
									getsym();
								}
							}
						}
						else
						{
							getsym();
							while (sym == SYM_IDENTIFIER || sym == SYM_NUMBER)
							{
								if (sym == SYM_NUMBER)
								{
									error(33);
								}
								i = position(id);
								if (table[i].kind == ID_CONSTANT)
								{
									error(33);
								}
								mk2 = (mask*)&table[i];
								gen(LEA, level - mk2->level, mk2->address);
								parameter++;
								getsym();
								if (sym == SYM_COMMA)//','
								{
									getsym();
								}
							}//while
						}
						if (sym == SYM_RPAREN)//')'
						{
							gen(CALL, parameter, mk->config[0]);
							getsym();
						}
						else
						{
							error(22);//Missing ')'.
						}
						if (parameter != mk->config[2] && mk->config[1] == 1)
						{
							error(27);//"The number of the parameter of the procedure is wrong."
						}
					}
					else
					{
						error(21);//"Procedure identifier can not be in an expression without bracket pair."
					}
					break;
				case ID_ARRAY:
					mk = (mask*)&table[i];
					i = 0;
					getsym();
					gen(LIT, 0, 0);
					if (sym == SYM_LSQUARE)
					{
						set = createset(SYM_RSQUARE, SYM_NULL);
						while (sym == SYM_LSQUARE)
						{
							getsym();
							expression(set);
							i++;
							gen(OPR, 0, OPR_ADD);
							gen(LIT, 0, mk->config[i + 1]);
							gen(OPR, 0, OPR_MUL);
							if (sym == SYM_RSQUARE)
							{
								getsym();
							}
							else
							{
								error(29);
							}
						}
						if (i != mk->config[0])
						{
							error(30);//Wrong dims.
						}
						gen(LODAR, level-mk->level, mk->address );
					}
					else
					{
						error(19);//Incorrect symbol.
					}
					break;
				} // switch
			}
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)//'('
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if (sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{
			getsym();
			expression(fsys);
			gen(OPR, 0, OPR_NEG);
		}
		else if (sym == SYM_NEG)//Expr -> '!'Expr
		{
			getsym();
			expression(fsys);
			if (true_index == 0 && false_index == 0)
			{
				true_list[true_index++] = cx;
				gen(JPC, 0, 0);
				false_list[false_index++] = cx;
				gen(JMP, 0, 0);
			}
			else
			{
				for (i = 0; i < true_index; i++)
				{
					temp[i] = true_list[i];
					true_list[i] = 0;
				}
				for (i = 0; i < false_index; i++)
				{
					true_list[i] = false_list[i];
					false_list[i] = 0;
				}
				for (i = 0; i < true_index; i++)
				{
					false_list[i] = temp[i];
				}
				i = true_index;
				true_index = false_index;
				false_index = i;
			}
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
} // factor

  //////////////////////////////////////////////////////////////////////
//判定因子
void term(symset fsys)
{
	int mulop;
	int i;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD,SYM_NULL));//（'*','/','%')
	factor(set);
	if ((true_index != 0 || false_index != 0)&&(sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_MOD))
	{
		code_adjust();
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		gen(LIT, 0, 0);
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx - 3;
			true_list[i] = 0;
		}
		for (i = 0; i < false_index; i++)
		{
			code[false_list[i]].a = cx - 1;
			false_list[i] = 0;
		}
		true_index = false_index = 0;
	}
	while (sym == SYM_TIMES || sym == SYM_SLASH||sym==SYM_MOD)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else if(mulop==SYM_SLASH)
		{
			gen(OPR, 0, OPR_DIV);
		}
		else
		{
			gen(OPR, 0, OPR_MOD);
		}
	} // while
	destroyset(set);
} // term

  //////////////////////////////////////////////////////////////////////
//判定'+','-'表达式
void ADD_exp(symset fsys)
{
	int addop;
	int i;
	symset set;
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));//'+','-'
	term(set);
	if ((true_index != 0 || false_index != 0)&&(sym == SYM_PLUS || sym == SYM_MINUS))
	{
		code_adjust();
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		gen(LIT, 0, 0);
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx - 3;
			true_list[i] = 0;
		}
		for (i = 0; i < false_index; i++)
		{
			code[false_list[i]].a = cx - 1;
			false_list[i] = 0;
		}
		true_index = false_index = 0;
	}
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // ADD_exp

  //////////////////////////////////////////////////////////////////////
  //判定条件语句项
void condition(symset fsys)
{
	int relop;
	int i;
	symset set;
	set = uniteset(fsys, createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_GEQ, SYM_GTR, SYM_LEQ));//'==','<>','>','<','<=','>='
	if (sym == SYM_ODD)
	{
		getsym();
		ADD_exp(fsys);
		gen(OPR, 0, OPR_ODD);
	}
	else
	{
		ADD_exp(set);
		if (inset(sym, relset))
		{
			if (true_index != 0 || false_index != 0)
			{
				code_adjust();
				gen(LIT, 0, 1);
				gen(JMP, 0, cx + 2);
				gen(LIT, 0, 0);
				for (i = 0; i < true_index; i++)
				{
					code[true_list[i]].a = cx - 3;
					true_list[i] = 0;
				}
				for (i = 0; i < false_index; i++)
				{
					code[false_list[i]].a = cx - 1;
					false_list[i] = 0;
				}
				true_index = false_index = 0;
			}
			relop = sym;
			getsym();
			ADD_exp(fsys);
			if (true_index != 0 || false_index != 0)
			{
				code_adjust();
				gen(LIT, 0, 1);
				gen(JMP, 0, cx + 2);
				gen(LIT, 0, 0);
				for (i = 0; i < true_index; i++)
				{
					code[true_list[i]].a = cx - 3;
					true_list[i] = 0;
				}
				for (i = 0; i < false_index; i++)
				{
					code[false_list[i]].a = cx - 1;
					false_list[i] = 0;
				}
				true_index = false_index = 0;
			}
			switch (relop)
			{
			case SYM_EQU:
				gen(JNE, 0, 0);
				false_list[false_index++] = cx - 1;
				gen(JMP, 0, 0);
				true_list[true_index++] = cx - 1;
				break;
			case SYM_NEQ:
				gen(JNE, 0, 0);
				true_list[true_index++] = cx - 1;
				gen(JMP, 0, 0);
				false_list[false_index++] = cx - 1;
				break;
			case SYM_GTR:
				gen(JG, 0, 0);
				true_list[true_index++] = cx - 1;
				gen(JMP, 0, 0);
				false_list[false_index++] = cx - 1;
				break;
			case SYM_LES:
				gen(JB, 0, 0);
				true_list[true_index++] = cx - 1;
				gen(JMP, 0, 0);
				false_list[false_index++] = cx - 1;
				break;
			case SYM_LEQ:
				gen(JG, 0, 0);
				false_list[false_index++] = cx - 1;
				gen(JMP, 0, 0);
				true_list[true_index++] = cx - 1;
				break;
			case SYM_GEQ:
				gen(JB, 0, 0);
				false_list[false_index++] = cx - 1;
				gen(JMP, 0, 0);
				true_list[true_index++] = cx - 1;
				break;
			} // switch
		} // if
	} // else
} // condition

//判断按位与语句
void BIT_AND(symset fsys)
{
	symset set;
	int i;
	set = uniteset(fsys, createset(SYM_BAND, SYM_NULL));
	condition(set);
	if ((true_index != 0 || false_index != 0)&&(sym == SYM_BAND))
	{
		code_adjust();
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		gen(LIT, 0, 0);
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx - 3;
			true_list[i] = 0;
		}
		for (i = 0; i < false_index; i++)
		{
			code[false_list[i]].a = cx - 1;
			false_list[i] = 0;
		}
		true_index = false_index = 0;
	}
	while (sym == SYM_BAND)
	{
		getsym();
		condition(set);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		gen(OPR, 0, OPR_BAND);
	}//while
	destroyset(set);
}//BIT_AND

//判断^语句
void BIT_XOR(symset fsys)
{
	symset set;
	int i;
	set = uniteset(fsys, createset(SYM_BXOR, SYM_NULL));
	BIT_AND(set);
	if ((true_index != 0 || false_index != 0)&&(sym==SYM_BXOR))
	{
		code_adjust();
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		gen(LIT, 0, 0);
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx - 3;
			true_list[i] = 0;
		}
		for (i = 0; i < false_index; i++)
		{
			code[false_list[i]].a = cx - 1;
			false_list[i] = 0;
		}
		true_index = false_index = 0;
	}
	while (sym == SYM_BXOR)
	{
		getsym();
		BIT_AND(set);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		gen(OPR, 0, OPR_BXOR);
	}
	destroyset(set);
}

//判断'|'语句
void BIT_OR(symset fsys)
{
	symset set;
	int i;
	set = uniteset(fsys, createset(SYM_BOR, SYM_NULL));
	BIT_XOR(set);
	if ((true_index != 0 || false_index != 0)&&(sym == SYM_BOR))
	{
		code_adjust();
		gen(LIT, 0, 1);
		gen(JMP, 0, cx + 2);
		gen(LIT, 0, 0);
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx - 3;
			true_list[i] = 0;
		}
		for (i = 0; i < false_index; i++)
		{
			code[false_list[i]].a = cx - 1;
			false_list[i] = 0;
		}
		true_index = false_index = 0;
	}
	while (sym == SYM_BOR)
	{
		getsym();
		BIT_XOR(set);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		gen(OPR, 0, OPR_BOR);
	}
	destroyset(set);
}

//判断条件语句因子&&
void LOGIC_AND(symset fsys)
{
	symset set;
	int temp[20],i,t=0;
	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
	BIT_OR(set);
	if ((true_index == 0 && false_index == 0)&&sym==SYM_AND)
	{
		gen(JPC, 0, 0);
		false_list[false_index++] = cx - 1;
		gen(JMP, 0, 0);
		true_list[true_index++] = cx - 1;
	}
	while (sym==SYM_AND)
	{
		getsym();
		code_adjust();
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx;
			true_list[i] = 0;
		}
		true_index = 0;
		for (i = t; (i - t) < false_index; i++)
		{
			temp[i] = false_list[i-t];
			false_list[i] = 0;
		}
		false_index = 0;
		t = i;
		BIT_OR(set);
		if (true_index == 0 && false_index == 0)
		{
			gen(JPC, 0, 0);
			false_list[false_index++] = cx - 1;
			gen(JMP, 0, 0);
			true_list[true_index++] = cx - 1;
		}
	}//while
	if (t != 0)
	{
		for (i = t; (i - t) < false_index; i++)
		{
			temp[i] = false_list[i-t];
			false_list[i] = 0;
		}
		t = i;
		for (i = 0; i < t; i++)
		{
			false_list[i] = temp[i];
		}
		false_index = t;
	}
	destroyset(set);
}

//判断表达式||
void LOGIC_OR(symset fsys)
{
	symset set;
	int temp[20], i, t = 0;
	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));
	LOGIC_AND(set);
	if ((true_index == 0 && false_index == 0) && sym == SYM_OR)
	{
		gen(JPC, 0, 0);
		false_list[false_index++] = cx - 1;
		gen(JMP, 0, 0);
		true_list[true_index++] = cx - 1;
	}
	while (sym == SYM_OR)//||
	{
		getsym();
		if (false_list[false_index - 1] == cx - 1)
		{
			false_list[--false_index] = 0;
			cx--;
		}
		else if (false_list[false_index - 1] == cx - 2)
		{
			reversal(cx - 2);
			false_list[--false_index] = 0;
			true_list[true_index - 1] = cx - 2;
			cx--;
		}
		for (i = 0; i < false_index; i++)
		{
			code[false_list[i]].a = cx;
			false_list[i] = 0;
		}
		false_index = 0;
		for (i = t; (i - t) < true_index; i++)
		{
			temp[i] = true_list[i-t];
			true_list[i] = 0;
		}
		true_index = 0;
		t = i;
		LOGIC_AND(set);
		if (true_index == 0 && false_index == 0)
		{
			gen(JPC, 0, 0);
			false_list[false_index++] = cx - 1;
			gen(JMP, 0, 0);
			true_list[true_index++] = cx - 1;
		}
	}//while
	if (t != 0)
	{
		for (i = t; (i - t) < true_index; i++)
		{
			temp[i] = true_list[i - t];
			true_list[i] = 0;
		}
		t = i;
		for (i = 0; i < t; i++)
		{
			true_list[i] = temp[i];
		}
		true_index = t;
	}
	destroyset(set);
}

void thr_expression(symset fsys)
{
	symset set;
	int temp[20], i, false_int,cx1;
	set = uniteset(fsys, createset(SYM_QMARK, SYM_NULL));
	LOGIC_OR(set);
	destroyset(set);
	if (sym == SYM_QMARK)
	{
		if (true_index == 0 && false_index == 0)
		{
			gen(JPC, 0, 0);
			false_list[false_index++] = cx - 1;
			gen(JMP, 0, 0);
			true_list[true_index++] = cx - 1;
		}
		code_adjust();
		for (i = 0; i < false_index; i++)
		{
			temp[i] = false_list[i];
			false_list[i] = 0;
		}
		false_int = false_index;
		false_index = 0;
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx;
			true_list[i] = 0;
		}
		true_index = 0;
		getsym();
		set = createset(SYM_COLON, SYM_NULL);
		LOGIC_OR(set);
		destroyset(set);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		cx1 = cx;
		gen(JMP, 0, 0);
		for (i = 0; i < false_int; i++)
		{
			code[temp[i]].a = cx;
		}
		if (sym == SYM_COLON)
		{
			getsym();
		}
		else
		{
			error(17);
		}
		LOGIC_OR(fsys);
		if (true_index != 0 || false_index != 0)
		{
			code_adjust();
			gen(LIT, 0, 1);
			gen(JMP, 0, cx + 2);
			gen(LIT, 0, 0);
			for (i = 0; i < true_index; i++)
			{
				code[true_list[i]].a = cx - 3;
				true_list[i] = 0;
			}
			for (i = 0; i < false_index; i++)
			{
				code[false_list[i]].a = cx - 1;
				false_list[i] = 0;
			}
			true_index = false_index = 0;
		}
		code[cx1].a = cx;
	}//if
}

//判断表达式
void expression(symset fsys)
{
	int sy,poi,addr;
	int i;
	char idn[MAXIDLEN+1];
	symset set;
	mask* mk;
	set = uniteset(fsys, createset(SYM_BECOMES, SYM_NULL));
	sy = sym;
	strcpy_s(idn, strlen(id) + 1, id);
	thr_expression(set);
	if (sy == SYM_IDENTIFIER&&sym==SYM_BECOMES)//=
	{
		if (code[cx-1].f==LOD||code[cx-1].f==LODAR)//':='前仅为一个变量,判断为赋值表达式
		{
			addr = code[cx - 1].a;
			cx--;//将LOD指令删除
			getsym();
			expression(fsys);//id=expression
			if (true_index != 0 || false_index != 0)
			{
				code_adjust();
				gen(LIT, 0, 1);
				gen(JMP, 0, cx + 2);
				gen(LIT, 0, 0);
				for (i = 0; i < true_index; i++)
				{
					code[true_list[i]].a = cx - 3;
					true_list[i] = 0;
				}
				for (i = 0; i < false_index; i++)
				{
					code[false_list[i]].a = cx - 1;
					false_list[i] = 0;
				}
				true_index = false_index = 0;
			}
			//gen(OPR, 0, OPR_BECOMES);
			if (!(poi = position(idn)))
			{
				error(11);//Undeclared identifier.
			}
			else
			{
				switch (table[poi].kind)
				{
				case ID_VARIABLE:
					mk = (mask*)&table[poi];
					gen(STO, level - mk->level, mk->address);
					break;
				case ID_ARRAY:
					mk = (mask*)&table[poi];
					gen(STOAR, level - mk->level, addr);
					break;
				}//switch
			}

		}
		else
		{
			error(12);//Illegal assignment.
		}
	}
	
}

  //////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int  cx1, cx2,jmp[10],i,j,savetx;
	int temp[20];
	int false_int;
	char idn[11];
	symset set1, set;
	mask* mk2;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		strcpy(idn, id);
		i = position(id);
		if (i != 0)
		{
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(fsys, set1);
			expression(set);
			if (true_index != 0 || false_index != 0)
			{
				code_adjust();
				gen(LIT, 0, 1);
				gen(JMP, 0, cx + 2);
				gen(LIT, 0, 0);
				for (i = 0; i < true_index; i++)
				{
					code[true_list[i]].a = cx - 3;
					true_list[i] = 0;
				}
				for (i = 0; i < false_index; i++)
				{
					code[false_list[i]].a = cx - 1;
					false_list[i] = 0;
				}
				true_index = false_index = 0;
			}
			gen(POP, 0, 1);
			destroyset(set1);
			destroyset(set);
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);//";" is expected
			}
		}
		else
		{
			getsym();
			if (sym == SYM_BECOMES)//"="
			{
				getsym();
				if (sym == SYM_LAMBDA)
				{
					cx1 = cx;
					gen(JMP, 0, 0);
					strcpy(id, idn);
					enter(ID_PROCEDURE);
					getsym();
					savetx = tx;
					formal = 0;
					level++;
					while (sym == SYM_IDENTIFIER)
					{
						formal++;
						tx++;
						strcpy(table[tx].name, id);
						mk2 = (mask*)&table[tx];
						mk2->level = level;
						mk2->kind = ID_VARIABLE;
						getsym();
						if (sym == SYM_COMMA)//','
						{
							getsym();
						}
					}
					if (sym == SYM_COLON)//':'
					{
						getsym();
					}
					else
					{
						error(31);//"Wrong lambda expression."
					}
					mk2 = (mask*)&table[savetx];
					mk2->config[0] = formal;
					j = formal + 1;
					for (i = savetx + 1; i <= tx; i++)
					{
						mk2 = (mask*)&table[i];
						mk2->address = -j;
						j--;
					}
					formal_para = formal;
					mk2 = (mask*)&table[savetx];
					mk2->address = cx;
					gen(INT, 0, 3);
					set = uniteset(statbegsys, fsys);
					statement(set);
					destroyset(set);
					formal_para = 0;
					code[cx1].a = cx;
					tx = savetx;
					level--;
				}
				else
				{
					error(11);
				}
			}
			else
			{
				error(11);
			}
		}
	}
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		if (sym == SYM_LPAREN)//'('
		{
			getsym();
		}
		else
		{
			error(22);//"Missing ')'or '('."
		}
		set1 = createset(SYM_RPAREN,SYM_NULL);
		set = uniteset(set1, fsys);
		expression(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22); // ')' expected.
		}
		code_adjust();
		if(false_index==0)
		{
			temp[0] = cx;
			gen(JPC, 0, 0);
			false_int = 1;
		}
		else
		{
			for (false_int = 0; false_int < false_index; false_int++)
			{
				temp[false_int] = false_list[false_int];
				false_list[false_int] = 0;
			}
			false_index = 0;
		}
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx;
			true_list[i] = 0;
		}
		true_index = 0;
		set1 = createset(SYM_ELSE, SYM_ELIF, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		cx1 = cx;//save the position of JMP in the instrutions
		gen(JMP, 0, 0);
		for (i = 0; i < false_int; i++)
		{
			code[temp[i]].a = cx;
		}
		if (sym == SYM_ELSE)
		{//else statement
			getsym();
			statement(fsys);
		}
		else if (sym == SYM_ELIF)
		{
			sym = SYM_IF;
			statement(fsys);
		}
		code[cx1].a = cx;
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_END, SYM_NULL);
		set = uniteset(set1, statbegsys);
		while (inset(sym, statbegsys))
		{
			statement(set);
		} // while
		destroyset(set);
		destroyset(set1);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		if (sym == SYM_LPAREN)//'('
		{
			getsym();
		}
		else
		{
			error(22);//"Missing ')'or '('."
		}
		set1 = createset(SYM_RPAREN, SYM_NULL);
		set = uniteset(set1, fsys);
		expression(set);
		destroyset(set1);
		destroyset(set);
		code_adjust();
		if (false_index == 0)
		{
			temp[0] = cx;
			gen(JPC, 0, 0);
			false_int = 1;
		}
		else
		{
			for (false_int = 0; false_int < false_index; false_int++)
			{
				temp[false_int] = false_list[false_int];
				false_list[false_int] = 0;
			}
			false_index = 0;
		}
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx;
			true_list[i] = 0;
		}
		true_index = 0;
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22); // ')' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		for (i = 0; i < false_int; i++)
		{
			code[temp[i]].a = cx;
		} 
	}
	else if (sym == SYM_FOR)
	{
		instruction tem_code[20];
		getsym();
		set = createset(SYM_COMMA, SYM_SEMICOLON,SYM_RPAREN, SYM_NULL);
		if (sym == SYM_LPAREN)//"("
		{
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				expression(set);
				while (sym == SYM_COMMA)//","
				{
					getsym();
					expression(set);
				}//init_expression
			}
			if (sym == SYM_SEMICOLON)//";"
			{
				getsym();
			}
			else
			{
				error(10);//";" expected
			}
			cx1 = cx;//store the cx
			expression(set);
			code_adjust();
			if (false_index == 0)
		{
			temp[0] = cx;
			gen(JPC, 0, 0);
			false_int = 1;
		}
		else
		{
			for (false_int = 0; false_int < false_index; false_int++)
			{
				temp[false_int] = false_list[false_int];
				false_list[false_int] = 0;
			}
			false_index = 0;
		}
		for (i = 0; i < true_index; i++)
		{
			code[true_list[i]].a = cx;
			true_list[i] = 0;
		}
		true_index = 0;
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);//";' expected
			}
			cx2 = cx-1;
			expression(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22);
			}
			i = cx - cx2;
			for (j = 1; j < i; j++)
			{
				tem_code[j] = code[cx2 + j];
			}//store the code of the expression
			cx = cx2 + 1;
			statement(fsys);
			for (j = 1; j < i; j++)
			{
				code[cx++] = tem_code[j];
			}
			gen(JMP, 0, cx1);
			for (i = 0; i < false_int; i++)
			{
				code[temp[i]].a = cx;
			}
			/*gen(JMP, 0, 0);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22);
			}
			gen(JMP, 0, cx1);
			code[cx2 + 1].a = cx;
			statement(fsys);
			gen(JMP, 0, cx2 + 2);
			code[cx2].a = cx;*/
		}
	}
	else if (sym == SYM_SWITCH)
	{
		getsym();
		if (sym == SYM_LPAREN)//"("
		{
			getsym();
		}
		else
		{
			error(22);//"(" excepted
		}
		set1 = createset(SYM_RPAREN, SYM_NULL);
		set = uniteset(set1, fsys);
		expression(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22);
		}
		if (sym == SYM_BEGIN)
		{
			getsym();
		}
		else
		{
			error(18);//"begin" excepted.
		}
		i = 0;
		while (sym == SYM_CASE)
		{
			getsym();
			set = createset(SYM_COLON,SYM_NULL);
			cx1 = cx;
			expression(set);
			cx2 = cx;
			for (j = cx1; j < cx2; j++)
			{
				if (code[j].f == LOD || code[j].f == LODAR)
				{
					error(14);
					break;
				}
			}
			destroyset(set);
			cx1 = cx;//record the position of the JNE instruction
			gen(JNE, 0, 0);
			if (sym == SYM_COLON)
			{
				getsym();
			}
			else
			{
				error(17);
			}
			set1= createset(SYM_CASE, SYM_NULL);
			set = uniteset(set1, fsys);
			statement(set);
			destroyset(set1);
			destroyset(set);
			jmp[i] = cx;
			i++;
			gen(JMP, 0, 0);
			code[cx1].a = cx;
		}
		cx--;//delete the last JMP instruction
		code[cx1].a = cx;//change the last JNE's address
		for (j = 0; j < i-1; j++)
		{
			code[jmp[j]].a = cx;
		}
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(16);//"end" excepted.
		}
	}
	else if (sym == SYM_RETURN)
	{//return statement
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_NULL);
		set = uniteset(set1, fsys);
		expression(set);
		destroyset(set1);
		destroyset(set);
		gen(STO, 0, -1);
		gen(OPR, formal_para, OPR_RET);
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else
		{
			error(10);//";" is excepted
		}
	}
	else if (sym == SYM_EXIT)
	{//exit statement
		gen(EXT, 0, 0);
		getsym();
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else
		{
			error(10);//";" is excepted
		}
	}
	test(fsys, phi, 19);
} // statement

  //////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk, *mk2;
	int block_dx;
	int savedformal;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*)&table[tx - formal];
	savedformal = formal;
	formal = 0;
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			} while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			int savetx;
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
				savetx = tx;//save the position of the procedure in the table
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}
			level++;
			if (sym == SYM_LPAREN)
			{
				int i,j;
				int savedtx = tx;
				getsym();
				while (sym == SYM_IDENTIFIER)
				{
					formal++;
					tx++;
					strcpy(table[tx].name, id);
					mk2 = (mask*)&table[tx];
					mk2->level = level;
					mk2->kind = ID_VARIABLE;
					getsym();
					if (sym == SYM_COMMA)//','
					{
						getsym();
					}
				}
				if (sym == SYM_RPAREN)//')'
				{
					getsym();
				}
				else
				{
					error(26);//"Illegal procedure declarations"
				}
				mk2 = (mask*)&table[savetx];
				mk2->config[0] = formal;
				j = formal+1;
				for (i = savedtx + 1; i <= tx; i++)
				{
					mk2 = (mask*)&table[i];
					mk2->address = -j;
					j--;
				}
			}
			set1 = createset(SYM_END, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);//difine procedure operation
			destroyset(set1);
			destroyset(set);
			tx = savetx;
			level--;
			/*set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
			set = uniteset(statbegsys, set1);
			test(set, fsys, 6);
			destroyset(set1);
			destroyset(set);*/
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	} while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;//procedure begin
	cx0 = cx;
	gen(INT, 0, block_dx-1);
	formal_para = savedformal;
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

  //////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;

	while (levelDiff--)
		b = stack[b];
	return b;
} // base

  //////////////////////////////////////////////////////////////////////
  // interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = b;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1 - i.l;
				pc = stack[top + 3 + i.l];
				b = stack[top + 2 + i.l];
				stack[top] = stack[top + i.l];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_MOD:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] %= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_BAND:
				top--;
				stack[top] &= stack[top + 1];
				break;
			case OPR_BOR:
				top--;
				stack[top] |= stack[top + 1];
				break;
			case OPR_BXOR:
				top--;
				stack[top] ^= stack[top + 1];
				break;
			case OPR_BECOMES:
				top++;
				stack[top] = stack[top - 1];
				break;
			} // switch
			break;
		case LEA:
			stack[++top] = base(stack, b, i.l) + i.a;
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case LODAR:
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			//printf("%d\n", stack[top]);
			break;
		case STOAR:
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			//printf("%d\n", stack[top]);
			stack[top-1] = stack[top ];
			top -= 1;
			break;
		case CAL:
			stack[top + 2] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 3] = b;
			stack[top + 4] = pc;
			b = top + 2;
			top=b;
			pc = i.a;
			break;
		case CALL:
			switch (i.a)
			{
			case FUN_PRINT:
				int j;
				for (j = i.l-1; j >=0; j--)
				{
					printf("%d\n", stack[top - j]);
				}
				top -= i.l;
				break;
			case FUN_RANDOM:
				stack[++top] = rand();
				break;
			case FUN_INPUT:
				printf("Enter the number:");
				for (j = i.l - 1; j >= 0; j--)
				{
					scanf_s("%d", &stack[stack[top - j]], 1);
				}
				top -= i.l;
				break;
			//case FUN_CALLSTACK:


			}//switch
			break;
		case INT:
			top += i.a;
			break;
		case POP:
			top -= i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case JNE:
			if (stack[top] != stack[top - 1])
				pc = i.a;
			top--;
			break;
		case JG:
			if (stack[top - 1] > stack[top])
				pc = i.a;
			top -= 2;
			break;
		case JGE:
			if (stack[top - 1] >= stack[top])
				pc = i.a;
			top -= 2;
			break;
		case JB:
			if (stack[top - 1] < stack[top])
				pc = i.a;
			top -= 2;
			break;
		case JBE:
			if (stack[top - 1] <= stack[top])
				pc = i.a;
			top -= 2;
			break;
		case EXT:
			printf("EXIT THE PROGRAM!\n");
			pc = 0;
			break;
		} // switch
	} while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

  //////////////////////////////////////////////////////////////////////
void main()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	DEFAULTPRO();
	for (i = 0; i < 20; i++)
	{
		true_list[i] = false_list[i] = 0;
	}
	printf("Please input source file name: "); // get file name to be compiled
	scanf_s("%s", s,80);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);

	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_IDENTIFIER,SYM_IF, SYM_WHILE,SYM_RETURN, SYM_FOR,SYM_SWITCH ,SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NEG,SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
	system("pause");
} // main

  //////////////////////////////////////////////////////////////////////
  // eof pl0.c
