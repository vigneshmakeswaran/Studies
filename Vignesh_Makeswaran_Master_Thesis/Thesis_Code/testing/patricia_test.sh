DIR=/bench/evocache_bench/patricia
#$DIR/patricia $DIR/vectors/verification/large.udp > $DIR/output_large.txt




./force_app_core 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions  $DIR/patricia $DIR/vectors/verification/large.udp > /root/patricia_output_1.txt  

wait

./force_app_core 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions  $DIR/patricia $DIR/vectors/verification/large.udp > /root/patricia_output_1.txt

wait

./force_app_core 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions  $DIR/patricia $DIR/vectors/verification/large.udp > /root/patricia_output_1.txt

wait

./force_app_core 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions  $DIR/patricia $DIR/vectors/verification/large.udp > /root/patricia_output_1.txt

wait
                                                                                                                                                                       
./force_app_core 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions  $DIR/patricia $DIR/vectors/verification/large.udp > /root/patricia_output_1.txt
                                                                                                                                                                      
wait


