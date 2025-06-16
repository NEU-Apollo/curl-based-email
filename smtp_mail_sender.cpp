#include "smtp_mail_sender.h"
#include "base64.h"
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <random>
#include <ctime>
#include <unistd.h>
#include <filesystem>

namespace smtp
{
    size_t EmailSender::get_class_count() const
    {
        std::vector<size_t> values = {html_text_body.size(), attachement_filepath.size(), plain_text_body.size()};

        size_t true_count = 0;
        for (bool value : values)
        {
            if (value != 0)
                true_count++;
        }

        return true_count;
    }

    std::string EmailSender::generateBoundary() const
    {
        std::stringstream ss;
        // 固定前缀保证以字母开头
        ss << "BOUNDARY_";
        // 当前时间戳，增加唯一性
        ss << std::time(nullptr);
        // 添加进程ID，进一步增强唯一性
        ss << "_" << getpid();
        // 添加随机数
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> dis(1000, 9999);
        ss << "_" << dis(gen);

        return ss.str();
    }

    // file_path: 文件完整路径
    // 返回值: 读取到的内容（可能包含 '\0'，适合做base64等二进制操作）
    std::string EmailSender::read_file_to_string(const std::string &file_path) const
    {
        std::ifstream file(file_path, std::ios::binary); // 二进制方式打开
        if (!file)
        {
            throw std::runtime_error("无法打开文件: " + file_path);
        }

        // 预先确定文件大小，分配空间，读取全部内容
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer;
        buffer.resize(size);

        if (!file.read(&buffer[0], size))
        {
            throw std::runtime_error("读取文件失败: " + file_path);
        }
        return buffer;
    }

    struct UploadStatus
    {
        size_t bytes_read;
        const std::string payload;
    };

    size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
    {
        UploadStatus *upload_ctx = static_cast<UploadStatus *>(userp);

        if (upload_ctx->bytes_read >= upload_ctx->payload.size())
        {
            return 0;
        }
        size_t bytes_to_copy = std::min(size * nmemb, upload_ctx->payload.size() - upload_ctx->bytes_read);

        std::memcpy(ptr, upload_ctx->payload.data() + upload_ctx->bytes_read, bytes_to_copy);
        upload_ctx->bytes_read += bytes_to_copy;
        return bytes_to_copy;
    }
    
    bool EmailSender::send_email()
    {
        if (is_empty())
        {
            throw std::runtime_error("EmailInfo::contentclass is empty");
        }

        CURL *curl = curl_easy_init();
        if (!curl)
        {
            return false;
        }
        CURLcode res;
        curl_slist *recipients = nullptr;

        std::string payload;
        payload += "To: " + to + "\r\n";
        payload += "From: " + from + "\r\n";
        payload += "Subject: " + subject + "\r\n";
        payload += "MIME-Version: 1.0\r\n";

        std::string boundary_mixed = generateBoundary();       // mixed 边界
        std::string boundary_alternative = generateBoundary(); // alternative 边界
        bool has_plain = !plain_text_body.empty();
        bool has_html = !html_text_body.empty();
        bool has_attachment = !attachement_filepath.empty();
        bool is_alternative = has_plain && has_html;
        bool is_mixed = has_attachment || is_alternative;

        if (is_mixed) {
            payload += "Content-Type: multipart/mixed; boundary=\"" + boundary_mixed + "\"\r\n\r\n";
            if (is_alternative) {
                payload += "--" + boundary_mixed + "\r\n";
                payload += "Content-Type: multipart/alternative; boundary=\"" + boundary_alternative + "\"\r\n\r\n";

                // plain part
                payload += "--" + boundary_alternative + "\r\n";
                payload += "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
                payload += plain_text_body + "\r\n";

                // html part
                payload += "--" + boundary_alternative + "\r\n";
                payload += "Content-Type: text/html; charset=UTF-8\r\n\r\n";
                payload += html_text_body + "\r\n";

                payload += "--" + boundary_alternative + "--\r\n";
            } else if (has_plain) {
                payload += "--" + boundary_mixed + "\r\n";
                payload += "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
                payload += plain_text_body + "\r\n";
            } else if (has_html) {
                payload += "--" + boundary_mixed + "\r\n";
                payload += "Content-Type: text/html; charset=UTF-8\r\n\r\n";
                payload += html_text_body + "\r\n";
            }

            // 附件部分
            if (has_attachment) {
                payload += "--" + boundary_mixed + "\r\n";
                std::string attachment_content = read_file_to_string(attachement_filepath);
                std::string encoded = base64_encode(
                    reinterpret_cast<const unsigned char *>(attachment_content.data()),
                    attachment_content.size());

                std::string filename = std::filesystem::path(attachement_filepath).filename().string();

                payload += "Content-Type: application/octet-stream; name=\"" + filename + "\"\r\n";
                payload += "Content-Transfer-Encoding: base64\r\n";
                payload += "Content-Disposition: attachment; filename=\"" + filename + "\"\r\n\r\n";
                payload += encoded + "\r\n";
            }

            payload += "--" + boundary_mixed + "--\r\n";
        }
        else if (has_plain) {
            payload += "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
            payload += plain_text_body + "\r\n";
        }
        else if (has_html) {
            payload += "Content-Type: text/html; charset=UTF-8\r\n\r\n";
            payload += html_text_body + "\r\n";
        }
        else {
            // 没有内容，理论上上层已经做过判断
            throw std::runtime_error("邮件内容为空");
        }

        UploadStatus upload_ctx = {0, payload};

        curl_easy_setopt(curl, CURLOPT_URL, smtp_url.c_str());                 // 设置SMTP服务器地址
        curl_easy_setopt(curl, CURLOPT_PORT, 587L);                                 // 设置SMTP端口
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());            // 设置SMTP用户名
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());            // 设置SMTP密码（授权码）
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + from + ">").c_str()); // 设置发件人邮箱
        recipients = curl_slist_append(nullptr, ("<" + to + ">").c_str());     // 构造收件人邮箱列表
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);                      // 设置收件人邮箱
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);                    // 启用SSL/TLS加密传输
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                         // 跳过SSL证书校验
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);                         // 跳过SSL主机名校验
        curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");                // 指定 LOGIN 认证方式
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);               // 设置回调函数，用于提供邮件内容读取
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);                      // 设置回调函数的上下文数据，传递upload_ctx
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);                                 // 设置为上传模式（即发邮件）
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);                                // 开启调试信息输出

        res = curl_easy_perform(curl);   // 正式执行SMTP流程（连接、认证、发信）。libcurl会自动完成所有SMTP细节
        curl_slist_free_all(recipients); // 释放收件人列表内存
        curl_easy_cleanup(curl);         // 清理CURL句柄

        if (res != CURLE_OK)
        {
            std::cerr << "❌ curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        return true;
    }
}