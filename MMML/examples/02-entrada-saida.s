start      :                                      # main function
             push "Digite seu nome: "
             acreate                              # type(str)
             aload
             prints
l2         : pop                                  # drop _
             reads
             acreate
l3         :                                      # store at 0 type(char[])
             push "Digite sua idade: "
             acreate                              # type(str)
             aload
             prints
l4         : pop                                  # drop _
             readi
l5         :                                      # store at 1 type(int)
             push "Olá, "
             acreate                              # type(str)
             load 0                               # type(char[])
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
             push ", voce tem "
             acreate                              # type(str)
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
             push " anos"
             acreate                              # type(str)
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
             crunch 0 2                           # end of context
l0         : print
             exit
