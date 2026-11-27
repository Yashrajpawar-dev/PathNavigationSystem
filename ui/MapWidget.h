#pragma once
// ============================================================
// MapWidget.h – VIT Pune Campus Navigator
// Custom QWidget that renders the campus map
// ============================================================

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPointF>
#include <QTimer>
#include <vector>
#include "Graph.h"
#include "Algorithms.h"

class MapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapWidget(QWidget* parent = nullptr);

    void setGraph(const VITNav::Graph* graph);
    void setRoute(const VITNav::PathResult& result);
    void setAnimationStep(int step);  // how many route nodes to colour
    void clearRoute();
    void setHighlightedLocation(int id);
    void setBlockedEdgeHighlight(bool enabled);

    // Map transformation helpers
    QPointF latLonToPixel(double lat, double lon) const;

    QSize sizeHint() const override { return QSize(900, 650); }

signals:
    void locationClicked(int locationId);
    void edgeBlockRequested(int fromId, int toId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void drawBackground(QPainter& p);
    void drawGrid(QPainter& p);
    void drawEdges(QPainter& p);
    void drawRoute(QPainter& p);
    void drawNodes(QPainter& p);
    void drawLegend(QPainter& p);
    void drawCompass(QPainter& p);
    void drawScale(QPainter& p);
    void drawCampusLabel(QPainter& p);

    QColor categoryColour(VITNav::LocationCategory cat) const;
    QString categoryIcon(VITNav::LocationCategory cat) const;

    int locationAtPixel(const QPointF& pt) const;
    void recalcNodePositions();

    // ---- State ----
    const VITNav::Graph* graph_{nullptr};
    VITNav::PathResult   route_;
    bool  hasRoute_{false};
    int   animStep_{-1};          // -1 = show full route
    int   highlightedId_{-1};
    bool  blockingMode_{false};
    int   blockFrom_{-1};

    // Pan/zoom state
    double  scale_{1.0};
    QPointF panOffset_{0, 0};
    QPointF lastMousePos_;
    bool    panning_{false};

    // Map bounding box (in geo coordinates)
    static constexpr double LAT_MIN  = 18.4625;
    static constexpr double LAT_MAX  = 18.4670;
    static constexpr double LON_MIN  = 73.8668;
    static constexpr double LON_MAX  = 73.8700;

    // Node pixel positions (computed from lat/lon)
    struct NodePos {
        int    id;
        QPointF pos;  // widget pixel coordinates
    };
    std::vector<NodePos> nodePosCache_;
};

#endif // MAPWIDGET_H
