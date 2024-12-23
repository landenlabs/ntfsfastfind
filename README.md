#NTFSFastFind

Windows only - Fast File Find using NTFS internal database

Search for file names using simple patterns to scan the internal NTFS file database. 
Much faster than using Explorer's find feature. 

Visit home website

[https://landenlabs.com](https://landenlabs.com)


Help Banner:
<pre>
NTFS Fast File Find v3.00 - Dec 15 2024
By: Dennis Lang
https://home.comcast.net/~lang.dennis/

Description:
   NTFSfastFind searches NTFS Master File Table (MFT) rather then iterating across directories.
   NTFSfastFind does not use or maintain an index database
   By reading the MFT directly, NTFSfastFind can locate files anywhere on a disk quickly.
   Note: Standard directory searching is faster if you know the directory to search.
   If you don't know the directory and need to search the entire disk drive, NTFSfastFind is fast.

   If you use the -z switch, it will iterate across the directories rather then using MFT.

Use:
   NTFSfastFind [options] <localNTFSdrivetoSearch>...
 Filter:
   -d <count>                        ; Filter by data stream count
   -f <fileFilter>                   ; Filter by filename, use * or ? patterns
   -s <size>                         ; Filter by file size
   -t <relativeModifyDate>           ; Filter by time modified, value is relative days
   -z                                ; Force slow style directory search
 Report:
   -A[=s|h|r|d|c]                    ; Include attributes, filter on attributes
   -D                                ; Include directory
   -I                                ; Include mft index
   -S                                ; Include size
   -T                                ; Include time
   -V                                ; Include VCN array
   -#                                ; Include stream and name counts

   -Q                                ; Query / Display MFT information only

 Examples:
  No filtering:
    c:                 ; scan c drive, display filenames.
    -ITSA  c:          ; scan c drive, display mft index, time, size, attributes, directory.
  Filter examples (precede 'f' command letter with ! to invert rule):
    -f *.txt d:        ; files ending in .txt on d: drive
    -f \*\foo*\*.txt d:; files ending in .txt on d: drive in directory starting with foo
    -!f *.txt d:       ; files NOT ending in .txt on d: drive
    -t 2.5 -f *.log    ; modified more than 2.5 days and ending in .log on c drive
    -t -7 e:           ; modified less than 7 days ago on e drive
    -s 1000 d:         ; file size greater than 1000 bytes on d drive
    -s -1000 d: e:     ; file size less than 1000 bytes on d and e drive
    -f F* c: d:        ; limit scan to files starting with F on either C or D
    -d 1 d:            ; files with more than 1 data stream on d: drive
    -Q c:              ; Display special NTFS files
    -z c:\windows\system32\*.dll   ; Force slow directory search.
</pre>
