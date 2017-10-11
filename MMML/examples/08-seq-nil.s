l0         :                                      # function filtra_positivos_menores_que
             load 2                               # type(int[])
             alen                                 # type(bool)
             bz l19                               # branch to true
             jump l18                             # branch to false
l19        :                                      # true branch
             load 3                               # type(int[])
             jump l1                              # OUT OF TRUE BRANCH
l18        :                                      # false branch
             load 2                               # type(int[])
             aload                                # unpack sequence
             push -1
             add
             acreate                              # repack n-1
# store at 4 type(int)

l22        :                                      # store at 5 type(int[])
             load 4                               # type(int)
             push 0                               # type(int)
             sub                                  # cmp
             bneg l25
             jump l27
l27        :                                      # or: test next
             load 4                               # type(int)
             load 1                               # type(int)
             sub                                  # cmp
             bneg l24
             jump l25
l25        :                                      # true branch
             push l30                             # @ret_point for filtra_positivos_menores_que
             load 1                               # type(int)
             load 5                               # type(int[])
             load 3                               # type(int[])
             mark 4                               # keep params for filtra_positivos_menores_que
             jump l0                              # call filtra_positivos_menores_que
l30        : nop
             jump l23                             # OUT OF TRUE BRANCH
l24        :                                      # false branch
             load 3                               # type(int[])
             load 4                               # type(int)
             acreate 1                            # type(int[])
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
             drop_mark                            # type(int[])
l32        :                                      # store at 6 type(int[])
             push l33                             # @ret_point for filtra_positivos_menores_que
             load 1                               # type(int)
             load 5                               # type(int[])
             load 6                               # type(int[])
             mark 4                               # keep params for filtra_positivos_menores_que
             jump l0                              # call filtra_positivos_menores_que
l33        : nop
             crunch 6 1                           # end of context
             jump l23                             # OUT OF FALSE BRANCH
l23        :                                      # out point of if
             crunch 4 2                           # end of context
             jump l1                              # OUT OF FALSE BRANCH
l1         : crunch 1 3                           # return
             swap
             drop_mark
             jump                                 # jump to return point
start      :                                      # main function
             push l35                             # @ret_point for filtra_positivos_menores_que
             push 10                              # type(int)
             push 1                               # type(int)
             acreate 1                            # type(int[])
             push -2                              # type(int)
             acreate 1                            # type(int[])
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
             drop_mark                            # type(int[])
             push 100                             # type(int)
             acreate 1                            # type(int[])
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
             drop_mark                            # type(int[])
             push 9                               # type(int)
             acreate 1                            # type(int[])
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
             drop_mark                            # type(int[])
             push 10                              # type(int)
             acreate 1                            # type(int[])
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
             drop_mark                            # type(int[])
             acreate 0                            # type(nil)
             mark 4                               # keep params for filtra_positivos_menores_que
             jump l0                              # call filtra_positivos_menores_que
l35        : nop
l34        : print
             exit
