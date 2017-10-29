DIR=/bench/evocache_bench/bzip
#$DIR/bzip2 -k -f $DIR/vectors/verification/elf/1

./force_app_core 2 time -v $DIR/bzip2 -k -f $DIR/vectors/verification/elf/1 > /root/bzip_output_1.txt  
