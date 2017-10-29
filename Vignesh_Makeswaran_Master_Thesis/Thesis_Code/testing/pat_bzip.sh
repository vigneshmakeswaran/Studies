./force_app_core2 1 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions  /bench/evocache_bench/patricia/patricia /bench/evocache_bench/patricia/vectors/verification/large.udp > /root/patricia_output_1.txt &
./force_app_core2 1 2 perf stat -r 1 -e context-switches,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions /bench/evocache_bench/bzip/bzip2 -k -f /bench/evocache_bench/bzip/vectors/verification/elf/1 > /root/bzip_output_1.txt &

