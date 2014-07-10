#include "includes.h"





VOID EXPENTRY FS_EXIT (
    USHORT	uid,
    USHORT	pid,
    USHORT	pdb )
{
  UtilEnterRamfs();
#if 0

  DEBUG_PRINTF3 ("FS_EXIT?  uid=%d, pid=%d, pdb=%d", uid, pid, pdb);

  DEBUG_PRINTF0 (" => void\r\n");

#endif
  UtilExitRamfs();
}
