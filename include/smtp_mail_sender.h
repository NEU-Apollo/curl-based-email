#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>

namespace smtp
{
    class EmailSender {
        std::string smtp_url;        // SMTP地址，例如 smtp://smtp.qq.com:587
        std::string username;        // 邮箱账号
        std::string password;        // 邮箱密码（授权码）
        std::string from;            // 发件人邮箱
        std::string to;              // 收件人邮箱
        std::string subject;         // 邮件主题
        std::string plain_text_body; // 邮件内容（纯文本）
        std::string html_text_body;  // 邮件内容（HTML）
        std::string attachement_filepath; //  附件路径
        std::string generateBoundary() const;
        std::string read_file_to_string(const std::string &file_path) const;
        bool is_empty() {
            return get_class_count() == 0;
        }
        bool is_hybrid() {
            return get_class_count() > 1;
        }
    public:
        EmailSender(const std::string &smtp_url,
                    const std::string &username,
                    const std::string &password,
                    const std::string &from,
                    const std::string &to,
                    const std::string &subject)
            : smtp_url(smtp_url),
            username(username),
            password(password),
            from(from),
            to(to),
            subject(subject) {}

        size_t get_class_count() const;
        bool send_email();

        // 可以添加额外的setter函数设置可选项
        void set_plain_text_body(const std::string &text) {
            plain_text_body = text;
        }

        void set_html_text_body(const std::string &html) {
            html_text_body = html;
        }

        void set_attachment(const std::string &filepath) {
            attachement_filepath = filepath;
        }
    };
}