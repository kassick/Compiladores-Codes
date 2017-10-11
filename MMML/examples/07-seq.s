l0         :                                      # function str_capitalize_
             load 1                               # type(char[])
             alen                                 # type(bool)
             bz l17                               # branch to true
             jump l16                             # branch to false
l17        :                                      # true branch
             load 2                               # type(char[])
             jump l1                              # OUT OF TRUE BRANCH
l16        :                                      # false branch
             load 1                               # type(char[])
             aload                                # unpack sequence
             push -1
             add
             acreate                              # repack n-1
# store at 3 type(char)

l20        :                                      # store at 4 type(char[])
             load 3                               # type(char)
             push 'a'                             # type(char)
             sub                                  # cmp
             bneg l23
             jump l26
l26        :                                      # and: next test
             load 3                               # type(char)
             push 'z'                             # type(char)
             sub                                  # cmp
             bpos l23
             jump l24
l24        :                                      # true branch
             load 3                               # type(char)
             push 'a'                             # type(char)
             push 'A'                             # type(char)
             sub                                  # type(char)
             sub                                  # type(char)
             jump l22                             # OUT OF TRUE BRANCH
l23        :                                      # false branch
             load 3                               # type(char)
             jump l22                             # OUT OF FALSE BRANCH
l22        :                                      # store at 5 type(char)
             push l29                             # @ret_point for str_capitalize_
             load 4                               # type(char[])
             load 2                               # type(char[])
             load 5                               # type(char)
             acreate 1                            # type(char[])
             mark 2                               # stack: {2: c, d} {2: a, b} |  # right left
             push 0                               # stack: 0 {2: c, d} {2: a, b} |  #int right left
             swap 0                               # stack: {2: a, b} {2: c, d} 0 | left right @len
             aload                                # stack: 2 b a {2: c, d} 0 | llen l9 l8 ... r0 right len 
             store 0                              # stack: b a {2: c, d} 2 | l9 l8 ... l0 right len
             load 1                               # stack: {2: c, d} b a {2: c, d} 2 | right l9 ... l0 right len
             aload                                # stack: 2 d c b a {2: c, d} 2 | rlen r9 .. r0 l9 .. l0 right len
             load 0                               # stack: 2 2 d c b a {2: c, d} 2  |
             add                                  # stack: 4 d c b a {2: c, d} 2  |
             acreate                              # stack : {4 a b c d} {2: c d} 2 |
             crunch 0 2                           # stack : {4 a b c d} |
             drop_mark                            # type(char[])
             mark 3                               # keep params for str_capitalize_
             jump l0                              # call str_capitalize_
l29        : nop
             crunch 5 1                           # end of context
             crunch 3 2                           # end of context
             jump l1                              # OUT OF FALSE BRANCH
l1         : crunch 1 2                           # return
             swap
             drop_mark
             jump                                 # jump to return point
l30        :                                      # function str_capitalize
             push l33                             # @ret_point for str_capitalize_
             load 1                               # type(char[])
             push ""
             acreate                              # type(str)
             mark 3                               # keep params for str_capitalize_
             jump l0                              # call str_capitalize_
l33        : nop
l31        : crunch 1 1                           # return
             swap
             drop_mark
             jump                                 # jump to return point
start      :                                      # main function
             reads
             acreate
l36        :                                      # store at 0 type(char[])
             push l37                             # @ret_point for str_capitalize
             load 0                               # type(char[])
             mark 2                               # keep params for str_capitalize
             jump l30                             # call str_capitalize
l37        : nop
             aload
             prints
             crunch 0 1                           # end of context
l34        : print
             exit
