/* $Id: patchram.h,v 1.2 2003/11/07 07:46:20 root Exp $ */

#pragma pack(1)

/* Search structure */

struct locator
{
 short offset;
 short shift;
 unsigned char c;
};

/* Area description structure */

struct area
{
 unsigned char *first;
 unsigned long pos;
 unsigned long len;
};

#pragma pack()

int verify(struct locator *l, struct area *stream, long offset);
long locate(struct locator *l, struct area *stream, unsigned long first);
void acs(unsigned char c, struct area *stream);
void aws(unsigned short w, struct area *stream);
void ads(unsigned long d, struct area *stream);

#define ac(c) acs(c, (struct area *)SSToDS(&a))
#define aw(w) aws(w, (struct area *)SSToDS(&a))
#define ad(d) ads(d, (struct area *)SSToDS(&a))
