build: poc.c
	gcc poc.c -o poc32 -m32 -ggdb -Wno-implicit-function-declaration
	patchelf --add-needed ./libc.so.6 poc32
	patchelf --add-needed ./libscudo32.so poc32
	patchelf --set-interpreter ./ld.so.2 poc32
