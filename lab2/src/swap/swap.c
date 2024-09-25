#include "swap.h"

void Swap(char *left, char *right)
{
	char interm = *left;
	*left = *right;
	*right = interm;
}
