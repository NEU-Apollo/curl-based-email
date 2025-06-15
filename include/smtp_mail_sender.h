#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>

namespace smtp
{
    struct EmailInfo
    {
        class EmailContentClass
        {
            bool is_html = false;         // 是否HTML格式
            bool has_attachments = false; // 是否有附件
            bool is_plain_text = false;

            size_t get_class_count() const
            {
                std::vector<bool> values = {is_html, has_attachments, is_plain_text};

                size_t true_count = 0;
                for (bool value : values)
                {
                    if (value)
                        true_count++;
                }

                return true_count;
            }

        public:
            void set_html()
            {
                this->is_html = true;
            }
            void set_attachments()
            {
                this->has_attachments = true;
            }
            void set_plain_text()
            {
                this->is_plain_text = true;
            }

            bool get_has_html() const { return is_html; }
            bool get_has_attachments() const { return has_attachments; }
            bool get_has_plain_text() const { return is_plain_text; }

            bool is_empty() const
            {
                return get_class_count() == 0;
            }

            bool is_hybrid() const
            {
                return get_class_count() >= 2; // 判断是否有两个或更多为true
            }
        };

        std::string smtp_url;        // SMTP地址，例如 smtp://smtp.qq.com:587
        std::string username;        // 邮箱账号
        std::string password;        // 邮箱密码（授权码）
        std::string from;            // 发件人邮箱
        std::string to;              // 收件人邮箱
        std::string subject;         // 邮件主题
        std::string plain_text_body; // 邮件内容（纯文本）
        std::string html_text_body;  // 邮件内容（HTML）
        std::string attachement_filepath; //  附件路径
        EmailContentClass contentclass; // 邮件内容类型
    };

    bool send_email(const EmailInfo &info);
}