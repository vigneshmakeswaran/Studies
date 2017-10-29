#!/bin/sh
DIR=/bench/evocache_bench/sha
#$DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt

./force_app_core 2 perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt  

wait

./force_app_core 2 perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt

wait

./force_app_core 2 perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt

wait

./force_app_core 2 perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt                      

wait
                                                                                                                                                                       
./force_app_core 2 perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt
                                                                                                                                                                      
wait


