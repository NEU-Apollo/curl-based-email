#include <iostream>
#include "smtp_mail_sender.h" 

int main() {
    try {
        // 初始化基本邮件信息
        smtp::EmailSender sender(
            "smtp://smtp.qq.com:587",         // SMTP 地址
            "xxxxx@qq.com",              // 用户名
            "************",               // 授权码
            "xxxxx@qq.com",              // 发件人
            "xxxxx@qq.com",              // 收件人
            "测试邮件 - 来自 EmailSender"      // 邮件主题
        );

        sender.set_html_text_body("<html><body><h2 style='color:blue;'>这是 HTML 格式的邮件</h2></body></html>");
        sender.set_attachment("/home/xxx/curl-based-email/测试邮件.txt");

        // 发送邮件
        if (sender.send_email()) {
            std::cout << "邮件发送成功！" << std::endl;
        } else {
            std::cout << "邮件发送失败！" << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
    }

    return 0;
}
