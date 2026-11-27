// ============================================================
// MainWindow.cpp – VIT Pune Campus Navigator
// Qt6 Widgets main application window
// ============================================================

#include "MainWindow.h"
#include "MapWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QSplitter>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QFontDatabase>
#include <QResizeEvent>
#include <sstream>
#include <iomanip>

static QString sectionLabel(const QString& text) {
    QString lbl = QString(
        "<span style='color:#90e0ef;font-size:10px;font-weight:bold;"
        "letter-spacing:1px;'>%1</span>").arg(text.toUpper());
    return lbl;
}

MainWindow::MainWindow(const QString& dataDir, QWidget* parent)
    : QMainWindow(parent), dataDir_(dataDir) {

    setWindowTitle("VIT Pune Campus Navigator");
    setMinimumSize(1280, 720);
    resize(1400, 820);

    // Load controller
    controller_ = std::make_unique<VITNav::NavigationController>();
    bool loaded  = controller_->loadAll(dataDir_.toStdString());

    setupUi();
    applyStyleSheet();

    if (loaded) {
        populateLocationCombos();
        mapWidget_->setGraph(&controller_->graph());
        statusBar()->showMessage(
            QString("✓ Campus loaded – %1 locations, %2 paths")
            .arg(controller_->graph().numLocations())
            .arg(controller_->graph().numEdges()));
    } else {
        statusBar()->showMessage(
            "⚠ Could not load campus data. Check data/locations.json");
    }

    // Animation timer
    animTimer_ = new QTimer(this);
    connect(animTimer_, &QTimer::timeout, this, &MainWindow::onAnimationTick);
}

void MainWindow::setupUi() {
    // ---- Menu Bar ----
    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Reset Route", this, &MainWindow::onResetRoute);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close);

    auto* toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction("&Compare Algorithms", this, &MainWindow::onCompareAlgorithms);
    toolsMenu->addAction("Clear &Blocked Paths", this, &MainWindow::onClearBlocks);

    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About", this, [this](){
        QMessageBox::about(this, "About VIT Campus Navigator",
            "<b>VIT Pune Campus Navigator</b><br>"
            "Version 1.0<br><br>"
            "DSA Project — Graph Algorithms Demo<br>"
            "Campus: Vishwakarma Institute of Technology,<br>"
            "Bibwewadi, Pune 411037<br><br>"
            "<small>Building positions are APPROXIMATE.<br>"
            "Based on OpenStreetMap data.</small>");
    });

    // ---- Central splitter ----
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(2);

    setupLeftPanel();
    setupCentralMap();
    setupRightPanel();

    splitter->addWidget(leftPanel_);
    splitter->addWidget(mapWidget_);
    splitter->addWidget(rightPanel_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);
    splitter->setSizes({280, 820, 300});

    // Header bar
    auto* headerWidget = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 8, 16, 8);

    auto* logoLabel = new QLabel("🗺", this);
    logoLabel->setFont(QFont("Segoe UI", 22));

    auto* titleLabel = new QLabel("VIT Pune Campus Navigator", this);
    titleLabel->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleLabel->setStyleSheet("color: #90e0ef;");

    auto* subtitleLabel = new QLabel("Vishwakarma Institute of Technology · Bibwewadi · Pune", this);
    subtitleLabel->setFont(QFont("Segoe UI", 9));
    subtitleLabel->setStyleSheet("color: #adb5bd;");

    auto* titleCol = new QVBoxLayout;
    titleCol->setSpacing(2);
    titleCol->addWidget(titleLabel);
    titleCol->addWidget(subtitleLabel);

    headerLayout->addWidget(logoLabel);
    headerLayout->addLayout(titleCol);
    headerLayout->addStretch();

    auto* versionLabel = new QLabel("DSA Project  |  v1.0", this);
    versionLabel->setStyleSheet("color:#6c757d;font-size:10px;");
    headerLayout->addWidget(versionLabel);
    headerWidget->setObjectName("headerWidget");
    headerWidget->setFixedHeight(60);

    // Main layout
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(splitter, 1);
    setCentralWidget(central);

    setupStatusBar();
}

