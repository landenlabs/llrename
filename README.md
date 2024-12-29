llrename
### Rename files


### Builds
* OSX(M3)      | Provided Xcode project
* Windows/DOS  | Provided Visual Studio solution
 
### Visit home website
[https://landenlabs.com](https://landenlabs.com)

### Description

C++ v17 code to build either a windows/dos or Mac/Linux command line tool 
that can scan directories and rename files, include case change and numbering.

### Help Banner:
<pre>
llrename  Dennis Lang v2.3 (LandenLabs.com) Dec 25 2024


Des: Rename file names
Use: llrename [options] directories...   or  files

 Options (only first unique characters required, options can be repeated):
   -includeFile=&lt;filePattern>   ; Include files by regex match
   -excludeFile=&lt;filePattern>   ; Exclude files by regex match
   -IncludePath=&lt;pathPattern>   ; Include path by regex match
   -ExcludePath=&lt;pathPattern>   ; Exclude path by regex match
   -D                           ; Rename directory
   -c/C                         ; lowercase or Uppercase
   -sub=&lt;regexp>                ; substitude regexpression
                                      /fromRexex/toRegex/
   -parts=&lt;fileParts>           ; See fileParts note below
   -startNum=1000               ; Start number, def=1
   -no                          ; No rename, dry run
   -force                       ; Deleted target if same name

   -toList=&lt;write_fileName>     ; Output List of 'old','new'
   -fromList=&lt;read_fileName>    ; Read List rename pair per line
 Used with -fromList
   -1       [default]           ; Rename 'old' to 'new'
   -2                           ; Rename 'new' to 'old'
   -full                        ; Log full path, default is relative
   -smartQuote                  ; Only add quotes if spaces in path
   -logStart="foo.csh "       ; Log prefix string, def none
   -logSep=", "               ; Log separator string, def=", "
   -logEnd="\n"              ; Log end of line, def="\n"

 FileParts:
     N=name, E=extension, #=number (note uppercase N and E)
     N-#.E
     N_####.E
     N.'foo'

 Debug:
   -showfiles                   ; Display files found
   -verbose                     ; Only dump parsed json

 Example:
  llrename -c -sub="/ /_/" -inc=*.png -exc=foo.png dir1 dir2
  llrename -C -start=1000 -part="N_####.E" -inc=*.jpg dir1 Foo*.png Bar*.jpg
  llrename -sub="/([^_]+)_([.*])/$2-$1/" dir1
 Warning with regular expression:
   Remember to escape special characters, such as ., ( and [
   For example to remove all  "copy (2) of" use
   -sub="/copy [(][0-9][)] of//"

</pre>

[Top](#top)
