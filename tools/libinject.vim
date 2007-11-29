if exists("main_syntax")
  let main_syntax = 'libinject'
endif

syn keyword ruleKeyword on from to when with port contained
syn match   ruleKeyword "talk-with" contained
syn keyword ruleTransport pipe ip tcp udp port any dns me command connect contained
syn keyword ruleNext continue goto next stop exec contained
syn match   ruleDo "do\(-once\(-per-\(call\|socket\)\)\?\)\?" contained
syn keyword ruleCond matched unmatched before after between never always cycle prob contained
syn keyword ruleAction skip echo syscall hang remove dump log truncate contained
syn match ruleAction "\(cancel-syscall\|local-hang\|remote-hang\|mark-done\)" contained
syn keyword ruleBool true false contained
syn match ruleLine "-\?[0-9]\+" contained
syn region rule start=/^/ end=/$/ contains=ruleLine,ruleKeyword,ruleTransport,ruleDo,ruleNext,ruleAction,ruleCond,ruleBool
syn region ruleSection start=/^\[/ end=/]/
syn region ruleComment start=/^;/ end=/$/ contains=@Spell


command -nargs=+ HiLink hi def link <args>
HiLink ruleLine    Constant
HiLink ruleBool    Constant
HiLink ruleComment Comment
HiLink ruleKeyword Operator
HiLink ruleTransport Type
HiLink ruleNext    Type
HiLink ruleDo      Type
HiLink ruleAction  Function
HiLink ruleCond    Operator
HiLink ruleSection Special
delcommand HiLink

" vim:et ts=2 sw=2
