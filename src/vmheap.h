FLAT   _pascal _near VMVirtToFlat (void *p);
FLAT   _pascal _near VMAlloc (int fForce, ULONG cbSize);
void   _pascal _near VMFree (FLAT flatBlock, ULONG cbSize);
UCHAR  _pascal _near VMReadUChar (FLAT flatSrc);
USHORT _pascal _near VMReadUShort (FLAT flatSrc);
void   _pascal _near VMReadBlk (BLOCK _ss *pBlk, FLAT flatBlk);
void   _pascal _near VMWriteBlk (FLAT flatBlk, BLOCK _ss *pBlk);
void   _pascal _near VMRead (void *pDest, FLAT flatSrc, USHORT cbLen);
void   _pascal _near VMWrite (FLAT flatDest, void *pSrc, USHORT cbLen);
void   _pascal _near VMCopy (FLAT flatDest, FLAT flatSrc, ULONG cbLen);
void   _pascal _near VMInit ();

#ifdef MAX_HEAP
extern unsigned long gcbHeapUsed;
extern unsigned long gcbHeapMax;
extern unsigned long gcVMBlocks;
extern unsigned long gcVMFrees;
extern unsigned long gcVMAllocs;
#endif
