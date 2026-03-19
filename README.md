# SoulBypass - Soul App 风控绕过模块

## 快速开始（3分钟搞定）

### 步骤1：创建 GitHub 仓库

1. 打开 https://github.com/new
2. Repository name 填 `SoulBypass`
3. 选择 **Public**（免费使用 Actions）
4. 点击 **Create repository**

### 步骤2：上传代码

1. 在新仓库页面，点击 **"uploading an existing file"**
2. 把 `SoulBypass-Project.zip` 解压后的**所有文件和文件夹**拖进去
   - 包括：`.github/`、`module/`、`zygisk/`（空文件夹也要）、`*.sh`、`*.md`、`*.prop`
3. 点击 **Commit changes**

### 步骤3：等待自动编译

1. 点击仓库顶部的 **Actions** 标签
2. 等待 2-3 分钟，直到变成绿色对勾 ✅
3. 点击最新的运行记录
4. 页面底部 **Artifacts** 区域，下载 **SoulBypass-Module**

### 步骤4：刷入 Magisk

1. 解压下载的 zip，得到 `SoulBypass-v1.0.0.zip`
2. 传到手机，Magisk → 模块 → 从本地安装
3. 重启手机

---

## ⚠️ 重要提示

**必须上传完整的文件夹结构：**
```
SoulBypass/
├── .github/workflows/main.yml    # GitHub Actions 配置
├── module/                        # C++ 源码
│   └── src/main/cpp/
│       ├── CMakeLists.txt
│       ├── src/
│       ├── include/
│       └── ...
├── zygisk/                        # 输出目录（可以为空）
├── build.sh                       # 构建脚本
├── customize.sh                   # Magisk 安装脚本
├── module.prop                    # 模块信息
└── README.md                      # 本文件
```

---

## 功能特性

- **Seccomp-BPF 系统调用拦截**
- **19 路径敏感过滤器**
- **PC 溯源判定**
- **memfd 文件伪造**

---

## 许可证

MIT License - 仅供学习研究使用
