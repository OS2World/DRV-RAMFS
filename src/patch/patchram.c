/* $Id: patchram.c,v 1.2 2003/11/07 07:46:20 root Exp $ */

#include "std32.h"
#include "patchram.h"

/* Area seek operation (SEEK_SET only) */

static void aseek(struct area *a, unsigned long pos)
{
 a->pos=(pos<=a->len)?pos:a->len;
}

/* Write operation */

void acs(unsigned char c, struct area *a)
{
 a->first[a->pos++]=c;
 if(a->pos>a->len)
  a->len=a->pos;
}

/* 2-byte write operation */

void aws(unsigned short w, struct area *a)
{
 unsigned char *p;

 p=(unsigned char *)SSToDS(&w);
 acs(p[0], a);
 acs(p[1], a);
}

/* 4-byte write operation */

void ads(unsigned long d, struct area *a)
{
 unsigned short *p;

 p=(unsigned short *)SSToDS(&d);
 aws(p[0], a);
 aws(p[1], a);
}

/* Read operation */

static unsigned int aread(unsigned char *dest, unsigned int e, unsigned int c,
                          struct area *a)
{
 unsigned int j, w=0;

 for(j=e*c; j>0; j--)
 {
  if(a->pos>=a->len)
   return(w);
  *dest++=a->first[a->pos++];
  w++;
 }
 return(w);
}

/* Verifies the bytes at location to match the pattern */

int verify(struct locator *l, struct area *stream, long offset)
{
 int i;
 unsigned char c;

 if(l->offset<0)
  return(1);
 aseek(stream, offset+l->offset);
 for(i=0; i<l->shift; i++)
 {
  if(aread((unsigned char *)SSToDS(&c), 1, 1, stream)==0)
   break;
  if(c==l->c)
  {
   if(verify(l+1, stream, offset+i+l->offset))
    return(1);
   aseek(stream, offset+l->offset+i+1);
  }
 }
 return(0);
}

/* Locate a patch strip in the memory */

long locate(struct locator *l, struct area *stream, unsigned long first)
{
 unsigned char buf[512];
 int i, fetch;
 long cur_pos=first;

 aseek(stream, first);
 while((fetch=aread((unsigned char *)SSToDS(buf), 1, sizeof(buf), stream))>0)
 {
  for(i=0; i<fetch; i++)
  {
   if(buf[i]==l->c&&verify(l+1, stream, cur_pos+i))
    return(cur_pos+i);
  }
  cur_pos+=fetch;
  aseek(stream, cur_pos);
 }
 return(-1);
}
