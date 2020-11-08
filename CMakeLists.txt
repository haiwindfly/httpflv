# CMakeList.txt: myserver 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
add_executable (myserver   "IO_SERVICE.h" "IO_SERVICE.cpp" "threadpool.h" "threadpool.cpp" "sql_pool.h" "sql_pool.cpp" "connnect.h" "http_conn.h" "http_conn.cpp" "connect.cpp"   "my_log.h" "my_log.cpp" "mytimer.h" "mytimer.cpp" "main.cpp" "oal.h")

# TODO: 如有需要，请添加测试并安装目标。
