" Copyright 2020 Iris Chase
"
" Licensed under the Apache License, Version 2.0 (the "License");
"   you may not use this file except in compliance with the License.
"   You may obtain a copy of the License at
"
"       http://www.apache.org/licenses/LICENSE-2.0
"
"   Unless required by applicable law or agreed to in writing, software
"   distributed under the License is distributed on an "AS IS" BASIS,
"   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"   See the License for the specific language governing permissions and
"   limitations under the License.

spec bleeding;

" Custom syntax for the IVD language.
" IVD: Interactive Visual Design

if exists("b:current_syntax")
    finish
endif

echom "IVD: Interactive Visual Design"

syntax keyword ivdLabel state nextgroup=ivdCustomState skipwhite
syntax keyword ivdKeyword exclusive

syntax keyword ivdAttributes  position position-within margin layout
syntax keyword ivdAttributes width height greyout

syntax match ivdCustomState "\i\+" contained
highlight link ivdCustomState Identifier 

syntax match ivdComment "\v//.*$"
highlight link ivdComment Comment


highlight link ivdLabel Label
highlight link ivdKeyword Keyword
highlight link ivdAttributes Type



let b:current_syntax = "ivd"

