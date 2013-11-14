rm -rf d1/* d2/*
build/namenode 50070 > /dev/null &
sleep 3 
build/datanode 50060 127.0.0.1 1 d1/
build/datanode 50061 127.0.0.1 2 d2/ 
./dfs 127.0.0.1 50070