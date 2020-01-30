for fl in (ls *.cc)
    g++ $fl -g -o /tmp/$fl.ex -std=c++17 -lr
end
