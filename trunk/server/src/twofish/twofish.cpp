/* 
   compiler is gcc(egcs-2.91.66)
   flags are -O3 -fomit-frame-pointer -Wall 
   Processor is 233Mhz Pentium II (Deschutes)
   OS is Linux 2.2.16

   Max encryption speed I've seen (in mulit-user mode even, although single
   user mode probably won't save more than a couple clocks):

   encs/sec = 506115.904591
   bytes/sec = 8097854.473457
   KB/sec = 7908.061009
   MB/sec = 7.722716
   approx clocks/enc (for 233Mhz) = 461.027466
      
   I easily beat the best C implementations (the best being MSC @ 600 clocks),
   so the target is the assembly implementations...

   according to twofish docs, fully tuned *assembly* (in clocks):
   compiled is 285          (shouldn't be able to do this)  (12.5 MB/sec)
   full keyed is 315        (if I get to 460, maybe this is possible but 
                             I *really* doubt it)  (11.3 MB/sec)
   partially keyed is 460   (I'm *really* close) (7.7 MB/sec)
   minimal keying is 720    (I've beat this -their C did too) (4.9 MB/sec)

*/

/* 
   gcc is smart enough to convert these to roll instructions.  If you want
   to see for yourself, either do gcc -O3 -S, or change the |'s to +'s and 
   see how slow things get (you lose about 30-50 clocks) :).
*/

#include "twofish.h"

#define u8 unsigned char
u8 RS[4][8] = {
    { 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, },
    { 0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5, },
    { 0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19, },
    { 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03, },
};

u8 Q0[] = {
    0xA9, 0x67, 0xB3, 0xE8, 0x04, 0xFD, 0xA3, 0x76, 
    0x9A, 0x92, 0x80, 0x78, 0xE4, 0xDD, 0xD1, 0x38, 
    0x0D, 0xC6, 0x35, 0x98, 0x18, 0xF7, 0xEC, 0x6C, 
    0x43, 0x75, 0x37, 0x26, 0xFA, 0x13, 0x94, 0x48, 
    0xF2, 0xD0, 0x8B, 0x30, 0x84, 0x54, 0xDF, 0x23, 
    0x19, 0x5B, 0x3D, 0x59, 0xF3, 0xAE, 0xA2, 0x82, 
    0x63, 0x01, 0x83, 0x2E, 0xD9, 0x51, 0x9B, 0x7C, 
    0xA6, 0xEB, 0xA5, 0xBE, 0x16, 0x0C, 0xE3, 0x61, 
    0xC0, 0x8C, 0x3A, 0xF5, 0x73, 0x2C, 0x25, 0x0B, 
    0xBB, 0x4E, 0x89, 0x6B, 0x53, 0x6A, 0xB4, 0xF1, 
    0xE1, 0xE6, 0xBD, 0x45, 0xE2, 0xF4, 0xB6, 0x66, 
    0xCC, 0x95, 0x03, 0x56, 0xD4, 0x1C, 0x1E, 0xD7, 
    0xFB, 0xC3, 0x8E, 0xB5, 0xE9, 0xCF, 0xBF, 0xBA, 
    0xEA, 0x77, 0x39, 0xAF, 0x33, 0xC9, 0x62, 0x71, 
    0x81, 0x79, 0x09, 0xAD, 0x24, 0xCD, 0xF9, 0xD8, 
    0xE5, 0xC5, 0xB9, 0x4D, 0x44, 0x08, 0x86, 0xE7, 
    0xA1, 0x1D, 0xAA, 0xED, 0x06, 0x70, 0xB2, 0xD2, 
    0x41, 0x7B, 0xA0, 0x11, 0x31, 0xC2, 0x27, 0x90, 
    0x20, 0xF6, 0x60, 0xFF, 0x96, 0x5C, 0xB1, 0xAB, 
    0x9E, 0x9C, 0x52, 0x1B, 0x5F, 0x93, 0x0A, 0xEF, 
    0x91, 0x85, 0x49, 0xEE, 0x2D, 0x4F, 0x8F, 0x3B, 
    0x47, 0x87, 0x6D, 0x46, 0xD6, 0x3E, 0x69, 0x64, 
    0x2A, 0xCE, 0xCB, 0x2F, 0xFC, 0x97, 0x05, 0x7A, 
    0xAC, 0x7F, 0xD5, 0x1A, 0x4B, 0x0E, 0xA7, 0x5A, 
    0x28, 0x14, 0x3F, 0x29, 0x88, 0x3C, 0x4C, 0x02, 
    0xB8, 0xDA, 0xB0, 0x17, 0x55, 0x1F, 0x8A, 0x7D, 
    0x57, 0xC7, 0x8D, 0x74, 0xB7, 0xC4, 0x9F, 0x72, 
    0x7E, 0x15, 0x22, 0x12, 0x58, 0x07, 0x99, 0x34, 
    0x6E, 0x50, 0xDE, 0x68, 0x65, 0xBC, 0xDB, 0xF8, 
    0xC8, 0xA8, 0x2B, 0x40, 0xDC, 0xFE, 0x32, 0xA4, 
    0xCA, 0x10, 0x21, 0xF0, 0xD3, 0x5D, 0x0F, 0x00, 
    0x6F, 0x9D, 0x36, 0x42, 0x4A, 0x5E, 0xC1, 0xE0, 
    };

