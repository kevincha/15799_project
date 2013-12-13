# Single client
./redis-benchmark -n $1 -r $1 -c 1 -t get
