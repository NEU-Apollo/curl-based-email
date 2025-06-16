# curl-based-email
learn from 
【C++实现邮件发送：基于 curl 库的 Email 模块封装】 https://www.bilibili.com/video/BV13sMbzMEsy/?share_source=copy_web&vd_source=3d302ad27dec2b86c804c1a070a39ec0

这是一个使用 C++ 和 libcurl 实现的 SMTP 邮件发送工具，支持以下功能：

- 支持 SMTP AUTH 登录（如 QQ 邮箱、163、Gmail 等）
- 支持发送纯文本和 HTML 正文
- 支持发送附件
- 自动构建 MIME 邮件结构

---

## 📦 功能特性

| 功能        | 支持情况 |
|-------------|----------|
| SMTP 认证发送 | ✅       |
| 纯文本正文    | ✅       |
| HTML 正文     | ✅       |
| 附件发送      | ✅       |

---

## 🛠️ 构建方式

### 依赖项

- **C++17**
- **libcurl**

### 安装 libcurl（Ubuntu）

```bash
sudo apt update
sudo apt-get install libcurl4-openssl-dev

## 编译
cmake -B build
cmake --build build --parallel