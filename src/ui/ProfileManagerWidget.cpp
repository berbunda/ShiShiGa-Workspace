#include "ProfileManagerWidget.h"

#include "core/ServiceManager.h"
#include "logging/CrashLogger.h"
#include "services/ServiceRegistry.h"
#include "services/ServiceState.h"

#include <QDir>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtConcurrent>

namespace {

enum Column {
    ServiceName = 0,
    ProfileId,
    ProfilePath,
    RuntimeState,
    DiskSize,
    Action,
    ColumnCount,
};

} // namespace

ProfileManagerWidget::ProfileManagerWidget(ServiceManager *serviceManager, QWidget *parent)
    : QWidget(parent)
    , m_serviceManager(serviceManager)
{
    setupUi();
    connectSignals();
    refreshProfileList();
}

void ProfileManagerWidget::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);

    m_sizeHintLabel = new QLabel(
        tr("Disk usage is calculated on demand. Open this tab or click \"Refresh sizes\" to update."),
        this);
    m_sizeHintLabel->setWordWrap(true);
    m_sizeHintLabel->setStyleSheet(QStringLiteral("color: #888;"));

    m_table = new QTableWidget(this);
    m_table->setColumnCount(ColumnCount);
    m_table->setHorizontalHeaderLabels({
        tr("Service"),
        tr("Profile ID"),
        tr("Profile path"),
        tr("State"),
        tr("Size on disk"),
        tr("Action"),
    });
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(ProfilePath, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(ServiceName, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(ProfileId, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(RuntimeState, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(DiskSize, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(Action, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);

    m_refreshSizesButton = new QPushButton(tr("Refresh sizes"), this);

    auto *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(m_refreshSizesButton);
    toolbarLayout->addStretch();

    rootLayout->addWidget(m_sizeHintLabel);
    rootLayout->addLayout(toolbarLayout);
    rootLayout->addWidget(m_table, 1);

    connect(m_refreshSizesButton, &QPushButton::clicked, this, &ProfileManagerWidget::requestDiskSizeRefresh);
}

void ProfileManagerWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_initialSizeScanDone) {
        m_initialSizeScanDone = true;
        requestDiskSizeRefresh();
    }
}

void ProfileManagerWidget::connectSignals()
{
    connect(&ProfileManager::instance(), &ProfileManager::profileDataCleared,
            this, &ProfileManagerWidget::onProfileDataCleared);

    if (m_serviceManager == nullptr)
        return;

    connect(m_serviceManager, &ServiceManager::serviceStateChanged,
            this, &ProfileManagerWidget::onServiceStateChanged);
    connect(m_serviceManager, &ServiceManager::serviceAdded,
            this, &ProfileManagerWidget::onServiceAdded);
    connect(m_serviceManager, &ServiceManager::serviceRemoved,
            this, &ProfileManagerWidget::onServiceRemoved);
}

void ProfileManagerWidget::refreshProfileList()
{
    m_profiles = ProfileManager::instance().registeredProfiles();
    m_table->setRowCount(m_profiles.size());

    for (int row = 0; row < m_profiles.size(); ++row) {
        const ManagedProfileInfo &profile = m_profiles.at(row);

        m_table->setItem(row, ServiceName, new QTableWidgetItem(profile.serviceDisplayName));
        m_table->setItem(row, ProfileId, new QTableWidgetItem(profile.profileId));
        m_table->setItem(row, ProfilePath, new QTableWidgetItem(profile.profileRootPath));
        m_table->setItem(row, RuntimeState,
                         new QTableWidgetItem(m_serviceManager != nullptr
                                                  ? m_serviceManager->profileRuntimeStateLabel(profile.serviceId)
                                                  : QStringLiteral("Unloaded")));

        const qint64 cachedSize = m_cachedSizes.value(profile.profileId, -1);
        setDiskSizeCell(row, cachedSize, cachedSize < 0);

        auto *actionButton = new QPushButton(this);
        connect(actionButton, &QPushButton::clicked, this, &ProfileManagerWidget::onProfileActionClicked);
        m_table->setCellWidget(row, Action, actionButton);
        updateActionButton(row);
    }
}

void ProfileManagerWidget::refreshRuntimeStates()
{
    for (int row = 0; row < m_profiles.size(); ++row)
        updateRuntimeStateCell(row);
}

void ProfileManagerWidget::requestDiskSizeRefresh()
{
    m_cachedSizes.clear();

    for (int row = 0; row < m_profiles.size(); ++row) {
        const ManagedProfileInfo &profile = m_profiles.at(row);
        setDiskSizeCell(row, -1, true);
        startDiskSizeScan(profile.profileId, profile.profileRootPath);
    }
}

ProfileManagerWidget::ProfileActionKind ProfileManagerWidget::actionKindFor(
    const ManagedProfileInfo &profile) const
{
    const ProfileDataState dataState = ProfileManager::instance().dataState(profile.profileId);
    if (dataState == ProfileDataState::Present)
        return ProfileActionKind::ClearProfile;

    return ProfileActionKind::SignIn;
}

QString ProfileManagerWidget::actionButtonText(const ProfileActionKind kind) const
{
    switch (kind) {
    case ProfileActionKind::ClearProfile:
        return tr("Clear Profile");
    case ProfileActionKind::SignIn:
        return tr("Sign in");
    case ProfileActionKind::None:
        return tr("Unavailable");
    }
    return {};
}

bool ProfileManagerWidget::isActionEnabled(const ProfileActionKind kind,
                                           const ManagedProfileInfo &profile) const
{
    if (kind == ProfileActionKind::None)
        return false;

    if (kind == ProfileActionKind::SignIn)
        return ServiceRegistry::isLaunchable(profile.serviceId);

    return true;
}

void ProfileManagerWidget::updateActionButton(const int row)
{
    if (row < 0 || row >= m_profiles.size())
        return;

    auto *button = qobject_cast<QPushButton *>(m_table->cellWidget(row, Action));
    if (button == nullptr)
        return;

    const ManagedProfileInfo &profile = m_profiles.at(row);
    const ProfileActionKind kind = actionKindFor(profile);
    button->setText(actionButtonText(kind));
    button->setEnabled(isActionEnabled(kind, profile));
    button->setProperty("serviceId", profile.serviceId);
    button->setProperty("profileId", profile.profileId);
    button->setProperty("actionKind", static_cast<int>(kind));
}

void ProfileManagerWidget::updateRuntimeStateCell(const int row)
{
    if (row < 0 || row >= m_profiles.size())
        return;

    QTableWidgetItem *item = m_table->item(row, RuntimeState);
    if (item == nullptr)
        return;

    const ManagedProfileInfo &profile = m_profiles.at(row);
    item->setText(m_serviceManager != nullptr
                      ? m_serviceManager->profileRuntimeStateLabel(profile.serviceId)
                      : QStringLiteral("Unloaded"));
}

int ProfileManagerWidget::rowForServiceId(const QString &serviceId) const
{
    for (int row = 0; row < m_profiles.size(); ++row) {
        if (m_profiles.at(row).serviceId == serviceId)
            return row;
    }
    return -1;
}

int ProfileManagerWidget::rowForProfileId(const QString &profileId) const
{
    for (int row = 0; row < m_profiles.size(); ++row) {
        if (m_profiles.at(row).profileId == profileId)
            return row;
    }
    return -1;
}

void ProfileManagerWidget::startDiskSizeScan(const QString &profileId, const QString &rootPath)
{
    if (m_sizeScanInFlight.value(profileId))
        return;

    m_sizeScanInFlight.insert(profileId, true);

    auto *watcher = new QFutureWatcher<qint64>(this);
    connect(watcher, &QFutureWatcher<qint64>::finished, this, [this, watcher, profileId]() {
        const qint64 bytes = watcher->result();
        m_sizeScanInFlight.remove(profileId);
        onDiskSizeReady(profileId, bytes);
        watcher->deleteLater();
    });

    watcher->setFuture(QtConcurrent::run([rootPath]() {
        const ProfileManager &manager = ProfileManager::instance();
        const QDir rootDir(rootPath);
        if (!rootDir.exists())
            return qint64(0);

        return manager.directorySizeOnDisk(rootDir.filePath(QStringLiteral("storage")))
            + manager.directorySizeOnDisk(rootDir.filePath(QStringLiteral("cache")));
    }));
}

void ProfileManagerWidget::setDiskSizeCell(const int row, const qint64 bytes, const bool scanning)
{
    QTableWidgetItem *item = m_table->item(row, DiskSize);
    if (item == nullptr) {
        item = new QTableWidgetItem();
        m_table->setItem(row, DiskSize, item);
    }

    if (scanning)
        item->setText(tr("Calculating…"));
    else if (bytes < 0)
        item->setText(tr("—"));
    else
        item->setText(formatByteSize(bytes));
}

void ProfileManagerWidget::onDiskSizeReady(const QString &profileId, const qint64 bytes)
{
    m_cachedSizes.insert(profileId, bytes);

    const int row = rowForProfileId(profileId);
    if (row >= 0)
        setDiskSizeCell(row, bytes, false);
}

QString ProfileManagerWidget::formatByteSize(const qint64 bytes)
{
    static const QStringList units = {
        QStringLiteral("B"),
        QStringLiteral("KB"),
        QStringLiteral("MB"),
        QStringLiteral("GB"),
        QStringLiteral("TB"),
    };

    double value = static_cast<double>(bytes);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < units.size() - 1) {
        value /= 1024.0;
        ++unitIndex;
    }

    return QStringLiteral("%1 %2").arg(QString::number(value, 'f', value >= 10.0 ? 0 : 1), units.at(unitIndex));
}

void ProfileManagerWidget::onProfileActionClicked()
{
    auto *button = qobject_cast<QPushButton *>(sender());
    if (button == nullptr)
        return;

    const QString serviceId = button->property("serviceId").toString();
    const ProfileActionKind kind = static_cast<ProfileActionKind>(button->property("actionKind").toInt());

    const int row = rowForServiceId(serviceId);
    if (row < 0 || row >= m_profiles.size())
        return;

    const ManagedProfileInfo &profile = m_profiles.at(row);

    if (kind == ProfileActionKind::SignIn) {
        if (m_serviceManager == nullptr)
            return;

        m_serviceManager->signInToService(serviceId);
        refreshRuntimeStates();
        return;
    }

    if (kind != ProfileActionKind::ClearProfile)
        return;

    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        tr("Clear profile"),
        tr("Profile %1 will be cleared. All authorization data will be removed. Continue?")
            .arg(profile.serviceDisplayName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    QString errorMessage;
    if (m_serviceManager == nullptr) {
        errorMessage = tr("Service manager is unavailable.");
        CrashLogger::instance().logOperationalError(
            QStringLiteral("profile"),
            QStringLiteral("Clear profile failed for %1: service manager unavailable")
                .arg(profile.profileId));
        QMessageBox::critical(this, tr("Clear profile failed"), errorMessage);
        return;
    }

    if (!m_serviceManager->clearServiceProfile(serviceId, &errorMessage)) {
        QMessageBox::critical(this,
                              tr("Clear profile failed"),
                              errorMessage.isEmpty() ? tr("Unable to clear the profile.") : errorMessage);
        return;
    }

    m_cachedSizes.remove(profile.profileId);
    setDiskSizeCell(row, 0, false);
    updateActionButton(row);
    refreshRuntimeStates();
}

void ProfileManagerWidget::onProfileDataCleared(const QString &profileId)
{
    const int row = rowForProfileId(profileId);
    if (row < 0)
        return;

    updateActionButton(row);
}

void ProfileManagerWidget::onServiceStateChanged(const QString &serviceId, ServiceState state)
{
    Q_UNUSED(state);
    const int row = rowForServiceId(serviceId);
    if (row >= 0)
        updateRuntimeStateCell(row);
}

void ProfileManagerWidget::onServiceAdded(const QString &serviceId)
{
    Q_UNUSED(serviceId);
    refreshRuntimeStates();
}

void ProfileManagerWidget::onServiceRemoved(const QString &serviceId)
{
    const int row = rowForServiceId(serviceId);
    if (row >= 0)
        updateRuntimeStateCell(row);
}