u8 Q1[] = {
    0x75, 0xF3, 0xC6, 0xF4, 0xDB, 0x7B, 0xFB, 0xC8, 
    0x4A, 0xD3, 0xE6, 0x6B, 0x45, 0x7D, 0xE8, 0x4B, 
    0xD6, 0x32, 0xD8, 0xFD, 0x37, 0x71, 0xF1, 0xE1, 
    0x30, 0x0F, 0xF8, 0x1B, 0x87, 0xFA, 0x06, 0x3F, 
    0x5E, 0xBA, 0xAE, 0x5B, 0x8A, 0x00, 0xBC, 0x9D, 
    0x6D, 0xC1, 0xB1, 0x0E, 0x80, 0x5D, 0xD2, 0xD5, 
    0xA0, 0x84, 0x07, 0x14, 0xB5, 0x90, 0x2C, 0xA3, 
    0xB2, 0x73, 0x4C, 0x54, 0x92, 0x74, 0x36, 0x51, 
    0x38, 0xB0, 0xBD, 0x5A, 0xFC, 0x60, 0x62, 0x96, 
    0x6C, 0x42, 0xF7, 0x10, 0x7C, 0x28, 0x27, 0x8C, 
    0x13, 0x95, 0x9C, 0xC7, 0x24, 0x46, 0x3B, 0x70, 
    0xCA, 0xE3, 0x85, 0xCB, 0x11, 0xD0, 0x93, 0xB8, 
    0xA6, 0x83, 0x20, 0xFF, 0x9F, 0x77, 0xC3, 0xCC, 
    0x03, 0x6F, 0x08, 0xBF, 0x40, 0xE7, 0x2B, 0xE2, 
    0x79, 0x0C, 0xAA, 0x82, 0x41, 0x3A, 0xEA, 0xB9, 
    0xE4, 0x9A, 0xA4, 0x97, 0x7E, 0xDA, 0x7A, 0x17, 
    0x66, 0x94, 0xA1, 0x1D, 0x3D, 0xF0, 0xDE, 0xB3, 
    0x0B, 0x72, 0xA7, 0x1C, 0xEF, 0xD1, 0x53, 0x3E, 
    0x8F, 0x33, 0x26, 0x5F, 0xEC, 0x76, 0x2A, 0x49, 
    0x81, 0x88, 0xEE, 0x21, 0xC4, 0x1A, 0xEB, 0xD9, 
    0xC5, 0x39, 0x99, 0xCD, 0xAD, 0x31, 0x8B, 0x01, 
    0x18, 0x23, 0xDD, 0x1F, 0x4E, 0x2D, 0xF9, 0x48, 
    0x4F, 0xF2, 0x65, 0x8E, 0x78, 0x5C, 0x58, 0x19, 
    0x8D, 0xE5, 0x98, 0x57, 0x67, 0x7F, 0x05, 0x64, 
    0xAF, 0x63, 0xB6, 0xFE, 0xF5, 0xB7, 0x3C, 0xA5, 
    0xCE, 0xE9, 0x68, 0x44, 0xE0, 0x4D, 0x43, 0x69, 
    0x29, 0x2E, 0xAC, 0x15, 0x59, 0xA8, 0x0A, 0x9E, 
    0x6E, 0x47, 0xDF, 0x34, 0x35, 0x6A, 0xCF, 0xDC, 
    0x22, 0xC9, 0xC0, 0x9B, 0x89, 0xD4, 0xED, 0xAB, 
    0x12, 0xA2, 0x0D, 0x52, 0xBB, 0x02, 0x2F, 0xA9, 
    0xD7, 0x61, 0x1E, 0xB4, 0x50, 0x04, 0xF6, 0xC2, 
    0x16, 0x25, 0x86, 0x56, 0x55, 0x09, 0xBE, 0x91, 
    };

