BEGIN{
    max_size=0;
    min_size=2^53;
    max_file_access=0;
}
{
    if($8=="getattr" && $5=="R3"){
        hex_prefix = "0x"
        hex_val = strtonum(hex_prefix $21);
        max_size = (hex_val>max_size)?hex_val:max_size;
        min_size = (hex_val<min_size)?hex_val:min_size;
        tot_size += hex_val;
        ops++;
        user[$17]++;
        file[$31]++;
    }
}
END{
    print "USERS","number of acceses";
    for(var in user){
        print var,user[var];
    }
    print "FILES ID", "number of acceses"
    for(var in file){
        print var,file[var];
        if(max_file_access < file[var]){
            max_access_file = var;
            max_file_access = file[var];
        }    
    }
    avg_size = tot_size/ops;
    print min_size,"MIN_SIZE",max_size,"MAX_SIZE",avg_size,"AVG_SIZE", max_access_file, "FILE_MAX_ACCESSED";
}