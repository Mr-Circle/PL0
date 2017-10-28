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
	if (cc == ll)//cc==ll代表当前line中元素并未完全扫描结束，不需要从文件中取下一行字符
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		/*利用while循环将文件中一行字符直接取出至line */
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;//ll始终为当前读取字母的数量，从line【1】处开始有字母
		} // while
		printf("\n");
		line[++ll] = ' ';//在取出字符串结尾处添加空格，方便getsym()中在一行字符处理结束后调用get()获取下一行字符串
	}
	ch = line[++cc];//先将文件中整行字符串读出至line中，再用ch从line中单个取字母
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
		strcpy(id, a);//将获取的字符串复制到字符数组id中
		word[0] = id;//将id赋值给word中保留位作为哨兵位，方便比较
		i = NRW;
		while (strcmp(id, word[i--]));//与保留字表中的保留字进行比较，判断字符串是否为保留字
		if (++i)//id与word中原有的保留字匹配上
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	/*若首部为数字则进入是否为数字的判断流程*/
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		/*利用字符串形式的数字求出真实数值存储在num中*/
		do
		{
			num = num * 10 + ch - '0';
			k++;//k记录数字的位数
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)//若数字位数超过定义过的最大位数
			error(25);     // The number is too great.
	}
	/*下面一系列流程均为判断字符是否为运算符，并且按照是否需要超前搜索分为两类*/
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
	/*判断字符是否为保存于csym中的无需超前搜索即可判断的运算符*/
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;//将当前字符赋值给csym数组中的保留位csym[0]作为哨兵位
		while (csym[i--] != ch);//循环判断当前字符是否在csym出现
		if (++i)//若当前字符与csym中原本保存的运算符匹配上
		{
			sym = ssym[i];
			getch();
		}
		else//若上述要求当前字符均不符合，则可判定为非法字符
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
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
	} // switch
} // enter

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
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
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

  //////////////////////////////////////////////////////////////////////
//项的判定
void factor(symset fsys)
{
	void expression(symset fsys);
	int i,j;
	int parameter = 0;
	symset set;

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
					break;
				case ID_VARIABLE:
					mk = (mask*)&table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					mk = (mask*)&table[i];
					getsym();
					if(sym==SYM_LPAREN)
					{
						getsym();
						while (sym == SYM_IDENTIFIER||sym == SYM_NUMBER)
						{
							if (sym == SYM_IDENTIFIER)
							{
								if ((j = position(id)) == 0)
								{
									error(11);//Undeclared identifier
								}
								else
								{
									switch (table[j].kind)
									{
									case ID_CONSTANT:
										gen(LIT, 0, table[j].value);
										break;
									case ID_VARIABLE:
										mk = (mask*)&table[j];
										gen(LOD, level - mk->level, mk->address);
										break;
									case ID_PROCEDURE:
										error(13);
										break;
									}
								}
							}
							else
							{//sym==SYM_NUMBER
								gen(LIT, 0, num);
							}
							parameter++;
							getsym();
							if (sym == SYM_COMMA)//','
							{
								getsym();
							}
						}
						if (sym == SYM_RPAREN)//')'
						{
							mk = (mask*)&table[i];
							gen(CAL, level - mk->level, mk->address);
						}
						else
						{
							error(22);//Missing ')'.
						}
						if (parameter != mk->config[0])
						{
							error(27);
						}
					}
					else
					{
						error(21);//"Procedure identifier can not be in an expression without bracket pair."
					}
					break;
				} // switch
			}
			getsym();
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
			gen(OPR, 0, OPR_NEGL);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
} // factor

  //////////////////////////////////////////////////////////////////////
//判定因子
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD,SYM_NULL));//（'*','/','%')
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH||sym==SYM_MOD)
	{
		mulop = sym;
		getsym();
		factor(set);
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
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));//'+','-'

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
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
	symset set;
	set = uniteset(fsys, createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_GEQ, SYM_GTR, SYM_LEQ));//'=','<>','>','<','<=','>='
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
			relop = sym;
			getsym();
			ADD_exp(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // if
	} // else
} // condition