u8 mult5B[] = {
    0x00, 0x5B, 0xB6, 0xED, 0x05, 0x5E, 0xB3, 0xE8, 
    0x0A, 0x51, 0xBC, 0xE7, 0x0F, 0x54, 0xB9, 0xE2, 
    0x14, 0x4F, 0xA2, 0xF9, 0x11, 0x4A, 0xA7, 0xFC, 
    0x1E, 0x45, 0xA8, 0xF3, 0x1B, 0x40, 0xAD, 0xF6, 
    0x28, 0x73, 0x9E, 0xC5, 0x2D, 0x76, 0x9B, 0xC0, 
    0x22, 0x79, 0x94, 0xCF, 0x27, 0x7C, 0x91, 0xCA, 
    0x3C, 0x67, 0x8A, 0xD1, 0x39, 0x62, 0x8F, 0xD4, 
    0x36, 0x6D, 0x80, 0xDB, 0x33, 0x68, 0x85, 0xDE, 
    0x50, 0x0B, 0xE6, 0xBD, 0x55, 0x0E, 0xE3, 0xB8, 
    0x5A, 0x01, 0xEC, 0xB7, 0x5F, 0x04, 0xE9, 0xB2, 
    0x44, 0x1F, 0xF2, 0xA9, 0x41, 0x1A, 0xF7, 0xAC, 
    0x4E, 0x15, 0xF8, 0xA3, 0x4B, 0x10, 0xFD, 0xA6, 
    0x78, 0x23, 0xCE, 0x95, 0x7D, 0x26, 0xCB, 0x90, 
    0x72, 0x29, 0xC4, 0x9F, 0x77, 0x2C, 0xC1, 0x9A, 
    0x6C, 0x37, 0xDA, 0x81, 0x69, 0x32, 0xDF, 0x84, 
    0x66, 0x3D, 0xD0, 0x8B, 0x63, 0x38, 0xD5, 0x8E, 
    0xA0, 0xFB, 0x16, 0x4D, 0xA5, 0xFE, 0x13, 0x48, 
    0xAA, 0xF1, 0x1C, 0x47, 0xAF, 0xF4, 0x19, 0x42, 
    0xB4, 0xEF, 0x02, 0x59, 0xB1, 0xEA, 0x07, 0x5C, 
    0xBE, 0xE5, 0x08, 0x53, 0xBB, 0xE0, 0x0D, 0x56, 
    0x88, 0xD3, 0x3E, 0x65, 0x8D, 0xD6, 0x3B, 0x60, 
    0x82, 0xD9, 0x34, 0x6F, 0x87, 0xDC, 0x31, 0x6A, 
    0x9C, 0xC7, 0x2A, 0x71, 0x99, 0xC2, 0x2F, 0x74, 
    0x96, 0xCD, 0x20, 0x7B, 0x93, 0xC8, 0x25, 0x7E, 
    0xF0, 0xAB, 0x46, 0x1D, 0xF5, 0xAE, 0x43, 0x18, 
    0xFA, 0xA1, 0x4C, 0x17, 0xFF, 0xA4, 0x49, 0x12, 
    0xE4, 0xBF, 0x52, 0x09, 0xE1, 0xBA, 0x57, 0x0C, 
    0xEE, 0xB5, 0x58, 0x03, 0xEB, 0xB0, 0x5D, 0x06, 
    0xD8, 0x83, 0x6E, 0x35, 0xDD, 0x86, 0x6B, 0x30, 
    0xD2, 0x89, 0x64, 0x3F, 0xD7, 0x8C, 0x61, 0x3A, 
    0xCC, 0x97, 0x7A, 0x21, 0xC9, 0x92, 0x7F, 0x24, 
    0xC6, 0x9D, 0x70, 0x2B, 0xC3, 0x98, 0x75, 0x2E, 
    };

