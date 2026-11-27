#pragma once
// ============================================================
// MainWindow.h – VIT Pune Campus Navigator
// Main application window – Qt6 Widgets
// ============================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QString>
#include <memory>
#include "NavigationController.h"

class MapWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString& dataDir, QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onFindRoute();
    void onResetRoute();
    void onAlgorithmChanged(int index);
    void onSourceChanged(int index);
    void onDestChanged(int index);
    void onSearchTextChanged(const QString& text);
    void onSearchItemClicked(QListWidgetItem* item);
    void onLocationClickedOnMap(int locationId);
    void onBlockEdgeRequested(int fromId, int toId);
    void onCompareAlgorithms();
    void onAnimationTick();
    void onClearBlocks();

private:
    void setupUi();
    void setupLeftPanel();
    void setupCentralMap();
    void setupRightPanel();
    void setupStatusBar();
    void applyStyleSheet();

    void populateLocationCombos();
    void displayPathResult(const VITNav::PathResult& result);
    void displayComparisonResult(const VITNav::NavigationController::ComparisonResult& cr);
    void updateAlgorithmStats(const VITNav::PathResult& result);
    void showNoPathMessage();
    void startRouteAnimation(const VITNav::PathResult& result);
    void stopAnimation();

    QString formatDistance(double metres) const;
    QString formatTime(double minutes) const;
    QString algorithmDescription(const QString& algName) const;

    // ---- Data ----
    std::unique_ptr<VITNav::NavigationController> controller_;
    VITNav::PathResult currentResult_;
    QString dataDir_;

    // ---- Left panel widgets ----
    QWidget*     leftPanel_{nullptr};
    QLineEdit*   searchBox_{nullptr};
    QListWidget* searchResults_{nullptr};
    QComboBox*   sourceCombo_{nullptr};
    QComboBox*   destCombo_{nullptr};
    QComboBox*   algorithmCombo_{nullptr};
    QPushButton* findRouteBtn_{nullptr};
    QPushButton* compareBtn_{nullptr};
    QPushButton* resetBtn_{nullptr};
    QPushButton* clearBlocksBtn_{nullptr};

    // ---- Central map ----
    MapWidget*   mapWidget_{nullptr};

    // ---- Right panel widgets ----
    QWidget*     rightPanel_{nullptr};
    QLabel*      routeSummaryLabel_{nullptr};
    QTextEdit*   routeStepsEdit_{nullptr};
    QLabel*      algNameLabel_{nullptr};
    QLabel*      nodesVisitedLabel_{nullptr};
    QLabel*      edgesExploredLabel_{nullptr};
    QLabel*      execTimeLabel_{nullptr};
    QLabel*      pathDistLabel_{nullptr};
    QLabel*      walkTimeLabel_{nullptr};
    QTextEdit*   algDescEdit_{nullptr};

    // ---- Animation ----
    QTimer*      animTimer_{nullptr};
    int          animStep_{0};

    // ---- Blocking mode ----
    bool         blockingMode_{false};
    int          blockFrom_{-1};
};

#endif // MAINWINDOW_H
