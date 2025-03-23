#include "return_codes.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define OUTPUT_SHIFT_SINGLE_MANT 1
#define OUTPUT_SHIFT_HALF_MANT 2
#define LEN_MANT_HALF_PRECISION 10
#define LEN_EXP_HALF_PRECISION 5
#define LEN_MANT_SINGLE_PRECISION 23
#define LEN_EXP_SINGLE_PRECISION 8
#define LEN_LONG 64

typedef enum type
{
	Zero = -9,
	Inf = 9,
	NaN = 18,
	JustNum = 2
} typeNum;

typedef struct
{
	uint8_t sign;
	uint8_t lenExp;
	int16_t exp;
	uint8_t lenMant;
	uint8_t invisibleBits;
	uint8_t lastBit;
	uint64_t mant;
	typeNum type;
} Num;

uint8_t highBit(uint64_t n)
{
	uint8_t ans = 0;
	while (ans != (LEN_LONG - 1) && n >= (1ll << (ans + 1)))
		++ans;
	return ans;
}

uint16_t myAbs(int16_t a)
{
	return a > 0 ? (uint16_t)a : (uint16_t)(-a);
}

void roundNum(Num *n, uint8_t typeRound)
{
	if (n->type != JustNum)
		return;
	int16_t nExp = n->exp;
	uint8_t nLenExp = n->lenExp;
	uint8_t nLenMant = n->lenMant;
	uint8_t upperBound = (uint8_t)((1 << (nLenExp - 1)) - 1);
	int16_t lowerBound = 1 - upperBound - nLenMant;
	if (nExp <= upperBound && nExp >= lowerBound)
	{
		int8_t diff = (int8_t)(highBit(n->mant) - nLenMant);
		uint8_t absDiff = (uint8_t)myAbs(diff);
		for (uint8_t i = 0; i < absDiff; ++i)
		{
			if (diff > 0)
			{
				if ((n->lastBit = n->mant & 1))
					++n->invisibleBits;
			}
			n->mant = diff > 0 ? n->mant >> 1 : n->mant << 1;
		}
		if ((typeRound == 1 && n->lastBit && (n->invisibleBits > 1 || n->mant & 1)) ||
			(typeRound > 1 && n->invisibleBits && ((n->sign + typeRound) & 1) ^ 1))
		{
			++n->mant;
			if (highBit(n->mant) != nLenMant)
			{
				++n->exp;
				n->mant >>= 1;
			}
		}
	}
	else if (nExp > upperBound)
	{
		if (typeRound == 1 || (typeRound && !((n->sign + typeRound) & 1)))
			n->type = Inf;
		else
		{
			n->mant = (1 << (nLenMant + 1)) - 1;
			n->exp = (int16_t)upperBound;
		}
	}
	else
	{
		if (typeRound <= 1 || (n->sign + typeRound) & 1)
			n->type = Zero;
		else
		{
			n->mant = (1 << nLenMant);
			n->exp = (int16_t)(1 - upperBound - nLenMant);
		}
	}
}

Num multiply(Num *n1, Num *n2)
{
	Num res = *n1;
	res.sign = n1->sign ^ n2->sign;
	typeNum n1Type = n1->type;
	typeNum n2Type = n2->type;
	if (n1Type == JustNum && n2Type == JustNum)
	{
		res.lastBit = res.invisibleBits = 0;
		res.mant = n1->mant * n2->mant;
		res.exp = (int16_t)((uint64_t)(n1->exp + n2->exp) + (res.mant >> (((res.lenMant + 1) * 2) - 1)));
	}
	else
		res.type = n1Type == NaN || n2Type == NaN || !(n1Type + n2Type) ? NaN : (n1Type + n2Type < 0 ? Zero : Inf);
	return res;
}

Num divide(Num *n1, Num *n2)
{
	Num res = *n1;
	res.sign = n1->sign ^ n2->sign;
	typeNum n1Type = n1->type;
	typeNum n2Type = n2->type;
	if (n1Type == JustNum && n2Type == JustNum)
	{
		uint64_t n1Mant = n1->mant;
		uint64_t n2Mant = n2->mant;
		uint8_t shift = LEN_LONG - (n1->lenMant + 1);

		res.lastBit = 0;
		res.exp = (int16_t)(n1->exp - n2->exp);
		res.mant = (n1Mant << shift) / n2Mant;
		res.invisibleBits = !!((n1Mant << shift) % n2Mant);
		res.exp -= res.lenMant - highBit(res.mant >> (shift - res.lenMant));
	}
	else
		res.type = myAbs((int16_t)n1Type) + myAbs((int16_t)n2Type) >= NaN && (n1Type + n2Type) ? NaN : (n1Type > n2Type ? Inf : Zero);
	return res;
}

uint8_t alignment(Num *n1, Num *n2)
{
	int16_t n1Exp = n1->exp;
	int16_t n2Exp = n2->exp;
	if (n1Exp != n2Exp && n1->type * n2->type == 2 * JustNum)
	{
		uint16_t Absdiff = myAbs((int16_t)(n1Exp - n2Exp));
		if (Absdiff <= (LEN_LONG - (n1->lenMant + 1)))
		{
			Num *maxExp = n1Exp >= n2Exp ? n1 : n2;
			maxExp->exp -= Absdiff;
			maxExp->mant <<= Absdiff;
			return 0;
		}
		Num *minExp = n1Exp <= n2Exp ? n1 : n2;
		minExp->mant = 0;
		return 1;
	}
	return 0;
}

