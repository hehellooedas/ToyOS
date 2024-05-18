## A Toy OS to learn x86 and OS



### How to run Toy OS

You need install the bochs with debugger first.

Build your own Bochs?[Click here](https://blog.csdn.net/qq_61653333/article/details/136962598?spm=1001.2014.3001.5501)



```shell
make check #To check you have debug environment or not
```



If you have passed the test,You can directly run the following script to start the kernel.

```
make bochs
```



![after run bochs](./doc/photo/bochs.png)

Choose to use Intel or AMD CPU. (But AMD CPU cannot get the full experience)



After starting Bochs, use "continue" to run the kernel.

