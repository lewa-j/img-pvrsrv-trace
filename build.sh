#! /bin/sh -v
g++ vk_test.cpp -lvulkan -o vk_test -Wall -g
gcc -std=gnu2x pvrsrv_trace.c -o pvrsrv_trace -Wall -I/usr/include/libdrm -g
g++ -I/usr/include/libdrm pvrsrv_test.cpp pvr_srv_bridge_client.c -ldrm -o pvrsrv_test -Wall -g
