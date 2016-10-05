##MemoryPatcher

inline hook

memory read/write

dump memory

print memory

easy to use

Configuration:
```
Config.h:
#ifndef __CONFIG_H_
#define __CONFIG_H_

/*
	For installation
*/
#define PATCH_TARGET_PATH "/Applications/Image2Icon.app/Contents/MacOS"
#define PATCH_TARGET_EXE  "Image2Icon"

/*
	Debug log
*/
#define DEBUG_LOG 1

/*
	Startup mode
	0 run in standalone thread
	1 run in main thread
*/
#define STARTUP_MODE 0

/*
	Delay for n seconds
*/
#define PROCESS_DELAY 0

#endif 
```

Step 1:
```
void mymain(){
	printf("Oh!!!\n");
}

void yourcode(int pid, uint64_t basicaddress) {
	printf("Hello world!\n");
	void *entrypoint = (void*)(basicaddress+0x4ca04);
	inline_hook(entrypoint, mymain);
}

```

Step 2:
```
make && make install
```

Step 3:
```
lockairs-iMac:MacOS xxxx$ ./xxxx
Thread         : standalone thread
Target pid     : 5402
Base address   : 10128d000
Hello world!
Oh!!!
lockairs-iMac:MacOS lockair$ 
```