/* rcopy.c: a small file copying program.
 *
 * Creates the destination file in its final size before copying data.
 * This should improve speed when copying large files to a ramfs drive
 * under OS/2, compared to using COPY or XCOPY.
 *
 * Karl Olsen, kro@post3.tele.dk
 */

/* Note: this program becomes obsolete with the performance patch.
   AAB 21/10/2002 */
 
#include <stdio.h>


char buffer[32768u];


int main (int argc, char **argv)
{
  FILE *srcfile;
  FILE *dstfile;
  long srcsize;
  size_t bytesread;
  size_t byteswritten;
  
  if (argc != 3)
  {
    fprintf (stderr, "Syntax: RCOPY sourcefile destfile\n");
    return 1;
  }
  
  /* Open the source file */
  srcfile = fopen (argv[1], "rb");
  if (srcfile == NULL)
  {
    fprintf (stderr, "Error: Cannot open source file %s.\n", argv[1]);
    return 1;
  }
  
  /* Find the size of the source file */
  fseek (srcfile, 0, SEEK_END);
  srcsize = ftell (srcfile);
  fseek (srcfile, 0, SEEK_SET);
  
  /* Create the destination file */
  dstfile = fopen (argv[2], "wb");
  if (dstfile == NULL)
  {
    fclose (srcfile);
    fprintf (stderr, "Error: Cannot create destination file %s.\n", argv[2]);
    return 1;
  }
  
  /* Set the size of the destination file by writing a byte to the last
   * position in it */
  if (srcsize != 0L)
  {
    fseek (dstfile, srcsize-1, SEEK_SET);
    if (fwrite (buffer, 1, 1, dstfile) != 1)
    {
      fclose (srcfile);
      fclose (dstfile);
      fprintf (stderr, "Error: Cannot set size of destination file.\n");
      return 1;
    }
    fseek (dstfile, 0, SEEK_SET);
  }
  
  /* Copy chunks of data from source to destination */
  while (srcsize)
  {
    bytesread = fread (buffer, 1, sizeof(buffer), srcfile);
    if (bytesread == 0)
    {
      fclose (srcfile);
      fclose (dstfile);
      fprintf (stderr, "Error: Read 0 bytes from source file.\n");
      return 1;
    }
    byteswritten = fwrite (buffer, 1, bytesread, dstfile);
    if (byteswritten != bytesread)
    {
      fclose (srcfile);
      fclose (dstfile);
      fprintf (stderr, "Error: Could not write %u bytes to destination file.\n",
               bytesread);
      return 1;
    }
    srcsize -= bytesread;
  }
  fclose (srcfile);
  fclose (dstfile);
  return 0;
}