u8 multEF[] = {
    0x00, 0xEF, 0xB7, 0x58, 0x07, 0xE8, 0xB0, 0x5F, 
    0x0E, 0xE1, 0xB9, 0x56, 0x09, 0xE6, 0xBE, 0x51, 
    0x1C, 0xF3, 0xAB, 0x44, 0x1B, 0xF4, 0xAC, 0x43, 
    0x12, 0xFD, 0xA5, 0x4A, 0x15, 0xFA, 0xA2, 0x4D, 
    0x38, 0xD7, 0x8F, 0x60, 0x3F, 0xD0, 0x88, 0x67, 
    0x36, 0xD9, 0x81, 0x6E, 0x31, 0xDE, 0x86, 0x69, 
    0x24, 0xCB, 0x93, 0x7C, 0x23, 0xCC, 0x94, 0x7B, 
    0x2A, 0xC5, 0x9D, 0x72, 0x2D, 0xC2, 0x9A, 0x75, 
    0x70, 0x9F, 0xC7, 0x28, 0x77, 0x98, 0xC0, 0x2F, 
    0x7E, 0x91, 0xC9, 0x26, 0x79, 0x96, 0xCE, 0x21, 
    0x6C, 0x83, 0xDB, 0x34, 0x6B, 0x84, 0xDC, 0x33, 
    0x62, 0x8D, 0xD5, 0x3A, 0x65, 0x8A, 0xD2, 0x3D, 
    0x48, 0xA7, 0xFF, 0x10, 0x4F, 0xA0, 0xF8, 0x17, 
    0x46, 0xA9, 0xF1, 0x1E, 0x41, 0xAE, 0xF6, 0x19, 
    0x54, 0xBB, 0xE3, 0x0C, 0x53, 0xBC, 0xE4, 0x0B, 
    0x5A, 0xB5, 0xED, 0x02, 0x5D, 0xB2, 0xEA, 0x05, 
    0xE0, 0x0F, 0x57, 0xB8, 0xE7, 0x08, 0x50, 0xBF, 
    0xEE, 0x01, 0x59, 0xB6, 0xE9, 0x06, 0x5E, 0xB1, 
    0xFC, 0x13, 0x4B, 0xA4, 0xFB, 0x14, 0x4C, 0xA3, 
    0xF2, 0x1D, 0x45, 0xAA, 0xF5, 0x1A, 0x42, 0xAD, 
    0xD8, 0x37, 0x6F, 0x80, 0xDF, 0x30, 0x68, 0x87, 
    0xD6, 0x39, 0x61, 0x8E, 0xD1, 0x3E, 0x66, 0x89, 
    0xC4, 0x2B, 0x73, 0x9C, 0xC3, 0x2C, 0x74, 0x9B, 
    0xCA, 0x25, 0x7D, 0x92, 0xCD, 0x22, 0x7A, 0x95, 
    0x90, 0x7F, 0x27, 0xC8, 0x97, 0x78, 0x20, 0xCF, 
    0x9E, 0x71, 0x29, 0xC6, 0x99, 0x76, 0x2E, 0xC1, 
    0x8C, 0x63, 0x3B, 0xD4, 0x8B, 0x64, 0x3C, 0xD3, 
    0x82, 0x6D, 0x35, 0xDA, 0x85, 0x6A, 0x32, 0xDD, 
    0xA8, 0x47, 0x1F, 0xF0, 0xAF, 0x40, 0x18, 0xF7, 
    0xA6, 0x49, 0x11, 0xFE, 0xA1, 0x4E, 0x16, 0xF9, 
    0xB4, 0x5B, 0x03, 0xEC, 0xB3, 0x5C, 0x04, 0xEB, 
    0xBA, 0x55, 0x0D, 0xE2, 0xBD, 0x52, 0x0A, 0xE5, 
    };


