
RAMFS - an IFS-based RAM Disk for OS/2 - Version 1.21.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Foreword by Karl Olsen
~~~~~~~~~~~~~~~~~~~~~~
Some time ago I wanted to learn about Installable File Systems (IFSes) in OS/2,
so I wrote RAMFS, an IFS-based RAM disk.

RAMFS is freeware. Feel free to use it, and to get ideas from the source code
when you develop your own file systems. I developed RAMFS on OS/2 Warp 3.0,
and have seen it work on OS/2 Warp 4.0.

RAMFS is a "remote file system" (as opposed to a "local file system") meaning
that it isn't associated with a hard disk partition. Instead, drive letters
are created through OS/2 calls. OS/2 doesn't know the internal data format in
the file system - it only accesses the data through special file system
functions.

Files are stored in RAM. This means that the data are lost when the system is
rebooted. It is useful for holding temporary files - just let the TMP and
TEMP variables point to a RAMFS drive, and your temp directory is
automatically cleaned up at each boot.


Features
~~~~~~~~
o Create as many RAM drives as you want, using the drive letters that you
  want.

o Size only limited by available RAM and swap disk space.

o Allocates swappable RAM from OS/2 as necessary when files are created, and
  releases it again when files are deleted.

o Long file name support like in HPFS. Case isn't significant, but preserved 
  like in HPFS. Files with long names are not visible from DOS and WINOS2 
  programs.

o Extended Attributes - up to 64 KB total for each file/directory.

o Intelligent RAM allocation algorithm - variable-length clusters provide
  greater performance yet less system resources.

o 3DNow!<TM> performance optimizations for AMD K7 (Athlon and Duron) family.


Misfeatures
~~~~~~~~~~~
o Does not support file locking functions (DosSetFileLocks()).

o Does not support the DosFindNotify...() functions. The notification
  mechanism is required for auto-refresh capability in WPS folders and
  may be used by some third-party file managers.

o Smaller files consume system resources heavily. For a typical OS/2
  workstation, having more than 10000 files on RAMFS may lead to a
  premature depletion of RAM handles.

o Multitasking is severely affected while preallocating large files on
  a RAMFS volume. See "Performance considerations".

o Directories are stored as plain lists, much like FAT. Large directories
  will have all manifestations of quadratic complexity - this means huge
  delays when adding/deleting files.

 
Installation and use
~~~~~~~~~~~~~~~~~~~~
Add "IFS=d:\path\RAMFS.IFS" to CONFIG.SYS and reboot. During boot, it will
show a short version message.

Then, from an OS/2 prompt, use RAMDISK.EXE to create a RAM drive. To create a
drive R:, type "RAMDISK R:" To have a RAM drive created at every boot, you
can add "CALL=d:\path\RAMDISK R:" to CONFIG.SYS.

In OS/2 v 4.50+, if you have a lot of physical memory and would like to
make all of it usable for RAMFS, the VIRTUALADDRESSLIMIT parameter in
CONFIG.SYS may have to be adjusted downward. On a system with 1536M RAM,
a value of VIRTUALADDRESSLIMIT=1024 is deemed enough for most practical
purposes.


Advanced configuration - RAMFS.IFS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The following parameters may be supplied to RAMFS.IFS entry in the
CONFIG.SYS:

o /Q = quiet initialization (suppresses the display of version notice)

o /M:<size> = limit the maximum amount of dynamic heap memory, kind of
  like setting the disk size. Default is 3GB (i.e. unlimited).
  The range of the size is 1MB to 3GB. Postfixes 'K' (kilobyte),
  'M' (megabyte) and 'G' (gigabyte) are allowed. Examples:
        /M:384M = Restrict RAMFS memory usage to 384 megabytes
          /M:1G = Restrict RAMFS memory usage to 1 gigabyte
  (It's recommended to set this value according to the common amount
  of free physical memory on your system. OS/2 has one or more nasty
  features which cause traps if you overcommit the memory too heavily.)

o /S:<size> = preset the free space reported by RAMFS. Default is 64M,
  however, if any applications complain about insufficient free space,
  this parameter may be helpful. The default unit is bytes. Append the
  value with 'K', 'M', 'G' or 'T' to scale accordingly. Examples:
      /S:100000 = Report 100000 bytes (rounded down to 98304)
        /S:355K = Report 355 kilobytes (rounded down to 352K)
         /S:15M = Report 15 megabytes
          /S:1G = Report 1 gigabyte
          /S:1T = Report 1 terabyte
  The granularity is 4K. Values above 2047M are not recommended (some
  applications misinterpret larger values).

