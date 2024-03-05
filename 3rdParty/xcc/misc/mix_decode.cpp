/*
    XCC Utilities and Library
    Copyright (C) 2000  Olaf van der Spek  <olafvdspek@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include <memory>
#include <mix_decode.h>

const static char* pubkey_str = "AihRvNoIbTn85FZRYNZRcT+i6KpU+maCsEqr3Q5q+LDB5tH7Tz2qQ38V";

const static char char2num[] = 
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

using word = unsigned short;
using bignum = uint32_t[64];

static struct 
{
	bignum key1;
	bignum key2;
	uint32_t len;
} pubkey;

static bignum glob1;
static uint32_t glob1_bitlen;
static uint32_t glob1_len_x2;
static uint32_t glob2[130];
static uint32_t glob1_hi[4];
static uint32_t glob1_hi_inv[4];
static uint32_t glob1_hi_bitlen;
static uint32_t glob1_hi_inv_lo;
static uint32_t glob1_hi_inv_hi;

static void init_bignum(bignum n, uint32_t val, uint32_t len)
{
	memset(n, 0, len * 4);
	n[0] = val;
}

static void move_key_to_big(bignum n, const char* key, uint32_t klen, uint32_t blen)
{
	uint32_t sign = key[0] & 0x80 ? 0xff : 0;
	int i = blen * 4;
	for (; i > klen; i--)
		((char *)n)[i - 1] = sign;
	for (; i > 0; i--) 
		((char *)n)[i - 1] = key[klen - i];
}

static void key_to_bignum(bignum n, char *key, uint32_t len)
{
	uint32_t keylen;
	int i;
	
	if (key[0] != 2) 
		return;
	key++;
	
	if (key[0] & 0x80) 
	{
		keylen = 0;
		for (i = 0; i < (key[0] & 0x7f); i++) keylen = (keylen << 8) | key[i+1];
		key += (key[0] & 0x7f) + 1;
	} 
	else 
	{
		keylen = key[0];
		key++;
	}
	if (keylen <= len * 4) 
		move_key_to_big(n, key, keylen, len);
}

static uint32_t len_bignum(bignum n, uint32_t len)
{
	int i = len - 1;
	while (i >= 0 && n[i] == 0)
		i--;
	return i + 1;
}

static uint32_t bitlen_bignum(bignum n, uint32_t len)
{
	uint32_t ddlen = len_bignum(n, len);
	if (ddlen == 0)
		return 0;
	uint32_t bitlen = ddlen * 32;
	uint32_t mask = 0x80000000;
	while ((mask & n[ddlen - 1]) == 0)
	{
		mask >>= 1;
		bitlen--;
	}
	return bitlen;
}

static void init_pubkey()
{
	init_bignum(pubkey.key2, 0x10001, 64);
	char keytmp[256];
	uint32_t i2 = 0;
	for (uint32_t i = 0; i < strlen(pubkey_str);)
	{
		uint32_t tmp = char2num[pubkey_str[i++]];
		tmp <<= 6; tmp |= char2num[pubkey_str[i++]];
		tmp <<= 6; tmp |= char2num[pubkey_str[i++]];
		tmp <<= 6; tmp |= char2num[pubkey_str[i++]];
		keytmp[i2++] = (tmp >> 16) & 0xff;
		keytmp[i2++] = (tmp >> 8) & 0xff;
		keytmp[i2++] = tmp & 0xff;
	}
	key_to_bignum(pubkey.key1, keytmp, 64);
	pubkey.len = bitlen_bignum(pubkey.key1, 64) - 1;
}

static uint32_t len_predata()
{
	uint32_t a = (pubkey.len - 1) / 8;
	return (55 / a + 1) * (a + 1);
}

static int cmp_bignum(bignum n1, bignum n2, uint32_t len)
{
	n1 += len - 1;
	n2 += len - 1;
	while (len > 0)
	{
		if (*n1 < *n2) return -1;
		if (*n1 > *n2) return 1;
		n1--;
		n2--;
		len--;
	}
	return 0;
}

static void mov_bignum(bignum dest, bignum src, uint32_t len)
{
	memmove(dest, src, len * 4);
}

static void shr_bignum(bignum n, uint32_t bits, long int len)
{
	uint32_t i;
	if (uint32_t i2 = bits / 32)
	{
		for (i = 0; i < len - i2; i++) 
			n[i] = n[i + i2];
		for (; i < len; i++) 
			n[i] = 0;
		bits %= 32;
	}
	if (!bits)
		return;
	for (i = 0; i < len - 1; i++)
		n[i] = (n[i] >> bits) | (n[i + 1] << (32 - bits));
	n[i] >>= bits;
}

static void shl_bignum(bignum n, uint32_t bits, uint32_t len)
{
	uint32_t i;
	if (uint32_t i2 = bits / 32)
	{
		for (i = len - 1; i > i2; i--) 
			n[i] = n[i - i2];
		for (; i > 0; i--) 
			n[i] = 0;
		bits %= 32;
	}
	if (!bits)
		return;
	for (i = len - 1; i > 0; i--)
		n[i] = (n[i] << bits) | (n[i - 1] >> (32 - bits));
	n[0] <<= bits;
}

static uint32_t sub_bignum(bignum dest, bignum src1, bignum src2, uint32_t carry, uint32_t len)
{
	len *= 2;
	while (--len != -1)
	{
		uint32_t i1 = *(word *)src1;
		uint32_t i2 = *(word *)src2;
		*(word *)dest = i1 - i2 - carry;
		src1 = (uint32_t *)(((word *)src1) + 1);
		src2 = (uint32_t *)(((word *)src2) + 1);
		dest = (uint32_t *)(((word *)dest) + 1);
		carry = ((i1 - i2 - carry) & 0x10000) ? 1 : 0;
	}
	return carry;
}

static void inv_bignum(bignum n1, bignum n2, uint32_t len)
{
	bignum n_tmp;

	init_bignum(n_tmp, 0, len);
	init_bignum(n1, 0, len);
	int n2_bitlen = bitlen_bignum(n2, len);
	uint32_t bit = ((uint32_t)1) << (n2_bitlen % 32);
	n1 += ((n2_bitlen + 32) / 32) - 1;
	uint32_t n2_bytelen = ((n2_bitlen - 1) / 32) * 4;
	n_tmp[n2_bytelen / 4] |= ((uint32_t)1) << ((n2_bitlen - 1) & 0x1f);

	while (n2_bitlen > 0)
	{
		n2_bitlen--;
		shl_bignum(n_tmp, 1, len);
		if (cmp_bignum(n_tmp, n2, len) != -1)
		{
			sub_bignum(n_tmp, n_tmp, n2, 0, len);
			*n1 |= bit;
		}
		bit >>= 1;
		if (bit == 0)
		{
			n1--;
			bit = 0x80000000;
		}
	}
	init_bignum(n_tmp, 0, len);
}

static void inc_bignum(bignum n, uint32_t len)
{
	while ((++*n == 0) && (--len > 0))
		n++;
}

static void init_two_dw(bignum n, uint32_t len)
{
	mov_bignum(glob1, n, len);
	glob1_bitlen = bitlen_bignum(glob1, len);
	glob1_len_x2 = (glob1_bitlen + 15) / 16;
	mov_bignum(glob1_hi, glob1 + len_bignum(glob1, len) - 2, 2);
	glob1_hi_bitlen = bitlen_bignum(glob1_hi, 2) - 32;
	shr_bignum(glob1_hi, glob1_hi_bitlen, 2);
	inv_bignum(glob1_hi_inv, glob1_hi, 2);
	shr_bignum(glob1_hi_inv, 1, 2);
	glob1_hi_bitlen = (glob1_hi_bitlen + 15) % 16 + 1;
	inc_bignum(glob1_hi_inv, 2);
	if (bitlen_bignum(glob1_hi_inv, 2) > 32) 
	{
		shr_bignum(glob1_hi_inv, 1, 2);
		glob1_hi_bitlen--;
	}
	glob1_hi_inv_lo = *(word *)glob1_hi_inv;
	glob1_hi_inv_hi = *(((word *)glob1_hi_inv) + 1);
}

static void mul_bignum_word(bignum n1, bignum n2, uint32_t mul, uint32_t len)
{
	uint32_t tmp = 0;
	for (uint32_t i = 0; i < len; i++)
	{
		tmp = mul * (*(word *)n2) + *(word *)n1 + tmp;
		*(word *)n1 = tmp;
		n1 = (uint32_t *)(((word *)n1) + 1);
		n2 = (uint32_t *)(((word *)n2) + 1);
		tmp >>= 16;
	}
	*(word *)n1 += tmp;
}

static void mul_bignum(bignum dest, bignum src1, bignum src2, uint32_t len)
{
	init_bignum(dest, 0, len * 2);
	for (uint32_t i = 0; i < len * 2; i++)
	{
		mul_bignum_word(dest, src1, *(word *)src2, len * 2);
		src2 = (uint32_t *)(((word *)src2) + 1);
		dest = (uint32_t *)(((word *)dest) + 1);
	}
}

static void not_bignum(bignum n, uint32_t len)
{
	for (uint32_t i = 0; i < len; i++)
		*(n++) = ~*n;
}

static void neg_bignum(bignum n, uint32_t len)
{
	not_bignum(n, len);
	inc_bignum(n, len);
}

static uint32_t get_mulword(bignum n)
{
	word* wn = (word *)n;
	uint32_t i = (((((((((*(wn - 1) ^ 0xffff) & 0xffff) * glob1_hi_inv_lo + 0x10000) >> 1)
      + (((*(wn-2) ^ 0xffff) * glob1_hi_inv_hi + glob1_hi_inv_hi) >> 1) + 1)
      >> 16) + ((((*(wn-1) ^ 0xffff) & 0xffff) * glob1_hi_inv_hi) >> 1) +
      (((*wn ^ 0xffff) * glob1_hi_inv_lo) >> 1) + 1) >> 14) + glob1_hi_inv_hi
      * (*wn ^ 0xffff) * 2) >> glob1_hi_bitlen;
  if (i > 0xffff) 
		i = 0xffff;
	return i & 0xffff;
}

static void dec_bignum(bignum n, uint32_t len)
{
	while ((--*n == 0xffffffff) && (--len > 0))
		n++;
}

static void calc_a_bignum(bignum n1, bignum n2, bignum n3, uint32_t len)
{
	mul_bignum(glob2, n2, n3, len);
	glob2[len * 2] = 0;
	uint32_t g2_len_x2 = len_bignum(glob2, len * 2 + 1) * 2;
	if (g2_len_x2 >= glob1_len_x2) 
	{
		inc_bignum(glob2, len * 2 + 1);
		neg_bignum(glob2, len * 2 + 1);
		uint32_t len_diff = g2_len_x2 + 1 - glob1_len_x2;
		word* esi = ((word *)glob2) + (1 + g2_len_x2 - glob1_len_x2);
		word* edi = ((word *)glob2) + (g2_len_x2 + 1);
		for (; len_diff != 0; len_diff--) 
		{
			edi--;
			word tmp = get_mulword((uint32_t *)edi);
			esi--;
			if (tmp > 0) 
			{
				mul_bignum_word((uint32_t *)esi, glob1, tmp, 2 * len);
				if ((*edi & 0x8000) == 0) 
				{
					if (sub_bignum((uint32_t *)esi, (uint32_t *)esi, glob1, 0, len)) 
						(*edi)--;
				}
			}
		}
		neg_bignum(glob2, len);
		dec_bignum(glob2, len);
	}
	mov_bignum(n1, glob2, len);
}

static void calc_a_key(bignum n1, bignum n2, bignum n3, bignum n4, uint32_t len)
{
	bignum n_tmp;
	init_bignum(n1, 1, len);
	uint32_t n4_len = len_bignum(n4, len);
	init_two_dw(n4, n4_len);
	uint32_t n3_bitlen = bitlen_bignum(n3, n4_len);
	uint32_t n3_len = (n3_bitlen + 31) / 32;
	uint32_t bit_mask = (((uint32_t)1) << ((n3_bitlen - 1) % 32)) >> 1;
	n3 += n3_len - 1;
	n3_bitlen--;
	mov_bignum(n1, n2, n4_len);
	while (--n3_bitlen != -1) 
	{
		if (bit_mask == 0) 
		{
			bit_mask = 0x80000000;
			n3--;
		}
		calc_a_bignum(n_tmp, n1, n1, n4_len);
		if (*n3 & bit_mask) 
			calc_a_bignum(n1, n_tmp, n2, n4_len);
		else 
			mov_bignum(n1, n_tmp, n4_len);
		bit_mask >>= 1;
	}
	init_bignum(n_tmp, 0, n4_len);
	init_bignum(glob1, 0, len);
	init_bignum(glob2, 0, len);
	init_bignum(glob1_hi_inv, 0, 4);
	init_bignum(glob1_hi, 0, 4);
	glob1_bitlen = 0;
	glob1_hi_bitlen = 0;
	glob1_len_x2 = 0;
	glob1_hi_inv_lo = 0;
	glob1_hi_inv_hi = 0;
}

static void process_predata(const byte* pre, uint32_t pre_len, byte *buf)
{
	bignum n2, n3;
	const uint32_t a = (pubkey.len - 1) / 8;
	while (a + 1 <= pre_len) 
	{
		init_bignum(n2, 0, 64);
		memmove(n2, pre, a + 1);
		calc_a_key(n3, n2, pubkey.key2, pubkey.key1, 64);
		
		memmove(buf, n3, a);
		
		pre_len -= a + 1;
		pre += a + 1;
		buf += a;
	}
}

void get_blowfish_key(const byte* s, span<byte> d)
{
	static bool public_key_initialized = false;
	if (!public_key_initialized)
	{
		init_pubkey();
		public_key_initialized = true;
	}
	byte key[256];
	process_predata(s, len_predata(), key);
	memcpy(d.data(), key, 56);
}