#define ROL(x,n) (((x) << ((n) & 0x1F)) | ((x) >> (32-((n) & 0x1F))))
#define ROR(x,n) (((x) >> ((n) & 0x1F)) | ((x) << (32-((n) & 0x1F))))

#if BIG_ENDIAN == 1
#define BSWAP(x) (((ROR(x,8) & 0xFF00FF00) | (ROL(x,8) & 0x00FF00FF)))
#else
#define BSWAP(x) (x)
#endif

#define _b(x, N) (((x) >> (N*8)) & 0xFF)

/* just casting to byte (instead of masking with 0xFF saves *tons* of clocks 
   (around 50) */
#define b0(x) ((BYTE)(x))
/* this saved 10 clocks */
#define b1(x) ((BYTE)((x) >> 8))
/* use byte cast here saves around 10 clocks */
#define b2(x) (BYTE)((x) >> 16)
/* don't need to mask since all bits are in lower 8 - byte cast here saves
   nothing, but hey, what the hell, it doesn't hurt any */
#define b3(x) (BYTE)((x) >> 24)  

#define BYTEARRAY_TO_U32(r) ((r[0] << 24) ^ (r[1] << 16) ^ (r[2] << 8) ^ r[3])
#define BYTES_TO_U32(r0, r1, r2, r3) ((r0 << 24) ^ (r1 << 16) ^ (r2 << 8) ^ r3)

void printSubkeys(u32 K[40])
{
    int i;
    printf("round subkeys\n");
    for (i=0;i<40;i+=2)
	printf("%08X %08X\n", K[i], K[i+1]);
}

/* 
   multiply two polynomials represented as u32's, actually called with BYTES,
   but since I'm not really going to too much work to optimize key setup (since
   raw encryption speed is what I'm after), big deal.
*/
u32 polyMult(u32 a, u32 b)
{
    u32 t=0;
    while (a)
    {
	/*printf("A=%X  B=%X  T=%X\n", a, b, t);*/
	if (a&1) t^=b;
	b <<= 1;
	a >>= 1;
    }
    return t;
}
	    
/* take the polynomial t and return the t % modulus in GF(256) */
u32 gfMod(u32 t, u32 modulus)
{
    int i;
    u32 tt;

    modulus <<= 7;
    for (i = 0; i < 8; i++)
    {
	tt = t ^ modulus;
	if (tt < t) t = tt;
	modulus >>= 1;
    }
    return t;
}

/*multiply a and b and return the modulus */
#define gfMult(a, b, modulus) gfMod(polyMult(a, b), modulus)

/* return a u32 containing the result of multiplying the RS Code matrix
   by the sd matrix
*/
u32 RSMatrixMultiply(BYTE sd[8])
{
    int j, k;
    BYTE t;
    BYTE result[4];

    for (j = 0; j < 4; j++)
    {
	t = 0;
	for (k = 0; k < 8; k++)
	{
	    /*printf("t=%X  %X\n", t, gfMult(RS[j][k], sd[k], RS_MOD));*/
	    t ^= gfMult(RS[j][k], sd[k], RS_MOD);
	}
	result[3-j] = t;
    }
    return BYTEARRAY_TO_U32(result);
}