Num add(Num *n1, Num *n2)
{
	Num res = *n1;
	typeNum n1Type = n1->type;
	typeNum n2Type = n2->type;
	if (n1Type <= JustNum && n2Type <= JustNum)
	{
		res.invisibleBits = alignment(n1, n2);
		uint8_t n1Sign = n1->sign;
		uint8_t n2Sign = n2->sign;
		uint64_t n1Mant = n1->mant;
		uint64_t n2Mant = n2->mant;

		Num *absGreater = n1Mant > n2Mant ? n1 : (n2Mant > n1Mant ? n2 : (n1Sign >= n2Sign ? n2 : n1));
		res.exp = absGreater->exp;
		if (res.invisibleBits)
		{
			res.mant = absGreater->mant;
			if ((res.lastBit = (uint8_t)(n1Sign ^ n2Sign)))
			{
				--res.mant;
				++res.invisibleBits;
			}
		}
		else
		{
			res.lastBit = 0;
			uint64_t absLessMant = absGreater == n1 ? n2Mant : n1Mant;
			res.mant = n1Sign ^ n2Sign ? absGreater->mant - absLessMant : absGreater->mant + absLessMant;
			if (res.mant)
				res.exp -= res.lenMant - highBit(res.mant);
		}
		res.sign = absGreater->sign;
		res.type = res.mant ? absGreater->type : Zero;
	}
	else
	{
		res.type = (n1Type + n2Type) && ((myAbs((int16_t)n1Type)) + (myAbs((int16_t)n2Type)) + n1->sign ^ n2->sign) > NaN ? NaN : Inf;
		res.sign = n1Type == Inf ? n1->sign : n2->sign;
	}
	return res;
}

Num subtract(Num *n1, Num *n2)
{
	n2->sign = !n2->sign;
	return add(n1, n2);
}

Num createNum(uint32_t intNum, uint8_t precision)
{
	Num structNum;
	structNum.invisibleBits = structNum.lastBit = 0;
	if (precision == 'h')
	{
		structNum.lenExp = LEN_EXP_HALF_PRECISION;
		structNum.lenMant = LEN_MANT_HALF_PRECISION;
	}
	else
	{
		structNum.lenExp = LEN_EXP_SINGLE_PRECISION;
		structNum.lenMant = LEN_MANT_SINGLE_PRECISION;
	}
	structNum.mant = intNum % (1 << structNum.lenMant);
	intNum /= (1 << structNum.lenMant);
	uint8_t shift = (uint8_t)((1 << (structNum.lenExp - 1)) - 1);
	structNum.exp = (int16_t)(intNum % (1 << structNum.lenExp));
	if (structNum.exp)
	{
		structNum.exp -= shift;
		structNum.type = structNum.exp != shift + 1 ? JustNum : (structNum.mant > 0 ? NaN : Inf);
		structNum.mant += 1 << structNum.lenMant;
	}
	else
	{
		structNum.type = structNum.mant ? JustNum : Zero;
		uint8_t diff = structNum.mant ? structNum.lenMant - highBit(structNum.mant) : 0;
		structNum.mant <<= diff;
		structNum.exp = diff ? 1 - shift - diff : 0;
	}
	intNum /= (1 << structNum.lenExp);
	structNum.sign = !!intNum;
	return structNum;
}

int main(int argc, char **argv)
{
	uint8_t precision;
	int8_t round;
	if ((argc != 4 && argc != 6) || strlen(argv[1]) > 1 || ((precision = (uint8_t)argv[1][0]) != 'f' && precision != 'h') ||
		sscanf(argv[2], "%hhd", &round) != 1 || round < 0 || round > 3)
	{
		fprintf(stderr, "Incorrect input data");
		return ERROR_ARGUMENTS_INVALID;
	}
	uint32_t intNum1;
	sscanf(argv[3], "%X", &intNum1);
	Num num1 = createNum(intNum1, precision);
	if (argc == 6)
	{
		uint8_t operation;
		if (strlen(argv[4]) > 1 || !(operation = (uint8_t)argv[4][0]))
		{
			fprintf(stderr, "Incorrect input data");
			return ERROR_ARGUMENTS_INVALID;
		}
		uint32_t intNum2;
		sscanf(argv[5], "%X", &intNum2);
		Num num2 = createNum(intNum2, precision);
		switch (operation)
		{
		case '*':
			num1 = multiply(&num1, &num2);
			break;

		case '/':
			num1 = divide(&num1, &num2);
			break;

		case '+':
			num1 = add(&num1, &num2);
			break;

		case '-':
			num1 = subtract(&num1, &num2);
			break;

		default:
			fprintf(stderr, "Not supported operation");
			return ERROR_ARGUMENTS_INVALID;
		}
		roundNum(&num1, (uint8_t)round);
	}

	if (num1.type == NaN)
		puts("nan");
	else if (num1.type == Inf)
		puts(num1.sign ? "-inf" : "inf");
	else
	{
		if (num1.type == Zero)
		{
			num1.mant = 0;
			num1.exp = 0;
		}
		printf(
			"%s0x%d.%0*llxp%+d\n",
			num1.sign ? "-" : "",
			!!num1.mant,
			precision == 'h' ? 3 : 6,
			num1.mant ? (num1.mant - (1ll << num1.lenMant)) << (num1.lenMant == 23 ? OUTPUT_SHIFT_SINGLE_MANT : OUTPUT_SHIFT_HALF_MANT) : 0,
			num1.exp);
	}
	return SUCCESS;
}
