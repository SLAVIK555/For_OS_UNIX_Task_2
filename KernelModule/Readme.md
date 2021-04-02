Compilation:
slava@debian:~/Source/For_OS_UNIX_Task_2/KernelModule$ make
make -C /lib/modules/4.19.0-14-amd64/build M=/home/slava/Source/For_OS_UNIX_Task_2/KernelModule modules
make[1]: вход в каталог «/usr/src/linux-headers-4.19.0-14-amd64»
  CC [M]  /home/slava/Source/For_OS_UNIX_Task_2/KernelModule/KM.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/slava/Source/For_OS_UNIX_Task_2/KernelModule/KM.mod.o
  LD [M]  /home/slava/Source/For_OS_UNIX_Task_2/KernelModule/KM.ko
make[1]: выход из каталога «/usr/src/linux-headers-4.19.0-14-amd64»

Загрузку модуля в в ядро и его выгрузку добавлю позже, так как у меня включен UEFI_SECURE_BOOT и это препятствует загрузке модуля и поэтому мне нужно будет переключиться на винду на виртуалку
