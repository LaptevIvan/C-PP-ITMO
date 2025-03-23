#include <stdint.h>
#include <stdio.h>

#define M_31 2147483647
#define TWO_POWER16 (UINT16_MAX + 1)
#define lenN 1
#define lenNFact 2
#define lenUINT16_MAX 5

uint64_t factorial(uint16_t n)
{
	uint64_t ans = 1;
	for (uint16_t i = 0; i < n; ++i)
	{
		ans = (ans * (i + 1)) % M_31;
	}
	return ans;
}

void printStr(uint8_t cnt, char start, char mid, char end)
{
	if (start)
		putchar(start);
	for (uint8_t p = 0; p < cnt; ++p)
		putchar(mid);
	if (end)
		printf("%c\n", end);
}

void printHead(uint8_t col1, uint8_t col2, int8_t allign)
{
	printStr(col1 + 2, '+', '-', 0);
	printStr(col2 + 2, '+', '-', '+');
	switch (allign)
	{
	case -1:
		printf("| %-*s | %-*s |\n", col1, "n", col2, "n!");
		break;
	case 0:
	{
		uint8_t indent1 = ((col1 - lenN) >> 1) + !(col1 & 1);
		uint8_t indent2 = ((col2 - lenNFact) >> 1) + (col2 & 1);
		printf("|%*s n %*s|%*s n! %*s|\n", indent1, "", col1 - lenN - indent1, "", indent2, "", col2 - lenNFact - indent2, "");
		break;
	}
	case 1:
		printf("| %*s | %*s |\n", col1, "n", col2, "n!");
		break;
	}
	printStr(col1 + 2, '+', '-', 0);
	printStr(col2 + 2, '+', '-', '+');
}

uint8_t lenNum(uint32_t num)
{
	uint8_t len = 0;
	do
	{
		num /= 10;
		++len;
	} while (num > 0);
	return len;
}

void printNumStr(uint8_t col1, uint16_t i, uint8_t col2, uint32_t num, int8_t align)
{
	switch (align)
	{
	case -1:
		printf("| %-*d | %-*d |\n", col1, i, col2, num);
		break;
	case 0:
	{
		uint8_t len1 = lenNum(i);
		uint8_t len2 = lenNum(num);
		uint8_t indent1 = ((col1 - len1) >> 1) + (col1 & 1 ^ len1 & 1);
		uint8_t indent2 = ((col2 - len2) >> 1) + (col2 & 1 ^ len2 & 1);
		printf("|%*s %d %*s|%*s %d %*s|\n", indent1, "", i, col1 - len1 - indent1, "", indent2, "", num, col2 - len2 - indent2, "");
		break;
	}
	case 1:
		printf("| %*d | %*d |\n", col1, i, col2, num);
		break;
	}
}

uint8_t checkIncrement(uint64_t *curVal, uint16_t *curCounter, const uint16_t *end)
{
	if (*curCounter == *end)
		return 0;
	*curCounter = (*curCounter + 1) % TWO_POWER16;
	*curVal = *curCounter != 0 ? (*curVal * *curCounter) % M_31 : 1;
	return 1;
}

int main(void)
{
	int32_t start, end;
	int8_t align;
	if (scanf("%d %d %hhd", &start, &end, &align) != 3 || align < -1 || align > 1 || start < 0 || end < 0)
	{
		fprintf(stderr, "Incorrect input data");
		return 1;
	}
	uint64_t current, currentForLen;
	current = currentForLen = factorial(start);
	uint8_t maxLength1 = start > end ? lenUINT16_MAX : lenNum(end);
	uint8_t maxLength2 = lenNFact;
	uint16_t i = start;
	do
	{
		uint8_t curLen = lenNum(currentForLen);
		maxLength2 = maxLength2 < curLen ? curLen : maxLength2;
	} while (checkIncrement(&currentForLen, &i, (uint16_t *)&end));

	printHead(maxLength1, maxLength2, align);

	do
	{
		printNumStr(maxLength1, start, maxLength2, current, align);
	} while (checkIncrement(&current, (uint16_t *)&start, (uint16_t *)&end));

	printStr(maxLength1 + 2, '+', '-', 0);
	printStr(maxLength2 + 2, '+', '-', '+');
	return 0;
}
