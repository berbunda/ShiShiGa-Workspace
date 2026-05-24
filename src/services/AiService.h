#pragma once

#include <QHash>
#include <QString>
#include <QUrl>

namespace AiService {

inline constexpr char ChatGpt[] = "chatgpt";
inline constexpr char Claude[] = "claude";
inline constexpr char Gemini[] = "gemini";
inline constexpr char DeepSeek[] = "deepseek";

inline QUrl urlFor(const QString &serviceId)
{
    static const QHash<QString, QUrl> urls = {
        {QString::fromLatin1(ChatGpt), QUrl(QStringLiteral("https://chatgpt.com"))},
        {QString::fromLatin1(Claude), QUrl(QStringLiteral("https://claude.ai"))},
        {QString::fromLatin1(Gemini), QUrl(QStringLiteral("https://gemini.google.com"))},
        {QString::fromLatin1(DeepSeek), QUrl(QStringLiteral("https://chat.deepseek.com"))},
    };

    return urls.value(serviceId);
}

} // namespace AiService
