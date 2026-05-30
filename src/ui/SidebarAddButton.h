#pragma once

#include <QToolButton>

class SidebarAddButton : public QToolButton
{
    Q_OBJECT

public:
    static constexpr int kSizeNumerator = 7;
    static constexpr int kSizeDenominator = 10;

    explicit SidebarAddButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateMetrics();
    void updateStyle();

    QSize m_actionSize;
};
