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
  /*getch()������Դ�����ȡ�ַ�*/
void getch(void)
{
	if (cc == ll)//cc==ll����ǰline��Ԫ��ɨ���������Ҫ���ļ���ȡ��һ���ַ�
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		/*����whileѭ�����ļ���һ���ַ�ֱ��ȡ����line */
		while ((!feof(infile)) // added & modified by alex 01-02-09
			&& ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;//llʼ��Ϊ��ǰ��ȡ��ĸ����������line��1������ʼ����ĸ
		} // while
		printf("\n");
		line[++ll] = ' ';//��ȡ���ַ�����β����ӿո񣬷���getsym()����һ���ַ�������������get()��ȡ��һ���ַ���
	}
	ch = line[++cc];//�Ƚ��ļ��������ַ���������line�У�����ch��line�е���ȡ��ĸ
} // getch

  //////////////////////////////////////////////////////////////////////
  // gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];//��Ϊ�����������ַ������ǰ׺����ʱ�洢

						 //�����ո���Ʊ��
	while (ch == ' ' || ch == '\t')
		getch();

	/*���ײ�Ϊ��ĸ������ʶ���������ֵ��ж�����*/
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
		strcpy(id, a);//����ȡ���ַ������Ƶ��ַ�����id��
		word[0] = id;//��id��ֵ��word�б���λ��Ϊ�ڱ�λ������Ƚ�
		i = NRW;
		while (strcmp(id, word[i--]));//�뱣���ֱ��еı����ֽ��бȽϣ��ж��ַ����Ƿ�Ϊ������
		if (++i)//id��word��ԭ�еı�����ƥ����
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	/*���ײ�Ϊ����������Ƿ�Ϊ���ֵ��ж�����*/
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		/*�����ַ�����ʽ�����������ʵ��ֵ�洢��num��*/
		do
		{
			num = num * 10 + ch - '0';
			k++;//k��¼���ֵ�λ��
			getch();
		} while (isdigit(ch));
		if (k > MAXNUMLEN)//������λ����������������λ��
			error(25);     // The number is too great.
	}
	/*����һϵ�����̾�Ϊ�ж��ַ��Ƿ�Ϊ����������Ұ����Ƿ���Ҫ��ǰ������Ϊ����*/
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
	/*�ж��ַ��Ƿ�Ϊ������csym�е����賬ǰ���������жϵ������*/
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;//����ǰ�ַ���ֵ��csym�����еı���λcsym[0]��Ϊ�ڱ�λ
		while (csym[i--] != ch);//ѭ���жϵ�ǰ�ַ��Ƿ���csym����
		if (++i)//����ǰ�ַ���csym��ԭ������������ƥ����
		{
			sym = ssym[i];
			getch();
		}
		else//������Ҫ��ǰ�ַ��������ϣ�����ж�Ϊ�Ƿ��ַ�
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
	case ID_CONSTANT://����
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE://����
		mk = (mask*)&table[tx];
		mk->level = level;//�������ڵĲ��
		mk->address = dx++;
		break;
	case ID_PROCEDURE://����
		mk = (mask*)&table[tx];
		mk->level = level;
		break;
	case ID_ARRAY://����
		mk = (mask*)&table[tx];
		mk->level = level;
		mk->address = dx;
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
void constdeclaration()//const��ֵ���
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
void vardeclaration(void)//�����������
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
					printf("num is %d\n", num);
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
//�����ɵ�ָ���г���
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
//����ж�
void factor(symset fsys)
{
	void expression(symset fsys);
	int i,offset=0;
	int parameter = 0;
	symset set;

	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
            //���ű���δ����
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
							error(27);
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
			gen(OPR, 0, OPR_NEGL);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
} // factor

  //////////////////////////////////////////////////////////////////////
//�ж�����
void term(symset fsys)
{
	int mulop;
	symset set;

	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MOD,SYM_NULL));//��'*','/','%')
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
//�ж�'+','-'���ʽ
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
  //�ж����������
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

//�жϰ�λ�����
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

//�ж�^���
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

//�ж�'|'���
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

//�ж������������&&
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

//�жϱ��ʽ||
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

//�жϱ��ʽ
void expression(symset fsys)
{
	int sy,poi,addr;
	char idn[MAXIDLEN+1];
	symset set;
	mask* mk;
	set = uniteset(fsys, createset(SYM_BECOMES, SYM_NULL));
	sy = sym;
	strcpy_s(idn, strlen(id) + 1, id);
	LOGIC_OR(set);
	if (sy == SYM_IDENTIFIER&&sym==SYM_BECOMES)//=
	{
		if (code[cx-1].f==LOD||code[cx-1].f==LODAR)//':='ǰ��Ϊһ������,�ж�Ϊ��ֵ���ʽ
		{
			addr = code[cx - 1].a;
			cx--;//��LODָ��ɾ��
			getsym();
			expression(fsys);//id=expression
			gen(OPR, 0, OPR_BECOMES);
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
	int  cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		set1 = createset(SYM_SEMICOLON, SYM_NULL);
		set = uniteset(fsys, set1);
		expression(set);
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
		cx1 = cx;
		gen(JPC, 0, 0);
		set1 = createset(SYM_ELSE, SYM_ELIF, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		code[cx1].a = cx+1;
		cx1 = cx;//save the position of JMP in the instrutions
		gen(JMP, 0, 0);
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
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22); // ')' expected.
		}
		cx2 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if (sym == SYM_FOR)
	{
		getsym();
		set = createset(SYM_COMMA, SYM_SEMICOLON,SYM_RPAREN, SYM_NULL);
		if (sym == SYM_LPAREN)//"("
		{
			getsym();
			expression(set);
			while(sym==SYM_COMMA)//","
			{
				getsym();
				expression(set);
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
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);//";' expected
			}
			cx2 = cx;
			gen(JPC, 0, 0);
			gen(JMP, 0, 0);
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
			code[cx2].a = cx;
		}
	}
	/*else if (sym == SYM_SWITCH)
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
	}*/
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
		case LODAR:
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case STOAR:
			stack[base(stack, b, i.l) + i.a + stack[top - 2]] = stack[top];
			printf("%d\n", stack[top]);
			stack[top-2] = stack[top ];
			top -= 2;
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
