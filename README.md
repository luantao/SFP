# SFP
SocketLock For PHP (高并发网络锁 For PHP)

# 使用方法

```
$cacheLock = new CacheLock();
$cacheLock->lock('key');
......需要被锁定执行的代码
$cacheLock->unlock();

```
