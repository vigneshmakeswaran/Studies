DIR=/bench/evocache_bench/sha
#$DIR/sha $DIR/vectors/verification/in_file9.asc > /root/sha_output_1.txt

#./force_app_core 2 time -v $DIR/sha $DIR/vectors/verification/in_file16.asc > /root/sha_output_1.txt  
./force_app_core 2 time -v ./repeat_sha.sh