/* the Zero-keyed h function (used by the key setup routine) */
u32 h(u32 X, u32 L[4], int k)
{
    BYTE y0, y1, y2, y3;
    BYTE z0, z1, z2, z3;
    y0 = b0(X);
    y1 = b1(X);
    y2 = b2(X);
    y3 = b3(X);

    switch(k)
    {
	case 4:
	    y0 = Q1[y0] ^ b0(L[3]);
	    y1 = Q0[y1] ^ b1(L[3]);
	    y2 = Q0[y2] ^ b2(L[3]);
	    y3 = Q1[y3] ^ b3(L[3]);
	case 3:
	    y0 = Q1[y0] ^ b0(L[2]);
	    y1 = Q1[y1] ^ b1(L[2]);
	    y2 = Q0[y2] ^ b2(L[2]);
	    y3 = Q0[y3] ^ b3(L[2]);
	case 2:
	    y0 = Q1[  Q0 [ Q0[y0] ^ b0(L[1]) ] ^ b0(L[0]) ];
	    y1 = Q0[  Q0 [ Q1[y1] ^ b1(L[1]) ] ^ b1(L[0]) ];
	    y2 = Q1[  Q1 [ Q0[y2] ^ b2(L[1]) ] ^ b2(L[0]) ];
	    y3 = Q0[  Q1 [ Q1[y3] ^ b3(L[1]) ] ^ b3(L[0]) ];
    }

    /* inline the MDS matrix multiply */
    z0 = multEF[y0] ^ y1 ^         multEF[y2] ^ mult5B[y3]; 
    z1 = multEF[y0] ^ mult5B[y1] ^ y2 ^         multEF[y3]; 
    z2 = mult5B[y0] ^ multEF[y1] ^ multEF[y2] ^ y3; 
    z3 = y0 ^         multEF[y1] ^ mult5B[y2] ^ mult5B[y3]; 

    return BYTES_TO_U32(z0, z1, z2, z3);
}

/* given the Sbox keys, create the fully keyed QF */
void fullKey(u32 L[4], int k, u32 QF[4][256])
{
    BYTE y0, y1, y2, y3;

    int i;
    
    /* for all input values to the Q permutations */
    for (i=0; i<256; i++)
    {
	/* run the Q permutations */
	y0 = i; y1=i; y2=i; y3=i;
	switch(k)
    	{
    	    case 4:
    		y0 = Q1[y0] ^ b0(L[3]);
    		y1 = Q0[y1] ^ b1(L[3]);
    		y2 = Q0[y2] ^ b2(L[3]);
    		y3 = Q1[y3] ^ b3(L[3]);
    	    case 3:
    		y0 = Q1[y0] ^ b0(L[2]);
    		y1 = Q1[y1] ^ b1(L[2]);
    		y2 = Q0[y2] ^ b2(L[2]);
    		y3 = Q0[y3] ^ b3(L[2]);
    	    case 2:
    		y0 = Q1[  Q0 [ Q0[y0] ^ b0(L[1]) ] ^ b0(L[0]) ];
    		y1 = Q0[  Q0 [ Q1[y1] ^ b1(L[1]) ] ^ b1(L[0]) ];
    		y2 = Q1[  Q1 [ Q0[y2] ^ b2(L[1]) ] ^ b2(L[0]) ];
    		y3 = Q0[  Q1 [ Q1[y3] ^ b3(L[1]) ] ^ b3(L[0]) ];
    	}
	
	/* now do the partial MDS matrix multiplies */
	QF[0][i] = ((multEF[y0] << 24) 
		    | (multEF[y0] << 16) 
		    | (mult5B[y0] << 8)
		    | y0);
	QF[1][i] = ((y1 << 24) 
		    | (mult5B[y1] << 16) 
		    | (multEF[y1] << 8)
		    | multEF[y1]);
	QF[2][i] = ((multEF[y2] << 24) 
		    | (y2 << 16) 
		    | (multEF[y2] << 8)
		    | mult5B[y2]);
	QF[3][i] = ((mult5B[y3] << 24) 
		    | (multEF[y3] << 16)
		    | (y3 << 8) 
		    | mult5B[y3]);
    }
}

