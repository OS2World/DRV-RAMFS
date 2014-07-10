extern int findnext_resume;

int FindStoreEntry (PCHAR _ss *ppData, USHORT _ss *pcbData, int level,
		    int flag, USHORT usAttrPattern, PEAOP pEAOP, PDIRENTRY pEntry);

int FindEntries (PSEARCH pSearch, PSZ pRefEntry, PDIRENTRY pEntry, PCHAR pData,
		 USHORT cbData, PUSHORT pcMatch, USHORT level, USHORT flags);