void MainWindow::setupLeftPanel() {
    leftPanel_ = new QWidget(this);
    leftPanel_->setFixedWidth(290);
    leftPanel_->setObjectName("leftPanel");

    auto* layout = new QVBoxLayout(leftPanel_);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    // ---- Search ----
    auto* searchGroup = new QGroupBox("🔍 Search Locations", leftPanel_);
    auto* searchLayout = new QVBoxLayout(searchGroup);
    searchBox_ = new QLineEdit(searchGroup);
    searchBox_->setPlaceholderText("Search buildings, labs, facilities...");
    searchBox_->setClearButtonEnabled(true);
    searchResults_ = new QListWidget(searchGroup);
    searchResults_->setMaximumHeight(100);
    searchResults_->setVisible(false);
    searchLayout->addWidget(searchBox_);
    searchLayout->addWidget(searchResults_);
    connect(searchBox_, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(searchResults_, &QListWidget::itemClicked, this, &MainWindow::onSearchItemClicked);
    layout->addWidget(searchGroup);

    // ---- Route Selection ----
    auto* routeGroup = new QGroupBox("🧭 Route Selection", leftPanel_);
    auto* routeLayout = new QGridLayout(routeGroup);
    routeLayout->setSpacing(8);

    auto* srcLbl = new QLabel("From:", routeGroup);
    srcLbl->setStyleSheet("color:#90e0ef;font-weight:bold;");
    sourceCombo_ = new QComboBox(routeGroup);
    sourceCombo_->setToolTip("Select starting location");

    auto* dstLbl = new QLabel("To:", routeGroup);
    dstLbl->setStyleSheet("color:#ef476f;font-weight:bold;");
    destCombo_ = new QComboBox(routeGroup);
    destCombo_->setToolTip("Select destination");

    routeLayout->addWidget(srcLbl,     0, 0);
    routeLayout->addWidget(sourceCombo_, 0, 1);
    routeLayout->addWidget(dstLbl,     1, 0);
    routeLayout->addWidget(destCombo_, 1, 1);
    layout->addWidget(routeGroup);

    connect(sourceCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSourceChanged);
    connect(destCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onDestChanged);

    // ---- Algorithm Selection ----
    auto* algGroup = new QGroupBox("⚙ Algorithm", leftPanel_);
    auto* algLayout = new QVBoxLayout(algGroup);
    algorithmCombo_ = new QComboBox(algGroup);
    algorithmCombo_->addItem("Dijkstra's Algorithm",        0);
    algorithmCombo_->addItem("A* Search (Haversine)",       1);
    algorithmCombo_->addItem("BFS (Min Hops)",              2);
    algorithmCombo_->addItem("DFS (Traversal Demo)",        3);
    algorithmCombo_->addItem("Floyd-Warshall (All Pairs)",  4);
    algLayout->addWidget(algorithmCombo_);
    layout->addWidget(algGroup);

    connect(algorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);

    // ---- Action buttons ----
    findRouteBtn_ = new QPushButton("▶  Find Route", leftPanel_);
    findRouteBtn_->setObjectName("findRouteBtn");
    findRouteBtn_->setFixedHeight(42);

    compareBtn_ = new QPushButton("📊  Compare All Algorithms", leftPanel_);
    compareBtn_->setObjectName("compareBtn");
    compareBtn_->setFixedHeight(36);

    resetBtn_ = new QPushButton("↺  Reset", leftPanel_);
    resetBtn_->setObjectName("resetBtn");
    resetBtn_->setFixedHeight(36);

    clearBlocksBtn_ = new QPushButton("🔓  Clear Blocked Paths", leftPanel_);
    clearBlocksBtn_->setFixedHeight(36);

    layout->addWidget(findRouteBtn_);
    layout->addWidget(compareBtn_);

    auto* btnRow = new QHBoxLayout;
    btnRow->addWidget(resetBtn_);
    btnRow->addWidget(clearBlocksBtn_);
    layout->addLayout(btnRow);

    connect(findRouteBtn_, &QPushButton::clicked, this, &MainWindow::onFindRoute);
    connect(compareBtn_,   &QPushButton::clicked, this, &MainWindow::onCompareAlgorithms);
    connect(resetBtn_,     &QPushButton::clicked, this, &MainWindow::onResetRoute);
    connect(clearBlocksBtn_, &QPushButton::clicked, this, &MainWindow::onClearBlocks);

    // ---- Hint ----
    auto* hint = new QLabel(
        "<small style='color:#6c757d;'>"
        "💡 Click a node on the map to select it.<br>"
        "Right-click two nodes to block a path.</small>",
        leftPanel_);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    layout->addStretch();
}

void MainWindow::setupCentralMap() {
    mapWidget_ = new MapWidget(this);
    connect(mapWidget_, &MapWidget::locationClicked,
            this, &MainWindow::onLocationClickedOnMap);
    connect(mapWidget_, &MapWidget::edgeBlockRequested,
            this, &MainWindow::onBlockEdgeRequested);
}

void MainWindow::setupRightPanel() {
    rightPanel_ = new QWidget(this);
    rightPanel_->setFixedWidth(310);
    rightPanel_->setObjectName("rightPanel");

    auto* layout = new QVBoxLayout(rightPanel_);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    // ---- Route Summary ----
    auto* routeGroup = new QGroupBox("🗺 Route Details", rightPanel_);
    auto* routeLayout = new QVBoxLayout(routeGroup);

    routeSummaryLabel_ = new QLabel("Select source and destination\nthen click Find Route.", routeGroup);
    routeSummaryLabel_->setWordWrap(true);
    routeSummaryLabel_->setStyleSheet("color:#90e0ef;font-weight:bold;");
    routeLayout->addWidget(routeSummaryLabel_);

    pathDistLabel_  = new QLabel("Distance: —", routeGroup);
    walkTimeLabel_  = new QLabel("Est. Time: — (walking at 5 km/h)", routeGroup);
    pathDistLabel_->setStyleSheet("color:#06d6a0;");
    walkTimeLabel_->setStyleSheet("color:#ffd166;");
    routeLayout->addWidget(pathDistLabel_);
    routeLayout->addWidget(walkTimeLabel_);

    auto* stepsLbl = new QLabel("Route Steps:", routeGroup);
    stepsLbl->setStyleSheet("color:#adb5bd;font-size:10px;");
    routeLayout->addWidget(stepsLbl);
    routeStepsEdit_ = new QTextEdit(routeGroup);
    routeStepsEdit_->setReadOnly(true);
    routeStepsEdit_->setMinimumHeight(140);
    routeLayout->addWidget(routeStepsEdit_);
    layout->addWidget(routeGroup);

    // ---- Algorithm Stats ----
    auto* statsGroup = new QGroupBox("📊 Algorithm Statistics", rightPanel_);
    auto* statsLayout = new QGridLayout(statsGroup);
    statsLayout->setSpacing(6);

    auto makeStat = [&](const QString& label, QLabel*& val, const QString& colour) {
        auto* lbl = new QLabel(label + ":", statsGroup);
        lbl->setStyleSheet("color:#adb5bd;font-size:10px;");
        val = new QLabel("—", statsGroup);
        val->setStyleSheet(QString("color:%1;font-weight:bold;font-size:11px;").arg(colour));
        return lbl;
    };
    int row = 0;
    statsLayout->addWidget(makeStat("Algorithm", algNameLabel_, "#90e0ef"), row, 0);
    statsLayout->addWidget(algNameLabel_, row++, 1);
    statsLayout->addWidget(makeStat("Nodes Visited", nodesVisitedLabel_, "#ffd166"), row, 0);
    statsLayout->addWidget(nodesVisitedLabel_, row++, 1);
    statsLayout->addWidget(makeStat("Edges Explored", edgesExploredLabel_, "#ffd166"), row, 0);
    statsLayout->addWidget(edgesExploredLabel_, row++, 1);
    statsLayout->addWidget(makeStat("Exec Time", execTimeLabel_, "#06d6a0"), row, 0);
    statsLayout->addWidget(execTimeLabel_, row++, 1);
    layout->addWidget(statsGroup);

    // ---- Algorithm Description ----
    auto* descGroup = new QGroupBox("ℹ Algorithm Info", rightPanel_);
    auto* descLayout = new QVBoxLayout(descGroup);
    algDescEdit_ = new QTextEdit(descGroup);
    algDescEdit_->setReadOnly(true);
    algDescEdit_->setMinimumHeight(100);
    algDescEdit_->setPlainText(algorithmDescription("Dijkstra's Algorithm"));
    descLayout->addWidget(algDescEdit_);
    layout->addWidget(descGroup);

    layout->addStretch();
}

void MainWindow::setupStatusBar() {
    statusBar()->setStyleSheet(
        "QStatusBar { background:#0d1b2a; color:#6c757d; font-size:10px; }");
}

void MainWindow::applyStyleSheet() {
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #0d1b2a;
            color: #e0e0ff;
            font-family: "Segoe UI", Arial, sans-serif;
            font-size: 11px;
        }
        #headerWidget {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #0a1628, stop:1 #0d1f38);
            border-bottom: 1px solid #1a3a5c;
        }
        #leftPanel, #rightPanel {
            background-color: #111d2e;
            border-right: 1px solid #1a3a5c;
        }
        QGroupBox {
            border: 1px solid #1a3a5c;
            border-radius: 8px;
            margin-top: 8px;
            padding-top: 8px;
            font-weight: bold;
            color: #90e0ef;
            font-size: 11px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QComboBox {
            background-color: #0d2137;
            border: 1px solid #1a3a5c;
            border-radius: 6px;
            padding: 5px 10px;
            color: #e0e0ff;
            selection-background-color: #023e8a;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #0d2137;
            border: 1px solid #1a3a5c;
            selection-background-color: #023e8a;
            color: #e0e0ff;
        }
        QLineEdit {
            background-color: #0d2137;
            border: 1px solid #1a3a5c;
            border-radius: 6px;
            padding: 6px 10px;
            color: #e0e0ff;
        }
        QLineEdit:focus { border-color: #90e0ef; }
        QListWidget {
            background-color: #0a1628;
            border: 1px solid #1a3a5c;
            border-radius: 6px;
            color: #e0e0ff;
        }
        QListWidget::item:hover { background-color: #023e8a; }
        QListWidget::item:selected { background-color: #0077b6; }
        QTextEdit {
            background-color: #0a1628;
            border: 1px solid #1a3a5c;
            border-radius: 6px;
            color: #c0c8d8;
            font-size: 10px;
        }
        QPushButton {
            background-color: #023e8a;
            border: 1px solid #0077b6;
            border-radius: 8px;
            color: white;
            font-weight: bold;
            padding: 6px 12px;
        }
        QPushButton:hover { background-color: #0077b6; }
        QPushButton:pressed { background-color: #0096c7; }
        #findRouteBtn {
            background-color: #0096c7;
            border-color: #00b4d8;
            font-size: 13px;
        }
        #findRouteBtn:hover { background-color: #00b4d8; }
        #compareBtn {
            background-color: #7b2d8b;
            border-color: #9b51a0;
        }
        #compareBtn:hover { background-color: #9b51a0; }
        #resetBtn {
            background-color: #333d4a;
            border-color: #4a5568;
        }
        QSplitter::handle { background-color: #1a3a5c; }
        QMenuBar {
            background-color: #0a1628;
            color: #e0e0ff;
            border-bottom: 1px solid #1a3a5c;
        }
        QMenuBar::item:selected { background-color: #0077b6; border-radius: 4px; }
        QMenu {
            background-color: #0d2137;
            border: 1px solid #1a3a5c;
            color: #e0e0ff;
        }
        QMenu::item:selected { background-color: #0077b6; }
        QScrollBar:vertical {
            background: #0a1628; width: 8px; margin: 0;
        }
        QScrollBar::handle:vertical { background: #1a3a5c; border-radius: 4px; }
    )");
}

// ---- Slots ----

void MainWindow::onFindRoute() {
    if (!controller_->isLoaded()) return;

    int srcData  = sourceCombo_->currentData().toInt();
    int dstData  = destCombo_->currentData().toInt();
    int algData  = algorithmCombo_->currentData().toInt();

    if (srcData == dstData) {
        QMessageBox::warning(this, "Invalid Route",
            "Source and destination cannot be the same location.");
        return;
    }

    VITNav::Algorithm alg;
    switch (algData) {
        case 0: alg = VITNav::Algorithm::Dijkstra;      break;
        case 1: alg = VITNav::Algorithm::AStar;         break;
        case 2: alg = VITNav::Algorithm::BFS;           break;
        case 3: alg = VITNav::Algorithm::DFS;           break;
        case 4: alg = VITNav::Algorithm::FloydWarshall; break;
        default: alg = VITNav::Algorithm::Dijkstra;
    }

    statusBar()->showMessage("Computing route…");
    currentResult_ = controller_->findRoute(srcData, dstData, alg);

    if (currentResult_.found) {
        displayPathResult(currentResult_);
        mapWidget_->setRoute(currentResult_);
        startRouteAnimation(currentResult_);
        statusBar()->showMessage(QString("✓ Route found: %1 → %2 (%3)")
            .arg(QString::fromStdString(controller_->graph().location(srcData).shortName))
            .arg(QString::fromStdString(controller_->graph().location(dstData).shortName))
            .arg(formatDistance(currentResult_.totalDistance)));
    } else {
        showNoPathMessage();
        mapWidget_->clearRoute();
        statusBar()->showMessage("⚠ No path found between selected locations.");
    }
}

void MainWindow::onResetRoute() {
    mapWidget_->clearRoute();
    routeSummaryLabel_->setText("Select source and destination\nthen click Find Route.");
    pathDistLabel_->setText("Distance: —");
    walkTimeLabel_->setText("Est. Time: — (walking at 5 km/h)");
    routeStepsEdit_->clear();
    algNameLabel_->setText("—");
    nodesVisitedLabel_->setText("—");
    edgesExploredLabel_->setText("—");
    execTimeLabel_->setText("—");
    stopAnimation();
    currentResult_ = {};
    statusBar()->showMessage("Route cleared.");
}

void MainWindow::onAlgorithmChanged(int) {
    int algData = algorithmCombo_->currentData().toInt();
    QString name;
    switch (algData) {
        case 0: name = "Dijkstra's Algorithm"; break;
        case 1: name = "A* Search";            break;
        case 2: name = "BFS";                  break;
        case 3: name = "DFS";                  break;
        case 4: name = "Floyd-Warshall";       break;
        default: name = "Dijkstra's Algorithm";
    }
    algDescEdit_->setPlainText(algorithmDescription(name));
}

void MainWindow::onSourceChanged(int) {}
void MainWindow::onDestChanged(int)   {}

void MainWindow::onSearchTextChanged(const QString& text) {
    searchResults_->clear();
    if (text.length() < 2) { searchResults_->setVisible(false); return; }
    auto results = controller_->searchLocations(text.toStdString());
    if (results.empty()) { searchResults_->setVisible(false); return; }
    for (const auto* loc : results) {
        auto* item = new QListWidgetItem(
            QString::fromStdString(loc->name));
        item->setData(Qt::UserRole, loc->id);
        searchResults_->addItem(item);
    }
    searchResults_->setVisible(true);
}

void MainWindow::onSearchItemClicked(QListWidgetItem* item) {
    int id = item->data(Qt::UserRole).toInt();
    // Set as source if no source selected, else as dest
    if (sourceCombo_->currentData().toInt() == 0 &&
        destCombo_->currentData().toInt() == 0) {
        for (int i = 0; i < sourceCombo_->count(); ++i) {
            if (sourceCombo_->itemData(i).toInt() == id) {
                sourceCombo_->setCurrentIndex(i); break;
            }
        }
    } else {
        for (int i = 0; i < destCombo_->count(); ++i) {
            if (destCombo_->itemData(i).toInt() == id) {
                destCombo_->setCurrentIndex(i); break;
            }
        }
    }
    searchResults_->setVisible(false);
    searchBox_->clear();
    mapWidget_->setHighlightedLocation(id);
}

void MainWindow::onLocationClickedOnMap(int locationId) {
    // First click → source; second click → dest (if source already set)
    int src = sourceCombo_->currentData().toInt();
    if (src <= 0) {
        for (int i = 0; i < sourceCombo_->count(); ++i) {
            if (sourceCombo_->itemData(i).toInt() == locationId) {
                sourceCombo_->setCurrentIndex(i); break;
            }
        }
    } else {
        for (int i = 0; i < destCombo_->count(); ++i) {
            if (destCombo_->itemData(i).toInt() == locationId) {
                destCombo_->setCurrentIndex(i); break;
            }
        }
    }
    mapWidget_->setHighlightedLocation(locationId);
    // Show info in status bar
    try {
        const auto& loc = controller_->graph().location(locationId);
        statusBar()->showMessage(
            QString("📍 %1 | %2 | %3")
            .arg(QString::fromStdString(loc.name))
            .arg(QString::fromStdString(loc.categoryString()))
            .arg(QString::fromStdString(loc.accuracyString())));
    } catch (...) {}
}

void MainWindow::onBlockEdgeRequested(int fromId, int toId) {
    if (!controller_->graph().edgeExists(fromId, toId)) {
        statusBar()->showMessage("⚠ No direct path between those locations.");
        return;
    }
    controller_->blockEdge(fromId, toId);
    mapWidget_->setGraph(&controller_->graph());
    mapWidget_->clearRoute();
    try {
        const auto& a = controller_->graph().location(fromId);
        const auto& b = controller_->graph().location(toId);
        statusBar()->showMessage(
            QString("🚧 Blocked path: %1 ↔ %2")
            .arg(QString::fromStdString(a.shortName))
            .arg(QString::fromStdString(b.shortName)));
    } catch (...) {}
}

void MainWindow::onCompareAlgorithms() {
    int srcData = sourceCombo_->currentData().toInt();
    int dstData = destCombo_->currentData().toInt();
    if (srcData == dstData || srcData <= 0) {
        QMessageBox::information(this, "Compare Algorithms",
            "Please select a source and destination first.");
        return;
    }
    auto cr = controller_->compareAlgorithms(srcData, dstData);
    displayComparisonResult(cr);
}

void MainWindow::onClearBlocks() {
    controller_->clearAllBlocks();
    mapWidget_->setGraph(&controller_->graph());
    statusBar()->showMessage("✓ All blocked paths cleared.");
}

void MainWindow::onAnimationTick() {
    ++animStep_;
    int total = currentResult_.path.empty() ? 0 : (int)currentResult_.path.size();
    if (animStep_ > total) {
        mapWidget_->setAnimationStep(-1);
        stopAnimation();
        return;
    }
    mapWidget_->setAnimationStep(animStep_);
}

// ---- Display helpers ----

void MainWindow::displayPathResult(const VITNav::PathResult& result) {
    updateAlgorithmStats(result);

    // Route summary
    if (!result.path.empty()) {
        const auto& src = controller_->graph().location(result.path.front());
        const auto& dst = controller_->graph().location(result.path.back());
        routeSummaryLabel_->setText(
            QString("🟢 %1\n🔴 %2")
            .arg(QString::fromStdString(src.name))
            .arg(QString::fromStdString(dst.name)));
    }

    pathDistLabel_->setText("Distance: " + formatDistance(result.totalDistance));
    walkTimeLabel_->setText("Est. Time: " + formatTime(result.totalTimeMin) +
                             " (walking 5 km/h)");

    // Route steps
    std::ostringstream ss;
    for (int i = 0; i < (int)result.path.size(); ++i) {
        try {
            const auto& loc = controller_->graph().location(result.path[i]);
            if (i == 0) ss << "🚶 START  ";
            else if (i == (int)result.path.size() - 1) ss << "📍 END    ";
            else ss << "  " << i << ".  ";
            ss << loc.name;
            if (i + 1 < (int)result.path.size()) {
                try {
                    auto edge = controller_->graph().getEdge(
                        result.path[i], result.path[i+1]);
                    if (edge) ss << "\n         ↓ " << (int)edge->distanceM << " m";
                } catch (...) {}
                ss << "\n";
            }
        } catch (...) {}
    }
    routeStepsEdit_->setPlainText(QString::fromStdString(ss.str()));
}

void MainWindow::displayComparisonResult(
    const VITNav::NavigationController::ComparisonResult& cr) {
    std::ostringstream ss;
    ss << "=== ALGORITHM COMPARISON ===\n\n";
    auto printRes = [&](const VITNav::PathResult& r) {
        ss << "• " << r.algorithmName << "\n";
        if (r.found) {
            ss << "  Distance:    " << std::fixed << std::setprecision(0)
               << r.totalDistance << " m\n";
            ss << "  Nodes:       " << r.nodesVisited << "\n";
            ss << "  Edges:       " << r.edgesExplored << "\n";
            ss << "  Time:        " << std::setprecision(3) << r.executionTimeMs << " ms\n";
            ss << "  Path hops:   " << (int)r.path.size() - 1 << "\n";
        } else {
            ss << "  → No path found\n";
        }
        ss << "\n";
    };
    printRes(cr.bfsResult);
    printRes(cr.dfsResult);
    printRes(cr.dijkstraResult);
    printRes(cr.aStarResult);

    routeStepsEdit_->setPlainText(QString::fromStdString(ss.str()));
    routeSummaryLabel_->setText("📊 Algorithm Comparison");

    // Display Dijkstra result on map (best shortest-path)
    if (cr.dijkstraResult.found) {
        mapWidget_->setRoute(cr.dijkstraResult);
        currentResult_ = cr.dijkstraResult;
        updateAlgorithmStats(cr.dijkstraResult);
        pathDistLabel_->setText("Distance: " + formatDistance(cr.dijkstraResult.totalDistance));
        walkTimeLabel_->setText("Est. Time: " + formatTime(cr.dijkstraResult.totalTimeMin));
    }
}

void MainWindow::updateAlgorithmStats(const VITNav::PathResult& result) {
    algNameLabel_->setText(QString::fromStdString(result.algorithmName));
    nodesVisitedLabel_->setText(QString::number(result.nodesVisited));
    edgesExploredLabel_->setText(QString::number(result.edgesExplored));
    execTimeLabel_->setText(QString::number(result.executionTimeMs, 'f', 3) + " ms");
}

void MainWindow::showNoPathMessage() {
    routeSummaryLabel_->setText("⚠ No path found.\nCheck for blocked paths.");
    pathDistLabel_->setText("Distance: N/A");
    walkTimeLabel_->setText("Est. Time: N/A");
    routeStepsEdit_->setPlainText("No route available between selected locations.\n"
        "Some paths may be blocked. Use 'Clear Blocked Paths' and try again.");
}

void MainWindow::startRouteAnimation(const VITNav::PathResult& result) {
    stopAnimation();
    animStep_ = 0;
    animTimer_->start(200); // 200ms per node
}

void MainWindow::stopAnimation() {
    if (animTimer_) animTimer_->stop();
    animStep_ = 0;
}

void MainWindow::populateLocationCombos() {
    sourceCombo_->clear();
    destCombo_->clear();
    sourceCombo_->addItem("— Select Starting Point —", 0);
    destCombo_->addItem("— Select Destination —", 0);
    for (const auto& loc : controller_->graph().allLocations()) {
        QString name = QString::fromStdString(loc.shortName);
        sourceCombo_->addItem(name, loc.id);
        destCombo_->addItem(name, loc.id);
    }
    // Set sensible defaults: Main Gate → Library
    sourceCombo_->setCurrentIndex(1); // Main Gate
    destCombo_->setCurrentIndex(9);   // Library
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
}

QString MainWindow::formatDistance(double metres) const {
    if (metres >= 1000.0)
        return QString::number(metres / 1000.0, 'f', 2) + " km";
    return QString::number((int)metres) + " m";
}

QString MainWindow::formatTime(double minutes) const {
    if (minutes < 1.0)
        return QString::number((int)(minutes * 60)) + " sec";
    int mins = (int)minutes;
    int secs = (int)((minutes - mins) * 60);
    if (secs > 0)
        return QString("%1 min %2 sec").arg(mins).arg(secs);
    return QString("%1 min").arg(mins);
}

QString MainWindow::algorithmDescription(const QString& algName) const {
    if (algName.contains("Dijkstra"))
        return "Dijkstra's Algorithm\n\nGuaranteed shortest weighted path.\n"
               "Uses a min-heap priority queue.\n\n"
               "Time:  O((V+E) log V)\nSpace: O(V)\n\n"
               "Best for: Minimum walking distance routes.";
    if (algName.contains("A*"))
        return "A* Search\n\nInformed shortest path using Haversine heuristic.\n"
               "Explores fewer nodes than Dijkstra by estimating remaining distance.\n\n"
               "Time:  O((V+E) log V)\nSpace: O(V)\n\n"
               "Best for: Faster route finding using geography.";
    if (algName.contains("BFS"))
        return "Breadth-First Search\n\nMinimum-hop path (ignores edge weights).\n"
               "Visits all nodes level by level.\n\n"
               "Time:  O(V+E)\nSpace: O(V)\n\n"
               "Best for: Fewest intermediate stops.\n"
               "⚠ Does NOT guarantee minimum distance!";
    if (algName.contains("DFS"))
        return "Depth-First Search\n\nGraph traversal algorithm.\n"
               "Explores deeply before backtracking.\n\n"
               "Time:  O(V+E)\nSpace: O(V)\n\n"
               "⚠ Does NOT guarantee shortest path!\n"
               "Use for reachability analysis only.";
    if (algName.contains("Floyd"))
        return "Floyd-Warshall\n\nAll-pairs shortest paths.\n"
               "Computes shortest routes between ALL location pairs.\n\n"
               "Time:  O(V³)\nSpace: O(V²)\n\n"
               "Best for: Pre-computing full routing tables.\n"
               "Slow for large graphs.";
    return "";
}