o /CLU:[<size>x<count>:[<size>x<count>:]...]<size> = configure the
  variable-length clusters of RAMFS to balance performance vs. memory
  overhead. <size> is the cluster size in 4K units (1 = 4K, 2 = 8K, ...,
  8 = 32K). <count> is the number of consecutive clusters for which
  <size> is maintained. The last <size> applies to the rest of file.

  Smaller <size>`s help to conserve the RAM at the cost of performance
  penalties on large files and excessive load on the system handles.
  Larger <size>`s waste some memory but accelerate RAMFS on large
  contiguous accesses, and preserve the handles. And this is how the
  variable-length mechanism works: at the beginning of each file, there
  is a fine division into very small clusters, which are followed by
  larger blocks towards the end:

  0 4 8  16  24  32              64              96K  Example:
  +-+-+---+---+---+---------------+---------------+   /CLU:1x2:2x3:8
  |#|#|###|###|###|###############|####xxxxxxxxxxx|   File size   : 72K
  +-+-+---+---+---+---------------+----slack space+   Memory usage: 96K

  More examples:
          /CLU:1 = the most memory-conserving option, recommended for
                   systems with less than 96M of system RAM.
      /CLU:1x8:8 = best for small files. First 32K (x8) of any file
                   are allocated by 4K (1 unit), the rest is allocated
                   with increments of 32K (8 units).
          /CLU:8 = best for large files. Similar to FAT16.
  /CLU:1x2:4x4:8 = heavy-duty, also the default.

  Tip: there may be up to 16 <size>x<count> expressions but it is wise
  not to complicate the allocation strategy.

o /R = force the files on RAMFS to be non-swappable (always resident
  in RAM). The primary purpose of this option is to ensure the
  shortest access time for the files created on RAMFS. Keep in mind
  that the performance will begin to decrease more and more once
  SWAPPER.DAT is "hit" (free RAM=512K). A progressing VMM starvation
  may drive the system into a permanent swapping cycle unless some
  space on RAMFS is freed.

o /!VM = do not verify the memory addresses. Improves speed by about
  10% at the cost of security - certain misbehaving applications will
  be able to trap the system. Observed on Win32 self-installers under
  Odin and synthetic 16-bit testcase programs.

o /!3 = disable 3DNow!<TM>. Using the AMD 3DNow!<TM> instruction set
  requires some modifications to the kernel code in memory. Use this
  parameter if you experience any abnormal effects (none are known
  so far).

o /3 = force 3DNow!<TM>. By default, if RAMFS fails to patch the
  kernel, it disables the 3DNow!<TM> support regardless of your CPU
  capabilities. You may enforce 3DNow!<TM> if you are unsure about
  whether it has been enabled, and leave this option if nothing
  breaks.

  Currently the 3DNow! initialization is deferred till the first use
  of a RAMFS volume, hence the user is unable to see it during boot.

o /¤¤¤å = (introducing national language options) disable the DBCS kernel
  workaround. Generally you have to supply this option only if you can
  read its name properly, or if your system is configured for some
  other double-byte character set (e.g. codepage 933). You can use
  "/DBCS" as a synonym, for the same purpose.

  Background: the OS/2 kernels from 14.089 up to 14.095 come with
  APAR PJ28318, a solution to improve DBCS support on remote filesystems
  (that covers RAMFS as well as IBM LAN Redirector). However, this APAR
  enforces an additional constraint on filename checking logic, incurring
  regressions in SBCS (Latin and Cyrillic) environments. By default,
  RAMFS attempts to patch the kernel to roll back PJ28318. This parameter
  will prevent it from doing that. More recent kernels apparently do not
  require this sort of circumvention.


Advanced configuration - RAMDISK.EXE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The version of RAMDISK.EXE supplied with the current version of RAMFS
supports the following command-line parameters:

o [/CREATE] <drive>: [volume label] = synonym for plain syntax of the
  older RAMDISK.EXE (which had no purpose other than creating new
  volumes). Therefore, the following two commands are equivalent:

    RAMDISK T: mylabel
  and
    RAMDISK /CREATE T: mylabel

o /DELETE <drive>: = delete a volume. This functionality is to be
  considered unstable for a number of forthcoming revisions of RAMFS.
  It may cause system traps when disposing of volumes currently in use.

  /DELETE may take some time to complete if the volume is heavily
  laden with small files. It is normal if the system freezes for a few
  seconds while processing this command.

o /HEAP:<size> = restrict memory consumption. Use this parameter if
  there's a risk of exceeding the available memory when working with
  RAMFS. In a nutshell, /HEAP just controls the overall "weight" of
  every RAMFS structure, and denies further growth when the limit is
  reached. "Structures" include directories as well as files, hence
  the total size of user data that can be poured into the volume will
  be somewhat lower than <size>.

  If there's more than one RAMFS volume, this parameter restricts
  the total memory usage across all volumes. The sum of space used on
  all volumes must be less than <size>.

