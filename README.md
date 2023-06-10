# discard serverプログラム
```
Usage: server [-b bufsize (4k)] [-p port (1234)] [-c run_cpu]
-b bufsize:    read buffer size (may add k for kilo, m for mega)
-p port:       port number (1234)
-c run_cpu:    specify server run cpu (in child proc)
```

# 起動例

```
./discard-server
```

起動後、port 1234で接続を待つ。
送られてきたデータは読むがないもしないで捨てる。