void printRound(int round, u32 R0, u32 R1, u32 R2, u32 R3, u32 K1, u32 K2)
{
    printf("round[%d] ['0x%08XL', '0x%08XL', '0x%08XL', '0x%08XL']\n", 
	   round, R0, R1, R2, R3);

}

/* fully keyed h (aka g) function */
#define fkh(X) (S[0][b0(X)]^S[1][b1(X)]^S[2][b2(X)]^S[3][b3(X)])

/* one encryption round */
#define ENC_ROUND(R0, R1, R2, R3, round)\
    T0 = fkh(R0);\
    T1 = fkh(ROL(R1, 8));\
    R2 = ROR(R2 ^ (T1 + T0 + K[2*round+8]), 1);\
    R3 = ROL(R3, 1) ^ (2*T1 + T0 + K[2*round+9]); 

void encrypt(u32 K[40], u32 S[4][256], BYTE PT[16])
{
    u32 R0, R1, R2, R3;
    u32 T0, T1;

    /* load/byteswap/whiten input */
    R3 = K[3] ^ BSWAP(((u32*)PT)[3]);
    R2 = K[2] ^ BSWAP(((u32*)PT)[2]);
    R1 = K[1] ^ BSWAP(((u32*)PT)[1]);
    R0 = K[0] ^ BSWAP(((u32*)PT)[0]);

    ENC_ROUND(R0, R1, R2, R3, 0);
    ENC_ROUND(R2, R3, R0, R1, 1);
    ENC_ROUND(R0, R1, R2, R3, 2);
    ENC_ROUND(R2, R3, R0, R1, 3);
    ENC_ROUND(R0, R1, R2, R3, 4);
    ENC_ROUND(R2, R3, R0, R1, 5);
    ENC_ROUND(R0, R1, R2, R3, 6);
    ENC_ROUND(R2, R3, R0, R1, 7);
    ENC_ROUND(R0, R1, R2, R3, 8);
    ENC_ROUND(R2, R3, R0, R1, 9);
    ENC_ROUND(R0, R1, R2, R3, 10);
    ENC_ROUND(R2, R3, R0, R1, 11);
    ENC_ROUND(R0, R1, R2, R3, 12);
    ENC_ROUND(R2, R3, R0, R1, 13);
    ENC_ROUND(R0, R1, R2, R3, 14);
    ENC_ROUND(R2, R3, R0, R1, 15);

    /* load/byteswap/whiten output */
    ((u32*)PT)[3] = BSWAP(R1 ^ K[7]);
    ((u32*)PT)[2] = BSWAP(R0 ^ K[6]);
    ((u32*)PT)[1] = BSWAP(R3 ^ K[5]);
    ((u32*)PT)[0] = BSWAP(R2 ^ K[4]);
}

/* one decryption round */
#define DEC_ROUND(R0, R1, R2, R3, round) \
    T0 = fkh(R0); \
    T1 = fkh(ROL(R1, 8)); \
    R2 = ROL(R2, 1) ^ (T0 + T1 + K[2*round+8]); \
    R3 = ROR(R3 ^ (T0 + 2*T1 + K[2*round+9]), 1); 

