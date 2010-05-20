#include <string.h>
#include <strings.h>

char *substr(char *str, char *sub, int i, int l)
{
	int j = 0;

	while (i-- > 0) str++;
	for (j = 0; j < l && str[j] != '\0'; j++)
		sub[j] = str[j];
	if (j <= l) sub[j] = '\0'; // encounter a null terminator

	return sub;
}
