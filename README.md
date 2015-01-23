# Tsudp
Workspace只是实现客户端连接服务器并向服务器传输1G文件

直接make编译
dd if=/dev/zero of=1G.img bs=1M count=1024
服务器端直接执行./server
客户端执行：./client 
