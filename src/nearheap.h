typedef void _ds *PNEARBLOCK;

void NearInit  (void);
int  NearAlloc (PNEARBLOCK _ss *ppBlock, USHORT cbSize);
void NearFree  (PNEARBLOCK pBlock);
