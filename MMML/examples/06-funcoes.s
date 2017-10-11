l0         :                                      # function ping_
             load 1                               # type(int)
             load 2                               # type(int)
             sub                                  # cmp
             bz l11
             jump l10
l11        :                                      # true branch
             load 2                               # type(int)
             jump l2                              # OUT OF TRUE BRANCH
l10        :                                      # false branch
             push "Ping "
             acreate                              # type(str)
             load 1                               # type(int)
             cast_s
             acreate
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
             push '\n'                            # type(char)
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
             aload
             prints
l15        : pop                                  # drop _
             push l16                             # @ret_point for pong_
             load 1                               # type(int)
             load 2                               # type(int)
             mark 3                               # keep params for pong_
             jump l1                              # call pong_
l16        : nop
             crunch 3 0                           # end of context
             jump l2                              # OUT OF FALSE BRANCH
l2         : crunch 1 2                           # return
             swap
             drop_mark
             jump                                 # jump to return point
l1         :                                      # function pong_
             load 1                               # type(int)
             load 2                               # type(int)
             sub                                  # cmp
             bz l26
             jump l25
l26        :                                      # true branch
             load 2                               # type(int)
             jump l17                             # OUT OF TRUE BRANCH
l25        :                                      # false branch
             push "Pong "
             acreate                              # type(str)
             load 1                               # type(int)
             cast_s
             acreate
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
             push '\n'                            # type(char)
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
             aload
             prints
l30        : pop                                  # drop _
             push l31                             # @ret_point for ping_
             load 1                               # type(int)
             push 1                               # type(int)
             add                                  # type(int)
             load 2                               # type(int)
             mark 3                               # keep params for ping_
             jump l0                              # call ping_
l31        : nop
             crunch 3 0                           # end of context
             jump l17                             # OUT OF FALSE BRANCH
l17        : crunch 1 2                           # return
             swap
             drop_mark
             jump                                 # jump to return point
start      :                                      # main function
             push l33                             # @ret_point for ping_
             push 0                               # type(int)
             push 10                              # type(int)
             mark 3                               # keep params for ping_
             jump l0                              # call ping_
l33        : nop
l32        : print
             exit
