#!/bin/sh
CF="-std=c++11 -Wall -Wextra -pedantic -Ofast -march=native -flto -DNDEBUG"
# CF="-std=c++11 -Wall -Wextra -pedantic -Og -g"
g++ -Wl,--no-as-needed -pthread $CF src/*.cc #&& ./a.out