inline void decrypt(u32 K[40], u32 S[4][256], BYTE PT[16])
{
    u32 T0, T1;
    u32 R0, R1, R2, R3;

    /* load/byteswap/whiten input */
    R3 = K[7] ^ BSWAP(((u32*)PT)[3]);
    R2 = K[6] ^ BSWAP(((u32*)PT)[2]);
    R1 = K[5] ^ BSWAP(((u32*)PT)[1]);
    R0 = K[4] ^ BSWAP(((u32*)PT)[0]);

    DEC_ROUND(R0, R1, R2, R3, 15);
    DEC_ROUND(R2, R3, R0, R1, 14);
    DEC_ROUND(R0, R1, R2, R3, 13);
    DEC_ROUND(R2, R3, R0, R1, 12);
    DEC_ROUND(R0, R1, R2, R3, 11);
    DEC_ROUND(R2, R3, R0, R1, 10);
    DEC_ROUND(R0, R1, R2, R3, 9);
    DEC_ROUND(R2, R3, R0, R1, 8);
    DEC_ROUND(R0, R1, R2, R3, 7);
    DEC_ROUND(R2, R3, R0, R1, 6);
    DEC_ROUND(R0, R1, R2, R3, 5);
    DEC_ROUND(R2, R3, R0, R1, 4);
    DEC_ROUND(R0, R1, R2, R3, 3);
    DEC_ROUND(R2, R3, R0, R1, 2);
    DEC_ROUND(R0, R1, R2, R3, 1);
    DEC_ROUND(R2, R3, R0, R1, 0);

    /* load/byteswap/whiten output */
    ((u32*)PT)[3] = BSWAP(R1 ^ K[3]);
    ((u32*)PT)[2] = BSWAP(R0 ^ K[2]);
    ((u32*)PT)[1] = BSWAP(R3 ^ K[1]);
    ((u32*)PT)[0] = BSWAP(R2 ^ K[0]);

}

/* the key schedule routine */
void keySched(BYTE M[], int N, u32 **S, u32 K[40], int *k)
{
    u32 Mo[4], Me[4];
    int i, j;
    BYTE vector[8];
    u32 A, B;

    *k = (N + 63) / 64;
    *S = (u32*)malloc(sizeof(u32) * (*k));

    for (i = 0; i < *k; i++)
    {
	Me[i] = BSWAP(((u32*)M)[2*i]);
	Mo[i] = BSWAP(((u32*)M)[2*i+1]);
    }

    for (i = 0; i < *k; i++)
    {
	for (j = 0; j < 4; j++) vector[j] = _b(Me[i], j);
	for (j = 0; j < 4; j++) vector[j+4] = _b(Mo[i], j);
	(*S)[(*k)-i-1] = RSMatrixMultiply(vector);
    }
    for (i = 0; i < 20; i++)
    {
	A = h(2*i*RHO, Me, *k);
	B = ROL(h(2*i*RHO + RHO, Mo, *k), 8);
	K[2*i] = A+B;
	K[2*i+1] = ROL(A + 2*B, 9);
    }
}	

/***********************************************************************
  TESTING FUNCTIONS AND STUFF STARTS HERE
***********************************************************************/
void printHex(BYTE b[], int lim)
{
    int i;
    for (i=0; i<lim;i++) 
	printf("%02X", (u32)b[i]);
}

/*
int main()
{
    u32 *S;
    u32 K[40];
    int k;
    u32 QF[4][256];
    BYTE text[16];
    BYTE key[32];

    memset(text, 0, 16);
    memset(key, 0, 32);
    keySched(key, 128, &S, K, &k);
    fullKey(S, k, QF);
    free(S);
    puts("before"); printHex(text, 16); printf("\n");
    encrypt(K, QF, text);
    puts("after"); printHex(text, 16); printf("\n");
    memcpy(key,  "\x9F\x58\x9F\x5C\xF6\x12\x2C\x32"
	         "\xB6\xBF\xEC\x2F\x2A\xE8\xC3\x5A", 16);
    memcpy(text, "\xD4\x91\xDB\x16\xE7\xB1\xC3\x9E"
	         "\x86\xCB\x08\x6B\x78\x9F\x54\x19", 16);
    keySched(key, 128, &S, K, &k);
    fullKey(S, k, QF);
    free(S);
    printf("before-->"); printHex(text, 16); printf("\n");
    encrypt(K, QF, text);
    printf("after--->"); printHex(text, 16); printf("\n");
    decrypt(K, QF, text);
    printf("after--->"); printHex(text, 16); printf("\n");

    return 0;
}
*/