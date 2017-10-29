#!/bin/sh
DIR=/bench/evocache_bench/bzip
#$DIR/bzip2 -k -f $DIR/vectors/verification/elf/1

./force_app_core 2 perf stat -r 1 -e L1-dcache-store-misses,L1-dcache-stores,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/bzip2 -k -f $DIR/vectors/verification/elf/1 > /root/bzip_output_1.txt  

wait

./force_app_core 2 perf stat -r 1 -e L1-dcache-store-misses,L1-dcache-stores,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/bzip2 -k -f $DIR/vectors/verification/elf/1 > /root/bzip_output_1.txt

wait

./force_app_core 2 perf stat -r 1 -e L1-dcache-store-misses,L1-dcache-stores,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/bzip2 -k -f $DIR/vectors/verification/elf/1 > /root/bzip_output_1.txt
                                                                                                                                                                      
wait
                                                                                                                                                                      
./force_app_core 2 perf stat -r 1 -e L1-dcache-store-misses,L1-dcache-stores,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/bzip2 -k -f $DIR/vectors/verification/elf/1 > /root/bzip_output_1.txt

wait

./force_app_core 2 perf stat -r 1 -e L1-dcache-store-misses,L1-dcache-stores,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/bzip2 -k -f $DIR/vectors/verification/elf/1 > /root/bzip_output_1.txt

wait