o /HEAP = query memory statistics. Under the "Maximum heap size", the
  value entered with /HEAP:<size> can be monitored. The default is 3G.
  Other fields represent diagnostic counters that can be watched by
  experienced users to track down memory leaks, if any.


Memory usage considerations
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The amount of free memory reported by RAMFS is a fixed value. (It isn't
easy to get the amount of free virtual memory from a ring-0 file system.)

When an application creates (or grows) a file on a RAMFS drive, RAMFS.IFS
allocates some swappable memory from OS/2. If this allocation fails, then
RAMFS.IFS returns a "disk full" error code to the application. This
mechanism has nothing to do with the value returned when an application asks
for the amount of free disk space (which can be controlled with the /S
parameter). So theoretically there is no risk involved in returning an
arbitrary value for the free disk space.

Ill-conceived applications might fail to check the return values from file
write operations if they have just found out that there is plenty of free
space. Applications should of course always check the return codes from all
system calls.

And what happens when OS/2 runs out of physical memory and disk swap space?
I am not sure that it checks the free space on the swap drive when virtual
memory is allocated. It would then have to enforce that there always is at
least as much free space on the swap drive as the amount of virtual memory
that could potentially need to be swapped out. And consequently it would
have to return "disk full" to applications that tried to create files on the
swap drive if that would cause the free space to drop below the enforced
minimum. I don't think that OS/2 does this.

So practically, I think that you should be careful about exhausting virtual
memory. When you experiment about what happens when you try to fill a RAMFS
drive, don't have important unsaved data in open applications :-)


Performance considerations
~~~~~~~~~~~~~~~~~~~~~~~~~~
The distinctive feature about RAMFS is the increased performance. However,
in its current implementation, there are certain bottlenecks.

o In the current release of RAMFS, subdirectories are stored as linear
  unsorted lists. This means RAMFS will need to read the directory from the
  beginning when adding, opening or deleting a file (much like FAT16).
  Having several thousand files in a single directory is considerably slower
  than having the same number scattered across subtrees.

o Inside RAMFS.IFS, there is no multitasking. And preallocating large amount
  of RAM usually turns out to be time-consuming. On a 1.5 GHz machine,
  creating a file 1024M long can freeze the system for some 5 seconds.
  Time-critical operations, such as data transfer to CD-R drives, will starve
  their CPU time (consequently, the CD recorder may encounter a fatal buffer
  underrun).

  "Preallocation" means the program already knows the resulting size, hence
  this degradation occurs mostly when copying files to RAMFS. As a rule of
  thumb, divide the MHz frequency of your CPU by 20 to obtain the maximum
  file size (in megabytes) that may be safely copied during the time-critical
  operations. E.g. for a 500 MHz CPU, this yields 25M.


Source code
~~~~~~~~~~~
RAMFS.IFS was originally written in Borland C++ 3.1 and Turbo Assembler 3.1.
Yes, Borland C++ happily compiles 16-bit code for OS/2. Starting with
version 1.02, it was migrated to IBM DDK environment (Microsoft C v 6.0).

RAMDISK.EXE comprises a collection of small utilities that may be compiled
with virtually any of the 11 C/C++ compilers available today for OS/2.

Shall the need to revisit an older version arise, one may easily downgrade the
code to the official 1.01 release by using the "rollback.gz" patch:

gzip -dc<rollback.gz|patch -p0

(gzip and GNU Patch are to be present on PATH).
 

History
~~~~~~~
The development path of this public-domain utility, due to independent
participation of various developers, is quite scattered. See the enclosed
RAMFS.GIF for better understanding of various branches.

1998-01-03: (without version number). Initial release by Karl Olsen.

1998-12-05: RAMFS64. The only modification is reporting 64M free space.
Released by Jack Stein.

2002-03-21: RAMFS64 Performance Update. Changed memory allocation algorithm
to avoid performance penalties associated with moving the same data back
and forth. This version was developed by Andrew Belov.

2002-04-14: Version 1.01. No big changes. Released by Karl Olsen and
Stewart Buckingham. Version 1.01 does NOT contain the aforementioned
performance patch. Here is the original change log:

o Added a version number :-), the boot message, and the /Q switch.

o Fixed a bug that showed up when doing wildcard searches from 32-bit
  programs, in particular Jonathan de Boyne Pollard's 32-bit Command
  Interpreter. Strange things happened if you specified an exact filename
  to a command that allowed wildcards. 

o Changed the amount of free space and total space reported to OS/2 from 1 MB
  to 64 MB. Some programs don't like to write large files to a drive that 
  apparently only has 1 MB free. A test version, ramfs64.zip, with just this
  change has been floating around for some time. 

