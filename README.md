#llrename
OSX / Linux / DOS Rename files


Visit home website

[https://landenlabs.com](https://landenlabs.com)


Help Banner:
<pre>
llrename  Dennis Lang v2.1 (LandenLabs.com) Nov 28 2024


Des: Renumber file names
Use: llrename [options] directories...   or  files

 Options (only first unique characters required, options can be repeated):
   -includeFile=<filePattern>   ; Include files by regex match
   -excludeFile=<filePattern>   ; Exclude files by regex match
   -IncludePath=<pathPattern>   ; Include path by regex match
   -ExcludePath=<pathPattern>   ; Exclude path by regex match
   -D                           ; Rename directory
   -c/C                         ; lowercase or Uppercase
   -sub=<regexp>                ; substitude regexpression
                                      /fromRexex/toRegex/
   -parts=<fileParts>           ; See fileParts note below
   -startNum=1000               ; Start number, def=1
   -no                          ; No rename, dry run

   -toList=<write_fileName>     ; Output List of 'old','new'
   -fromList=<read_fileName>    ; Read List rename pair per line
 Used with -fromList
   -1       [default]           ; Rename 'old' to 'new'
   -2                           ; Rename 'new' to 'old'
   -full                        ; Log full path, default is relative
   -smartQuote                  ; Only add quotes if spaces in path
   -logStart="foo.csh "       ; Log prefix string, def none
   -logSep=", "               ; Log separator string, def=", "
   -logEnd="\n"              ; Log end of line, def="\n"

 FileParts:
     N=name, E=extension, #=number
     N-#.E
     N_####.E
     N.'foo'

 Debug:
   -showfiles                   ; Display files found
   -verbose                     ; Only dump parsed json

 Example:
  llrename -c -sub="/ /_/" -inc=\*.png -exc=foo.png dir1 dir2
  llrename -C -start=1000 -part="N_####.E" -inc=\*.jpg -inc=\*.png dir1 Foo*.png Bar*.jpg
  llrename -sub="/([^_]+)_([.*])/$2-$1/" dir1

</pre>
