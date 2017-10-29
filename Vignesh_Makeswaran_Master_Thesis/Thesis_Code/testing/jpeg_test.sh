DIR=/bench/evocache_bench/jpeg
#$DIR/cjpeg -opt -outfile $DIR/output_1_encode.jpeg $DIR/vectors/verification/1.ppm





perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/cjpeg -opt -outfile $DIR/output_1_encode.jpeg $DIR/vectors/verification/1.ppm > /root/jpeg_output_1.txt

wait

perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/cjpeg -opt -outfile $DIR/output_1_encode.jpeg $DIR/vectors/verification/1.ppm > /root/jpeg_output_1.t

wait 

perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/cjpeg -opt -outfile $DIR/output_1_encode.jpeg $DIR/vectors/verification/1.ppm > /root/jpeg_output_1.t

wait

 
perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/cjpeg -opt -outfile $DIR/output_1_encode.jpeg $DIR/vectors/verification/1.ppm > /root/jpeg_output_1.t




wait
                                                                                                                                                                       
perf stat -r 1 -e L1-dcache-stores,L1-dcache-store-misses,L1-dcache-load-misses,L1-dcache-loads,L1-icache-load-misses,L1-icache-loads,cycles,instructions $DIR/cjpeg -opt -outfile $DIR/output_1_encode.jpeg $DIR/vectors/verification/1.ppm > /root/jpeg_output_1.txt

                                                                                                                                                                      
wait


