NTFSFastFind
### Windows NTFS Fast File Find


### Builds
* Windows/DOS  | Provided Visual Studio solution

### Visit home website
[https://landenlabs.com/console/ntfsfastfind/ntfsfastfind.html](https://landenlabs.com/console/ntfsfastfind/ntfsfastfind.html)

### Description

Windows only - Fast File Find using NTFS internal database

Search for file names using simple patterns to scan the internal NTFS file database. 
Much faster than using Explorer's find feature. 

NTFSfastFind searches NTFS Master File Table (MFT) rather then iterating across directories.
NTFSfastFind does not use or maintain an index database
By reading the MFT directly, NTFSfastFind can locate files anywhere on a disk quickly.

Note: Standard directory searching is faster if you know the directory to search.
If you don't know the directory and need to search the entire disk drive, NTFSfastFind is fast.
If you use the -z switch, it will iterate across the directories rather then using MFT.



### Help Banner:
<pre>
NTFS Fast File Find v3.01 - Dec 28 2024
By: Dennis Lang
https://landenlabs.com

   NTFSfastFind [options] &lt;localNTFSdrivetoSearch>...
 Filter:
   -d &lt;count>                        ; Filter by data stream count
   -f &lt;fileFilter>                   ; Filter by filename, use * or ? patterns
   -s &lt;size>                         ; Filter by file size
   -t &lt;relativeModifyDate>           ; Filter by time modified, value is relative days
   -z                                ; Force slow style directory search
 Report:
   -A[=s|h|r|d|f|c]                  ; Include attributes, filter on attributes 
        s=system, h=hidden, r=readonly, d=directory, f=file, c=compressed
   -D                                ; Include directory
   -I                                ; Include mft index
   -S                                ; Include size
   -T                                ; Include time
   -V                                ; Include VCN array
   -X                                ; Only deleted entries 
   -#                                ; Include stream and name counts

 Query Drive status only, no file search
   -Q                                ; Query / Display MFT information only (see -v) 

 Examples:
    c: d:                  ; List entire c and d drive, display filenames. 
    -ITSA  d:              ; List entire d drive, display mft index, time, size, attributes, directory. 

  Filter examples (precede 'f' command letter with ! to invert rule):
    -f *.txt d:            ; Files ending in .txt on d: drive 
    -f \*\foo*\*.txt d: ; Files ending in .txt on d: drive in directory starting with foo 
 -f Map1.* -f Map2.*  c:   ; Files matching two patterns on c drive 
 -T -S -f *cache -t -0.1  c: ; Files ending in cache, modified less than 0.1 days ago 
    -!f *.txt d:           ; Files NOT ending in .txt on d: drive 
    -t 2.5 -f *.log        ; Modified more than 2.5 days and ending in .log on c drive 
    -t -0.2 e:             ; Modified less than 0.2 days ago on e drive 
    -s 1000 d:             ; File size greater than 1000 bytes on d drive 
    -s -1000 d: e:         ; File size less than 1000 bytes on d and e drive 
    -f F* c: d:            ; Limit scan to files starting with F on either C or D 
    -d 1 d:                ; Files with more than 1 data stream on d: drive 

    -X -f * c:             ; All deleted entries on c: drive 
-X -T -S -f *cache  c:     ; Delete files ending in cache, show modify time and size 
-X  -f *cache -t -1 c:     ; Deleted files modifies less than 1 day ago 

    -Q c:              ; Display special NTFS files
    -z c:\windows\system32\*.dll   ; Force slow directory search.
</pre>

## Web resources

* https://www.writeblocked.org/resources/NTFS_CHEAT_SHEETS.pdf
* https://docs.velociraptor.app/docs/forensic/ntfs/
* https://medium.com/search?q=A+Journey+into+NTFS

[Top](#top)