//判断按位与语句
void BIT_AND(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_BAND, SYM_NULL));
	condition(set);
	while (sym == SYM_BAND)
	{
		getsym();
		condition(set);
		gen(OPR, 0, OPR_BAND);
	}//while
	destroyset(set);
}//BIT_AND

//判断^语句
void BIT_XOR(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_BXOR, SYM_NULL));
	BIT_AND(set);
	while (sym == SYM_BXOR)
	{
		getsym();
		BIT_AND(set);
		gen(OPR, 0, OPR_BXOR);
	}
	destroyset(set);
}

//判断'|'语句
void BIT_OR(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_BOR, SYM_NULL));
	BIT_XOR(set);
	while (sym == SYM_BOR)
	{
		getsym();
		BIT_XOR(set);
		gen(OPR, 0, OPR_OR);
	}
	destroyset(set);
}

//判断条件语句因子&&
void LOGIC_AND(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
	BIT_OR(set);
	while (sym==SYM_AND)
	{
		getsym();
		BIT_OR(set);
		gen(OPR, 0, OPR_AND);
	}//while
	destroyset(set);
}

//判断表达式||
void LOGIC_OR(symset fsys)
{
	symset set;
	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));
	LOGIC_AND(set);
	while (sym == SYM_OR)//||
	{
		getsym();
		LOGIC_AND(set);
		gen(OPR, 0, OPR_OR);
	}//while
	destroyset(set);
}

//判断表达式
void expression(symset fsys)
{
	int i,sy,poi;
	char idn[MAXIDLEN+1];
	symset set;
	mask* mk;
	set = uniteset(fsys, createset(SYM_BECOMES, SYM_NULL));
	i = cx;
	sy = sym;
	strcpy_s(idn, strlen(id) + 1, id);
	LOGIC_OR(set);
	if (sy == SYM_IDENTIFIER&&sym==SYM_BECOMES)//=
	{
		if ((cx - i) == 1)//':='前仅为一个变量,判断为赋值表达式
		{
			cx--;//将LOD指令删除
			getsym();
			expression(fsys);//id=expression
			gen(OPR, 0, OPR_BECOMES);
			if (!(poi = position(idn)))
			{
				error(11);//Undeclared identifier.
			}
			else if (table[poi].kind!=ID_VARIABLE)
			{
				error(12);//Illegal assignment.
				poi = 0;
			}
			mk = (mask*)&table[poi];
			if (poi)
			{
				printf("position:%d\n", poi);
				gen(STO, level-mk->level, mk->address);
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
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		expression(fsys);
		/*mask* mk;
		if (!(i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask*)&table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}*/
	}
	/*else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (!(i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*)&table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	}*/
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		if (sym == SYM_LPAREN)//'('
		{
			getsym();
		}
		else
		{
			error(22);
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
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
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
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		expression(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if (sym == SYM_RETURN)
	{//return statement
		mask* mk;
		int i;
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_NULL);
		set = uniteset(set1, fsys);
		expression(set);
		gen(STO, 0, -1);
		gen(OPR, formal_para, OPR_RET);
		/*if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11);//Undeclared Identifier
			}
			else
			{
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);
				gen(STO, 0, -1);//store the returned value
			}
		}
		else if (sym == SYM_NUMBER)
		{
			gen(LIT, 0, num);
			gen(STO, 0, -1);//store the returned value
		}
		getsym();*/
	}
	test(fsys, phi, 19);
} // statement

  //////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk, *mk2;
	int block_dx;
	int savedTx;
	int savedformal;
	symset set1, set;

	dx = 4;
	block_dx = dx;
	mk = (mask*)&table[tx - formal];
	savedformal = formal;
	formal = 0;
	printf("tx: %d\n", tx);
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


			/*if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}*/
			level++;
			savedTx = tx;
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
				printf("formal:%d,savedtx:%d,tx:%d\n", formal, savedtx,tx);
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
			tx = savedTx;
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
	gen(INT, 0, block_dx);
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
	top = 3;
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
				top = b - 1-i.l;
				pc = stack[top + 3+i.l];
				b = stack[top + 2+i.l];
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
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			case OPR_NEGL:
				stack[top] = !stack[top];
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
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 2] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 3] = b;
			stack[top + 4] = pc;
			b = top + 2;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
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
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
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
