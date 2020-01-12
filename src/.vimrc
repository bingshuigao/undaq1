filetype indent plugin on
map <CR> i<CR><Esc>
map <Tab> i<Tab><Esc>l
map <Space> i<Space><Esc>l
set modeline
source $VIMRUNTIME/ftplugin/man.vim
set timeout timeoutlen=800
nmap KK :Man <cword><CR><c-w>j
"set softtabstop=4
set fileencodings=utf-8,gbk
nnoremap <silent> <F8> :TlistToggle<CR>
imap <c-e> <Esc>A

"set path
set path+=/usr/local/root/include/root
set path+=/usr/local/include
set path+=/usr/include/gtkmm-3.0
"set auto line break
"set tw=76

"set virtualedit
set virtualedit=block

"set backspace
set backspace=indent,start

"set mouse
set mouse=a

"Read .exrc file in current directory
set exrc
set secure

"For ctags, then it can find the 'tags' file even not in current directory
set tags=./tags;/
"Get out of VI's compatible mode..
set nocompatible
"Sets how many lines of history VIM har to remember
set history=400
"Set to auto read when a file is changed from the outside
set autoread
"Have the mouse enabled all the time:
"when you need to copy from vim, maybe you have to ':set mouse=' first
"set mouse=a
"""""""""""""""""""""""""""""""""""""
" Colors and Fonts
"""""""""""""""""""""""""""""""""""""
"Enable syntax highlight
syntax on
"set colorscheme
colorscheme anotherdark
set cursorline
hi CursorLine term=bold cterm=bold guibg=Grey40
"endif
"""""""""""""""""""""""""""""""""""""
" VIM userinterface
"""""""""""""""""""""""""""""""""""""
"Set 7 lines to the curors away from the border- when moving vertical..
set so=7
"Turn on WiLd menu
set wildmenu
"Always show current position
set ruler
"The commandbar is 2 high
"set cmdheight=2
"Show line number
set number
"set mouse=nv
"Set backspace
"set backspace=eol,start,indent
"Bbackspace and cursor keys wrap to
"set whichwrap+=<,>,h,l
"show matching bracets
"set showmatch
"How many tenths of a second to blink
"set mat=2
"Highlight search things
set hlsearch
"imediately show the search result
set incsearch
"""""""""""""""""""""""""""""""""""""
" Folding
"""""""""""""""""""""""""""""""""""""
"Enable folding, I find it very useful
set nofen
set fdl=0
set foldmethod=manual
"""""""""""""""""""""""""""""""""""""
" Text options
"""""""""""""""""""""""""""""""""""""
"set expandtab
"set shiftwidth=2
set ambiwidth=double
"set smarttab
"Set Tab=4 spaces
"set ts=4
"set lbr
"set tw=500
"set selection=inclusive
   """"""""""""""""""""""""""""""
   " Indent
   """"""""""""""""""""""""""""""
   "Auto indent
   set ai
   "Set auto indent width = 4 spaces
   "set sw=8
   "C-style indenting
   set cindent "usage: select codes, press '=' key, the codes will autoindenting
   "Wrap lines
   "set wrap
"Encoding settings
"""""""""""""""""""""""""""""""""""""
"plugins
"""""""""""""""""""""""""""""""""""""
" Tlist
if &diff
let Tlist_Auto_Open=0 "don't auto pen when compare two files
else
let Tlist_Auto_Open=0 "don't auto pen Tlist when open a file
endif
"set taglist window in right, delete the following line if you don't like
"let Tlist_Use_Right_Window=1
let Tlist_Auto_Update=1 
let Tlist_File_Fold_Auto_Close=1
"auto close Tlist when exiting file.
let Tlist_Exit_OnlyWindow = 1 
let g:tex_flavor = "latex"
let g:localvimrc_sandbox=0
let g:localvimrc_count=1
let g:localvimrc_ask=0
set viewoptions=cursor,folds,slash,unix
let g:skipview_files = ['*\.vim'] 

"autocmd InsertEnter * :set number
"autocmd InsertLeave * :set relativenumber
