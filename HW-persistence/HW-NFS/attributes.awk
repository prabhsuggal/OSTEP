{
    attr[$8]++;
    tot++;
} 
END {
    for (var in attr){
        print var,"used",attr[var],"times"
    }
    print tot,"total_op"
}
