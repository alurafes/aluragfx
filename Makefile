test:
	C:\VulkanSDK\1.3.275.0\Bin\glslc.exe .\shaders\shader.frag -o .\shaders\frag.spv
	C:\VulkanSDK\1.3.275.0\Bin\glslc.exe .\shaders\shader.vert -o .\shaders\vert.spv
	gcc -o main ./src/main.c ./src/engine.c ./src/present.c ./src/context.c ./src/swapchain.c ./src/renderer.c ./src/utils.c -g -lmingw32 -lSDL2main -lSDL2 -lvulkan-1 -I./include -IE:\cpplibs\sdl2-x86_64-w64-mingw32\include -IC:\VulkanSDK\1.3.275.0\Include -LC:\VulkanSDK\1.3.275.0\Lib -LE:\cpplibs\sdl2-x86_64-w64-mingw32\lib

clean:
	rm main.exe