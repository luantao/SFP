# SFP
SocketLock For PHP (高并发网络锁 For PHP)

多台服务器间通信,文件锁和内存锁的替代方案

# 使用方法

```php
<?php

$cacheLock = new SocketLock();
$cacheLock->lock('key');
......需要被锁定执行的代码
$cacheLock->unlock();

```

## 其他说明

由于windows下cygwin不支持epoll,用select实现

如果在linux下,根据自己的需求,可以将select换成epoll方式提高连接数量级

## 后期版本目标
