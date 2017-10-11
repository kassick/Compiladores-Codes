start      :                                      # main function
             readi
l2         :                                      # store at 0 type(int)
             load 0                               # type(int)
             push 0                               # type(int)
             sub                                  # cmp
             bz l6
             jump l5
l6         :                                      # true branch
             push 10                              # type(int)
             jump l4                              # OUT OF TRUE BRANCH
l5         :                                      # false branch
             push 11                              # type(int)
             jump l4                              # OUT OF FALSE BRANCH
l4         :                                      # store at 1 type(int)
             load 1                               # type(int)
             push 10                              # type(int)
             add                                  # type(int)
             print
             crunch 1 1                           # end of context
             crunch 0 1                           # end of context
l0         : print
             exit
