# utils

## Description

本项目包含了用于以下处理逻辑

    int func(const char *url, const char *进程名) {
    if (url != NULL && url !="") { target_url = url; }
    1. 读取/sys/class/dmi/id/xxx_serial 文件内容, 拼接sn
    2. 发送给指定的URL地址: sn, 进程名
        Sn 拼接后编码
    3. 接收返回值
    4. 解析的字符串match字段匹配, kill 指定进程名称

    如果初始化传递了默认进程名.
    }

## 编译说明

### 安装编译依赖

基础依赖

```bash
sudo apt update
sudo apt install libcurl4-openssl-dev cmake curl
```

如果需要使用 `libcurl` 的 SSL 功能, 还需要安装 OpenSSL 开发包:

```bash
sudo apt install -y libssl-dev
```

如果需要进行单元测试, 还需要安装 `golang` 环境

```bash
sudo apt install golang
```

### 编译项目

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行测试

```bash
make test
```

测试日志的输出路径为 `build/Testing/Temporary`。
