# Milestone 3 安装说明

## 依赖要求

Milestone 3 引入了数据库功能，需要安装 SQLite3 开发库。

### 安装方法

#### 方法 1：使用安装脚本

```bash
./scripts/install_deps.sh
```

#### 方法 2：手动安装

```bash
sudo apt-get update
sudo apt-get install -y libsqlite3-dev
```

### 验证安装

安装完成后，可以验证 SQLite3 是否可用：

```bash
# 检查运行时库
dpkg -l | grep libsqlite3

# 应该看到:
# libsqlite3-0       - SQLite 3 shared library
# libsqlite3-dev     - SQLite 3 development files
```

### 编译项目

依赖安装完成后，运行：

```bash
./scripts/build.sh
```

### 运行服务器

```bash
./scripts/run.sh
```

### 访问聊天界面

浏览器访问：`http://localhost:8080`

## 常见问题

### Q: 没有 sudo 权限怎么办？

A: 请联系系统管理员安装 `libsqlite3-dev` 包，或者在有 sudo 权限的环境中运行。

### Q: 编译时还是提示找不到 sqlite3.h？

A: 请确认 libsqlite3-dev 已正确安装：
```bash
dpkg -L libsqlite3-dev | grep sqlite3.h
```

应该显示类似：`/usr/include/sqlite3.h`

### Q: 链接时报错？

A: 确保 CMakeLists.txt 中包含了 `sqlite3` 库：
```cmake
target_link_libraries(${PROJECT_NAME} 
    pthread
    sqlite3
    stdc++fs
    ${CMAKE_DL_LIBS}
)
```
