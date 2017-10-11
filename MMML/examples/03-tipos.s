start      :                                      # main function
             push 1                               # type(int)
l2         :                                      # store at 0 type(int)
             push 'c'                             # type(char)
l3         :                                      # store at 1 type(char)
             push 1.4                             # type(float)
l4         :                                      # store at 2 type(float)
             push "abc"
             acreate                              # type(str)
l5         :                                      # store at 3 type(char[])
             load 0                               # type(int)
             cast_d                               # type(float)
             load 2                               # type(float)
             add                                  # type(float)
l6         :                                      # store at 4 type(float)
             load 0                               # type(int)
             load 1                               # type(char)
             cast_i                               # type(int)
             add                                  # type(int)
l7         :                                      # store at 5 type(int)
             load 1                               # type(char)
             acreate 1                            # type(char[])
l8         :                                      # store at 6 type(char[])
             load 6                               # type(char[])
             acreate 1                            # type(char[][])
l9         :                                      # store at 7 type(char[][])
             load 6                               # type(char[])
             load 3                               # type(char[])
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
l10        :                                      # store at 8 type(char[])
             push 1                               # type(bool)
             bz l14
l12        :                                      # and: next test
             push 0                               # type(bool)
             bz l14
             jump l15                             # AND JUMP TRUE
l15        : push 1
             jump l13                             # AND JUMP OUT
l14        : push 0
l13        : 
l11        :                                      # store at 9 type(bool)
             load 1                               # type(char)
             bnz l19
l17        :                                      # or: test next
             load 2                               # type(float)
             bnz l19
             jump l20                             # OR JUMP FALSE
l20        : push 0
             jump l18                             # OR JUMP OUT
l19        : push 1
l18        : 
l16        :                                      # store at 10 type(bool)
             load 3                               # type(char[])
             alen                                 # type(bool)
             bnz l24
l22        :                                      # or: test next
             load 6                               # type(char[])
             alen                                 # type(bool)
             bnz l24
             jump l25                             # OR JUMP FALSE
l25        : push 0
             jump l23                             # OR JUMP OUT
l24        : push 1
l23        : 
l21        :                                      # store at 11 type(bool)
             load 0                               # type(int)
             cast_d                               # type(float)
l26        :                                      # store at 12 type(float)
             load 2                               # type(float)
             cast_c                               # type(char)
l27        :                                      # store at 13 type(char)
             load 10                              # type(bool)
             print
             crunch 0 14                          # end of context
l0         : print
             exit
