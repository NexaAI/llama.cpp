# Llama cpp
The original Llama cpp implementation is available at [here](https://github.com/ggerganov/llama.cpp).  

To submit changes:
```
git add .
git commit -m "commit message"
git push origin master
```

To build this project:
```
make clean
cmake -B build
cmake --build build --config Release -j 24
```