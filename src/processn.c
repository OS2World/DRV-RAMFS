#include "includes.h"





APIRET EXPENTRY FS_PROCESSNAME (
    PSZ		pNameBuf )	/* not used */
{
  UtilEnterRamfs();
  UtilExitRamfs();
  return NO_ERROR;
}