o Added the RCOPY program. 

2002-10-21: Version 1.02 Performance Update. This is a merger of the
previous two releases, released by Andrew Belov. Summary of changes:

o Removed RCOPY (made obsolete by the performance patch).

o Moved to Microsoft C v 6.0 and the DDK build environment.

o Implemented AMD 3DNow!<TM> block copy code as per AMD technical note
  #22007. Added a kernel patch module and command-line parameters
  (/3, /!3) to control 3DNow!<TM>.

o Added /S command-line parameter to set the free space amount reported
  by RAMFS.

2002-11-03: Version 1.03.

o Fixed wrong behavior when memory allocation for large files failed
  (regression from 2002-03-21 perf. enh.)

2002-12-06: Version 1.04.

o Added the /R parameter to keep the RAMFS data resident.

2003-01-12: Version 1.05.

o Fixed a bug which resulted in read-only files being unrestricted for
  overwriting.

2003-02-19: Version 1.06.

o Fixed the "avalanche" trap effect that might occur under certain
  conditions with 3DNow!<TM> optimization (regression from 1.02).

2003-02-22: Version 1.07.

o The performance optimizations are restricted to K7 only.

o Fixed an issue with the wrong detection of 3DNow!<TM> capabilities
  (regression from 1.02).

2003-05-31: Version 1.08.

o Fixed the attributes returned for long filenames.

2003-06-16: Version 1.10.

o Added an SBCS patch for 14.09x kernels (fixes inability to create
  filenames which end with an odd number of high-ASCII characters).

o Interim solution for omissions in file list when retrieving multiple
  directory entries using DosFindFirst/FindNext.

2003-11-03: Version 1.11.

o Implemented options to restrict the overall memory usage in RAMFS.

2003-11-17: Version 1.12.

o Valid date/time properties are now reported for the root directory.

2003-12-02: Version 1.15.

o Fixed traps during startup and upon memory exhaustion when using
  MEMMAN=NOSWAP.

o Added the /CLU parameter to configure the variable cluster ranges.

o Added a check for invalid memory addresses along with the /!VM parameter
  that disables it in favor of speed.

2005-06-22: Version 1.16.

o Rearranged the read/write operations to ensure reentrancy when data
  transfer between RAMFS and a freshly allocated region requires program
  code to be loaded from RAMFS. Old behavior caused a deadlock condition
  (e.g. with IBM Web Browser installer).

o Added a check to prevent empty filenames from being created (as in
  "prompt>T:\").

2005-09-17: Version 1.17. Corrected the open/replace logic:

o Previous versions of RAMFS unconditionally prohibited replacing open
  files - this has been replaced with a check for current sharing
  restrictions.

o It is now impossible to replace a hidden/system file with a regular one.

2005-10-21: Version 1.18.

o Compatibility fix: error codes for deleting non-empty directories, i.e.
  DosDeleteDir("somepath/somefile") and DosDeleteDir("somepath"), now
  match those returned by NETWKSTA.200 in similar cases.

2006-03-16: Version 1.20.

o Fixed missing files in DosFindNext requests for redirected RAMFS volumes.

2006-04-02: Version 1.21.

o Rolled back the 1.20 fix due to compatibility/performance regressions.

o Introduced experimental support for FS_DETACH (RAMDISK /DELETE x:).


Distribution package
~~~~~~~~~~~~~~~~~~~~
  README.TXT  : This file
  RAMFS.GIF   : Graphical supplement :-)
  RAMFS.IFS   : The Installable File System driver
  RAMFS.SYM   : Symbolic information for debugging
  RAMDISK.EXE : Utility to manage RAM disk drives
  SRC\*.*     : Source code to RAMFS.IFS, RAMDISK.EXE, and RCOPY.EXE


Credits
~~~~~~~
RAMFS has been originally conceived, developed and released under a
public-domain license by Karl Olsen <kro@post3.tele.dk>, http://karl.myip.org.

Andrew Belov <andrew_belov@newmail.ru> worked on performance improvements
within RAMFS, as well as provided various minor bugfixes and enhancements.

Knut St. Osmundsen <bird@anduin.net> has contributed a solution to control
memory heap usage for version 1.11 of RAMFS.

The "osFree TPE development group" (having chosen to remain anonymous) has
supplied a precious tool to glue the 16/32-bit parts of RAMFS together.

Finally, the authors and maintainers would like to extend their thanks to
end-users of RAMFS for valuable suggestions and bug reports.


Contacting the maintainer
~~~~~~~~~~~~~~~~~~~~~~~~~
RAMFS is presently maintained by Andrew Belov <andrew_belov@newmail.ru>.
