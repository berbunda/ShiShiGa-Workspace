#pragma once

#include "profile/ProfileManager.h"
#include "services/ServiceState.h"

#include <QHash>
#include <QShowEvent>
#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;
class ServiceManager;

class ProfileManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileManagerWidget(ServiceManager *serviceManager, QWidget *parent = nullptr);

    void refreshProfileList();
    void refreshRuntimeStates();

protected:
    void showEvent(QShowEvent *event) override;

public slots:
    void requestDiskSizeRefresh();

private slots:
    void onProfileActionClicked();
    void onProfileDataCleared(const QString &profileId);
    void onServiceStateChanged(const QString &serviceId, ServiceState state);
    void onServiceAdded(const QString &serviceId);
    void onServiceRemoved(const QString &serviceId);
    void onDiskSizeReady(const QString &profileId, qint64 bytes);

private:
    enum class ProfileActionKind {
        ClearProfile,
        SignIn,
        None,
    };

    void setupUi();
    void connectSignals();
    ProfileActionKind actionKindFor(const ManagedProfileInfo &profile) const;
    QString actionButtonText(ProfileActionKind kind) const;
    bool isActionEnabled(ProfileActionKind kind, const ManagedProfileInfo &profile) const;
    void updateActionButton(int row);
    void updateRuntimeStateCell(int row);
    int rowForServiceId(const QString &serviceId) const;
    int rowForProfileId(const QString &profileId) const;
    void startDiskSizeScan(const QString &profileId, const QString &rootPath);
    void setDiskSizeCell(int row, qint64 bytes, bool scanning);
    static QString formatByteSize(qint64 bytes);

    ServiceManager *m_serviceManager = nullptr;
    bool m_initialSizeScanDone = false;
    QTableWidget *m_table = nullptr;
    QPushButton *m_refreshSizesButton = nullptr;
    QLabel *m_sizeHintLabel = nullptr;

    QList<ManagedProfileInfo> m_profiles;
    QHash<QString, qint64> m_cachedSizes;
    QHash<QString, bool> m_sizeScanInFlight;
};
