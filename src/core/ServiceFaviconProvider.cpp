#include "ServiceFaviconProvider.h"

#include <QGuiApplication>
#include <QPainter>
#include <QWebEnginePage>

ServiceFaviconProvider::ServiceFaviconProvider(QObject *parent)
    : QObject(parent)
{
}

QIcon ServiceFaviconProvider::placeholderIcon(const QString &displayName, const int logicalSize)
{
    const qreal devicePixelRatio = qApp != nullptr ? qApp->devicePixelRatio() : 1.0;
    const int pixelSize = qMax(1, qRound(logicalSize * devicePixelRatio));

    QPixmap pixmap(pixelSize, pixelSize);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF bounds(0.0, 0.0, logicalSize, logicalSize);
    painter.setBrush(QColor(0x44, 0x44, 0x44));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(bounds, 8.0, 8.0);

    painter.setPen(QColor(0xcc, 0xcc, 0xcc));
    QFont font = painter.font();
    font.setPixelSize(qMax(12, logicalSize / 2));
    font.setBold(true);
    painter.setFont(font);

    const QString letter = displayName.isEmpty() ? QStringLiteral("?") : displayName.left(1).toUpper();
    painter.drawText(bounds, Qt::AlignCenter, letter);

    return QIcon(pixmap);
}

QIcon ServiceFaviconProvider::normalizedIcon(const QIcon &icon, const int logicalSize)
{
    if (icon.isNull())
        return {};

    const qreal devicePixelRatio = qApp != nullptr ? qApp->devicePixelRatio() : 1.0;
    const QSize targetSize(qMax(1, qRound(logicalSize * devicePixelRatio)),
                           qMax(1, qRound(logicalSize * devicePixelRatio)));

    QPixmap pixmap = icon.pixmap(targetSize);
    if (pixmap.isNull())
        return {};

    if (pixmap.size() != targetSize) {
        pixmap = pixmap.scaled(targetSize,
                               Qt::KeepAspectRatioByExpanding,
                               Qt::SmoothTransformation);
        const int xOffset = (pixmap.width() - targetSize.width()) / 2;
        const int yOffset = (pixmap.height() - targetSize.height()) / 2;
        pixmap = pixmap.copy(xOffset, yOffset, targetSize.width(), targetSize.height());
    }

    pixmap.setDevicePixelRatio(devicePixelRatio);
    return QIcon(pixmap);
}

void ServiceFaviconProvider::bindPage(const QString &serviceId, QWebEnginePage *page)
{
    if (page == nullptr)
        return;

    m_pages.insert(serviceId, page);

    connect(page, &QWebEnginePage::iconChanged, this,
            [this, serviceId](const QIcon &icon) { applyIcon(serviceId, icon); });

    applyIcon(serviceId, page->icon());
}

void ServiceFaviconProvider::unbindPage(const QString &serviceId)
{
    m_pages.remove(serviceId);
}

QIcon ServiceFaviconProvider::cachedIcon(const QString &serviceId) const
{
    return m_cache.value(serviceId);
}

bool ServiceFaviconProvider::hasCachedIcon(const QString &serviceId) const
{
    return m_cache.contains(serviceId);
}

void ServiceFaviconProvider::applyIcon(const QString &serviceId, const QIcon &pageIcon)
{
    if (pageIcon.isNull())
        return;

    const QIcon normalized = normalizedIcon(pageIcon);
    if (normalized.isNull())
        return;

    const auto cached = m_cache.constFind(serviceId);
    if (cached != m_cache.constEnd() && cached->cacheKey() == normalized.cacheKey())
        return;

    m_cache.insert(serviceId, normalized);
    emit faviconChanged(serviceId, normalized);
}
