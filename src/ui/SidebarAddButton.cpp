#include "SidebarAddButton.h"

#include "ServiceButton.h"

#include <QEvent>

SidebarAddButton::SidebarAddButton(QWidget *parent)
    : QToolButton(parent)
{
    setText(QStringLiteral("+"));
    setToolTip(tr("Add service"));
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setVisible(false);

    updateMetrics();
    updateStyle();

    if (parent != nullptr) {
        parent->setMouseTracking(true);
        parent->installEventFilter(this);
    }
}

QSize SidebarAddButton::sizeHint() const
{
    return m_actionSize;
}

QSize SidebarAddButton::minimumSizeHint() const
{
    return m_actionSize;
}

bool SidebarAddButton::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget()) {
        switch (event->type()) {
        case QEvent::Enter:
            setVisible(true);
            break;
        case QEvent::Leave:
            setVisible(false);
            break;
        default:
            break;
        }
    }

    return QToolButton::eventFilter(watched, event);
}

void SidebarAddButton::updateMetrics()
{
    const QSize serviceIconTarget = ServiceButton::iconTouchTargetSize();
    const int side = (serviceIconTarget.width() * kSizeNumerator) / kSizeDenominator;
    m_actionSize = QSize(side, side);
    setFixedSize(m_actionSize);
}

void SidebarAddButton::updateStyle()
{
    setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  border-radius: 6px;"
        "  font-size: 14px;"
        "  color: #ddd;"
        "}"
        "QToolButton:hover {"
        "  background: #444;"
        "}"));
}
