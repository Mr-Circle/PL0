#pragma once
#ifndef SET_H
#define SET_H

//Í¬²½·ûºÅ¼¯
typedef struct snode
{
	int elem;
	struct snode* next;
} snode, *symset;

extern symset phi, declbegsys, statbegsys, facbegsys, relset;

symset createset(int data, .../* SYM_NULL */);
void destroyset(symset s);
symset uniteset(symset s1, symset s2);
int inset(int elem, symset s);

#endif
// EOF set.h
