/* $Id: main.c,v 1.4 2006/04/01 09:51:35 andrew_belov Exp $ */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ramfsutl.h"

extern int GENMAIN(ramdisk) (int argc, char **argv);
extern int GENMAIN(deldisk) (int argc, char **argv);
extern int GENMAIN(maxheap) (int argc, char **argv);
extern int GENMAIN(heapstat) (int argc, char **argv);

/* Switchboard */

static struct
{
 char *prefix;                          /* Prefix keyword */
 int has_suffix;                        /* If data is to immediately follow the prefix */
 int min_subseq;                        /* # of mandatory parameters */
 int (*handler)(int,char **);           /* Entry to submodule */
 int l;                                 /* Length (calculated) */
}
sw[]=
{
 {"/CREATE", 0, 0, &GENMAIN(ramdisk), 0}, /* Legacy mode - allows to omit "create" */
 {"/DELETE", 0, 1, &GENMAIN(deldisk), 0},
 {"/HEAP:", 1, 0, &GENMAIN(maxheap), 0},
 {"/HEAP", 0, 0, &GENMAIN(heapstat), 0},
 {NULL, 0, 0, NULL, 0}
};

/* Special parameter slots in the switchboard */

#define CREATE_PARAM                0   /* Create a new volume */
#define RAW_PARAM                 127   /* Special - marks an
                                           nprocessed parameter */

/* Displays help */

static void display_help()
{
 printf ("Syntax:\n"
         "  RAMDISK [/CREATE ][<drive>: [volume]] [/DELETE <drive>:] [/HEAP[:<size>]]\n"
         "where:\n"
         "      <drive>  is used to create a new RAMFS volume\n"
         "      /DELETE  deletes a previously created volume\n"
         "        /HEAP  queries the memory allocation statistics\n"
         " /HEAP:<size>  changes the memory allocation quota\n"
         "\n"
         "Examples:\n"
         "  RAMDISK /create d:    creates RAM disk d:\n"
         "  RAMDISK d:            creates RAM disk d: (shorthand for the above)\n"
         "  RAMDISK d: vollabel   creates RAM disk d: with label \"vollabel\"\n"
         "  RAMDISK /delete d:    deletes RAM disk d:\n"
         "  RAMDISK /heap:128M    limits RAMFS memory consumption to 128 megabytes\n"
         "  RAMDISK /create x: mylabel /heap:1G\n"
         "                        creates disk X: labeled \"mylabel\" and limits\n"
         "                        RAMFS memory usage to 1 gigabyte\n");
 exit (1);
}

/* A handy malloc() */

static void *malloc_msg(unsigned int sz)
{
 void *rc;

 if((rc=malloc(sz))==NULL)
 {
  printf("Low memory - failed to allocate %u bytes\n");
  exit(100);
 }
 return(rc);
}

/* Multiplex main() */

int main(int argc, char **argv)
{
 char **margv;
 int margc=1;
 char *proc_argc;
 int i, j;
 int k;
 int rc=0, mrc=0;

 if(argc<2)
  display_help();
 proc_argc=malloc_msg(sizeof(*proc_argc)*(argc+1));
 margv=malloc_msg(sizeof(*margv)*(argc+1));
 margv[0]=argv[0];
 memset(proc_argc, RAW_PARAM, argc*sizeof(*proc_argc));
 /* Update lengths in the switchboard */
 for(j=0; sw[j].prefix!=NULL; j++)
  sw[j].l=strlen(sw[j].prefix);
 /* Locate parameter keywords */
 for(i=1; i<argc; i++)
 {
  for(j=0; sw[j].prefix!=NULL; j++)
  {
   if(!memicmp(argv[i], sw[j].prefix, sw[j].l)&&
      (!sw[j].has_suffix&&argv[i][sw[j].l]=='\0'||
       sw[j].has_suffix&&argv[i][sw[j].l]!='\0'))
   {
    proc_argc[i]=j;
    break;
   }
  }
 }
 /* Cook the parameters */
 k=0;
 j=CREATE_PARAM;
 for(i=1; i<=argc; i++)
 {
  if(argv[i]==NULL||proc_argc[i]!=RAW_PARAM)
  {
   margv[margc]=NULL;
   if(margc<=sw[j].min_subseq)
    printf("Insufficient parameters for `%s`\n", argv[k]);
   else
    rc=sw[j].handler(margc, margv);
   margc=1;
   /* Back to the grammar school. Abandon any further parameters. */
   if(rc==BADSYNTAX)
    display_help();
   /* Terminate with an error if anything goes wrong. */
   if(rc)
    break;
   /* Bail out if done with command line */
   if(argv[i]==NULL)
    break;
   j=proc_argc[i];
   if(sw[j].has_suffix)
    margv[margc++]=argv[i]+sw[j].l;    
  }
  else
   margv[margc++]=argv[i];
 }
 free(margv);
 free(proc_argc);
 return(rc);
}
