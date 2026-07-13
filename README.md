<table border="0">
  <tr>
    <td>
      <!-- VERSION -->v6.07.10<br>
      <!-- DATE -->12-Jul-2026<br>
      Win & MacOS<br>
      <a href="https://landenlabs.com">Home</a>
    </td>
    <td>
      <a href="https://landenlabs.com">
        <img src="screens/landen_labs_300.webp" width="300" alt="LanDen Labs">
      </a>
    </td>
  </tr>
</table>

# llrename
### Rename files

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE.txt)
![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows-lightgrey.svg)

### Builds
* OSX(M3)      | Provided Xcode project
* Windows/DOS  | Provided Visual Studio solution

### Dependencies
* [llcommon](https://github.com/landenlabs/llcommon) - shared LanDen Labs utility library (git submodule)
 
### Visit home website
[https://landenlabs.com](https://landenlabs.com)

### Description

C++ v17 code to build either a windows/dos or Mac/Linux command line tool 
that can scan directories and rename files, include case change and numbering.

### Help Banner:
<pre>
llrename  Dennis Lang <!-- VERSION -->v6.07.10<!-- DATE -->12-Jul-2026


Des: Rename file names
Use: llrename [options] directories...   or  files

 Options (only first unique characters required, options can be repeated):
   -includeItem=&lt;filePattern>   ; Include files or dirs by regex match
   -excludeItem=&lt;filePattern>   ; Exclude files or dirs by regex match
   -IncludePath=&lt;pathPattern>   ; Include path by regex match
   -ExcludePath=&lt;pathPattern>   ; Exclude path by regex match
   -D                           ; Rename directory
   -c/C                         ; lowercase or Uppercase
   -sub=&lt;regexp>                ; substitute regexpression
                                      /fromRexex/toRegex/
   -parts=&lt;fileParts>           ; See fileParts note below
   -startNum=1000               ; Start number, def=1
   -no                          ; No rename, dry run
   -force                       ; Deleted target if same name
   -recurse                     ; Recurse into directories

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
  llrename -c -r -sub="/ /_/" -inc=*.png -exc=foo.png dir1 dir2
  llrename -C -r -start=1000 -part="N_####.E" -inc=*.jpg dir1 Foo*.png Bar*.jpg
  llrename -sub="/([^_]+)_([.*])/$2-$1/" dir1
 Warning with regular expression:
   Remember to escape special characters, such as ., ( and [
   For example to remove all  "copy (2) of" use
   -sub="/copy [(][0-9][)] of//"

   The include/exclude regular expression internally converts
      * to .*   and  ?  to .
     Ex:  *.png  is internally .*.png

</pre>

### License

```
Copyright 2026 Dennis Lang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
See [LICENSE.txt](LICENSE.txt) for the full license text.

[Top](#top)
