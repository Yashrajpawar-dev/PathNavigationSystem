// ============================================================
// MapWidget.cpp – VIT Pune Campus Navigator
// Campus map renderer – draws nodes, edges, routes, legend
// ============================================================

#include "MapWidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QFontMetrics>
#include <QRadialGradient>
#include <QLinearGradient>
#include <cmath>
#include <algorithm>

MapWidget::MapWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(600, 400);
    setBackgroundRole(QPalette::Base);
}

void MapWidget::setGraph(const VITNav::Graph* g) {
    graph_ = g;
    recalcNodePositions();
    update();
}

void MapWidget::setRoute(const VITNav::PathResult& result) {
    route_    = result;
    hasRoute_ = result.found;
    animStep_ = -1;
    update();
}

void MapWidget::setAnimationStep(int step) {
    animStep_ = step;
    update();
}

void MapWidget::clearRoute() {
    hasRoute_ = false;
    route_    = {};
    animStep_ = -1;
    update();
}

void MapWidget::setHighlightedLocation(int id) {
    highlightedId_ = id;
    update();
}

void MapWidget::setBlockedEdgeHighlight(bool enabled) {
    blockingMode_ = enabled;
    blockFrom_    = -1;
    update();
}

QPointF MapWidget::latLonToPixel(double lat, double lon) const {
    // Map geo bounding box → widget pixels with current pan/zoom
    double normX = (lon - LON_MIN) / (LON_MAX - LON_MIN);
    double normY = 1.0 - (lat - LAT_MIN) / (LAT_MAX - LAT_MIN); // invert Y

    double baseX = normX * width();
    double baseY = normY * height();

    return QPointF(baseX * scale_ + panOffset_.x(),
                   baseY * scale_ + panOffset_.y());
}

void MapWidget::recalcNodePositions() {
    nodePosCache_.clear();
    if (!graph_) return;
    for (const auto& loc : graph_->allLocations()) {
        nodePosCache_.push_back({loc.id, latLonToPixel(loc.lat, loc.lon)});
    }
}

void MapWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    recalcNodePositions();
}

// ---- Painting ----

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    drawBackground(p);
    if (!graph_) {
        p.setPen(Qt::white);
        p.setFont(QFont("Segoe UI", 16));
        p.drawText(rect(), Qt::AlignCenter, "Loading campus data...");
        return;
    }
    drawGrid(p);
    drawEdges(p);
    if (hasRoute_) drawRoute(p);
    drawNodes(p);
    drawLegend(p);
    drawCompass(p);
    drawScale(p);
    drawCampusLabel(p);
}

void MapWidget::drawBackground(QPainter& p) {
    // Dark map-like background
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0.0, QColor(0x1a, 0x1a, 0x2e));
    bg.setColorAt(1.0, QColor(0x16, 0x21, 0x3e));
    p.fillRect(rect(), bg);

    // Faint "map tile" grid pattern
    p.setPen(QPen(QColor(255, 255, 255, 12), 0.5));
    int step = 60;
    for (int x = 0; x < width(); x += step)
        p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += step)
        p.drawLine(0, y, width(), y);
}

void MapWidget::drawGrid(QPainter& p) {
    // Campus area highlight
    QPointF tl = latLonToPixel(LAT_MAX, LON_MIN);
    QPointF br = latLonToPixel(LAT_MIN, LON_MAX);
    QRectF campusRect(tl, br);

    QLinearGradient grad(tl, br);
    grad.setColorAt(0.0, QColor(0x0f, 0x3c, 0x60, 80));
    grad.setColorAt(1.0, QColor(0x0a, 0x29, 0x4a, 60));
    p.fillRect(campusRect, grad);

    p.setPen(QPen(QColor(0x00, 0xb4, 0xd8, 100), 1.5, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(campusRect, 8, 8);
}

void MapWidget::drawEdges(QPainter& p) {
    if (!graph_) return;
    for (const auto& loc : graph_->allLocations()) {
        QPointF fromPt = latLonToPixel(loc.lat, loc.lon);
        for (const auto& [nbId, dist] : graph_->allNeighbours(loc.id)) {
            if (nbId <= loc.id) continue; // draw each edge once
            bool blocked = graph_->edgeBlocked(loc.id, nbId);
            try {
                const auto& nb = graph_->location(nbId);
                QPointF toPt = latLonToPixel(nb.lat, nb.lon);

                if (blocked) {
                    p.setPen(QPen(QColor(0xe6, 0x3b, 0x2e, 180), 2.5,
                                  Qt::DashLine));
                } else {
                    p.setPen(QPen(QColor(0x48, 0xca, 0xe4, 160), 2.0));
                }
                p.drawLine(fromPt, toPt);

                // Distance label on edge midpoint
                if (!blocked) {
                    QPointF mid((fromPt.x() + toPt.x()) / 2,
                                (fromPt.y() + toPt.y()) / 2);
                    p.setPen(QColor(0xad, 0xe8, 0xf4, 180));
                    p.setFont(QFont("Segoe UI", 7));
                    p.drawText(mid, QString::number((int)dist) + "m");
                }
            } catch (...) {}
        }
    }
}

void MapWidget::drawRoute(QPainter& p) {
    if (!hasRoute_ || route_.path.empty()) return;

    int displayCount = (animStep_ < 0) ? (int)route_.path.size() : animStep_;
    displayCount = std::min(displayCount, (int)route_.path.size());

    // Draw glowing route line
    for (int i = 0; i + 1 < displayCount; ++i) {
        try {
            const auto& a = graph_->location(route_.path[i]);
            const auto& b = graph_->location(route_.path[i + 1]);
            QPointF pa = latLonToPixel(a.lat, a.lon);
            QPointF pb = latLonToPixel(b.lat, b.lon);

            // Outer glow
            p.setPen(QPen(QColor(0xff, 0xd6, 0x00, 60), 10));
            p.drawLine(pa, pb);
            // Main route
            p.setPen(QPen(QColor(0xff, 0xd6, 0x00), 4));
            p.drawLine(pa, pb);
            // Direction arrow
            QPointF dir = pb - pa;
            double len = std::sqrt(dir.x()*dir.x() + dir.y()*dir.y());
            if (len > 20) {
                dir = dir / len;
                QPointF mid((pa.x()+pb.x())/2, (pa.y()+pb.y())/2);
                QPointF perp(-dir.y(), dir.x());
                QPolygonF arrow;
                arrow << mid + dir*8 << mid - dir*4 + perp*4
                      << mid - dir*4 - perp*4;
                p.setBrush(QColor(0xff, 0xd6, 0x00));
                p.setPen(Qt::NoPen);
                p.drawPolygon(arrow);
                p.setBrush(Qt::NoBrush);
            }
        } catch (...) {}
    }

    // Source node
    if (!route_.path.empty()) {
        try {
            const auto& src = graph_->location(route_.path.front());
            QPointF sp = latLonToPixel(src.lat, src.lon);
            p.setBrush(QColor(0x06, 0xd6, 0xa0));
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(sp, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            p.drawText(QRectF(sp.x()-14, sp.y()-14, 28, 28),
                       Qt::AlignCenter, "S");
        } catch (...) {}
    }

    // Destination node
    if (displayCount >= (int)route_.path.size() && route_.path.size() > 1) {
        try {
            const auto& dst = graph_->location(route_.path.back());
            QPointF dp = latLonToPixel(dst.lat, dst.lon);
            p.setBrush(QColor(0xef, 0x47, 0x6f));
            p.setPen(QPen(Qt::white, 2));
            p.drawEllipse(dp, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            p.drawText(QRectF(dp.x()-14, dp.y()-14, 28, 28),
                       Qt::AlignCenter, "D");
        } catch (...) {}
    }
}

void MapWidget::drawNodes(QPainter& p) {
    if (!graph_) return;
    for (const auto& np : nodePosCache_) {
        try {
            const auto& loc = graph_->location(np.id);
            QColor col = categoryColour(loc.category);
            bool isInRoute = hasRoute_ &&
                std::find(route_.path.begin(), route_.path.end(), np.id)
                             != route_.path.end();
            bool isHighlighted = (np.id == highlightedId_);

            double radius = isHighlighted ? 14.0 : (isInRoute ? 12.0 : 10.0);

            // Glow for route nodes
            if (isInRoute || isHighlighted) {
                QRadialGradient glow(np.pos, radius * 2);
                glow.setColorAt(0, QColor(col.red(), col.green(), col.blue(), 120));
                glow.setColorAt(1, QColor(col.red(), col.green(), col.blue(), 0));
                p.setBrush(glow);
                p.setPen(Qt::NoPen);
                p.drawEllipse(np.pos, radius * 2, radius * 2);
            }

            // Node circle
            p.setBrush(col);
            p.setPen(QPen(Qt::white, isHighlighted ? 2.5 : 1.5));
            p.drawEllipse(np.pos, radius, radius);

            // Icon/initial inside node
            QString icon = categoryIcon(loc.category);
            p.setPen(Qt::white);
            p.setFont(QFont("Segoe UI", 7, QFont::Bold));
            p.drawText(QRectF(np.pos.x()-radius, np.pos.y()-radius,
                              radius*2, radius*2),
                       Qt::AlignCenter, icon);

            // Name label below
            p.setPen(QColor(0xe0, 0xe0, 0xff));
            p.setFont(QFont("Segoe UI", 7));
            QString lbl = loc.shortName;
            QFontMetrics fm(p.font());
            int tw = fm.horizontalAdvance(lbl);
            // Background for label
            QRectF lblRect(np.pos.x() - tw/2.0 - 3, np.pos.y() + radius + 2,
                           tw + 6, 14);
            p.fillRect(lblRect, QColor(0, 0, 0, 140));
            p.drawText(lblRect, Qt::AlignCenter, lbl);
        } catch (...) {}
    }
}

void MapWidget::drawLegend(QPainter& p) {
    // Category legend (bottom-left)
    int x = 10, y = height() - 10;
    struct LegendEntry { VITNav::LocationCategory cat; QString label; };
    std::vector<LegendEntry> entries = {
        {VITNav::LocationCategory::Gate,           "Gate"},
        {VITNav::LocationCategory::Academic,       "Academic"},
        {VITNav::LocationCategory::Department,     "Department"},
        {VITNav::LocationCategory::Library,        "Library"},
        {VITNav::LocationCategory::Facility,       "Facility"},
        {VITNav::LocationCategory::Laboratory,     "Lab"},
        {VITNav::LocationCategory::Sports,         "Sports"},
        {VITNav::LocationCategory::Hostel,         "Hostel"},
        {VITNav::LocationCategory::Health,         "Health"},
        {VITNav::LocationCategory::Research,       "Research"},
    };

    p.setFont(QFont("Segoe UI", 7));
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        y -= 18;
        p.setBrush(categoryColour(it->cat));
        p.setPen(QPen(Qt::white, 1));
        p.drawEllipse(x, y + 3, 10, 10);
        p.setPen(QColor(0xcc, 0xcc, 0xff));
        p.drawText(x + 14, y + 13, it->label);
    }

    // Route legend
    x = 10; y -= 30;
    p.setBrush(QColor(0xff, 0xd6, 0x00));
    p.setPen(Qt::NoPen);
    p.fillRect(x, y + 5, 24, 4, QColor(0xff, 0xd6, 0x00));
    p.setPen(QColor(0xcc, 0xcc, 0xff));
    p.setFont(QFont("Segoe UI", 7));
    p.drawText(x + 28, y + 13, "Shortest Path");
    y -= 16;
    p.setPen(QPen(QColor(0xe6, 0x3b, 0x2e), 2, Qt::DashLine));
    p.drawLine(x, y + 7, x + 24, y + 7);
    p.setPen(QColor(0xcc, 0xcc, 0xff));
    p.drawText(x + 28, y + 13, "Blocked Path");
}

void MapWidget::drawCompass(QPainter& p) {
    int cx = width() - 40, cy = 40;
    p.setBrush(QColor(0, 0, 0, 140));
    p.setPen(Qt::NoPen);
    p.drawEllipse(cx - 20, cy - 20, 40, 40);
    p.setPen(QPen(Qt::white, 1));
    p.setFont(QFont("Segoe UI", 8, QFont::Bold));
    p.drawText(cx - 4, cy - 8, "N");
    p.setPen(QColor(0xef, 0x47, 0x6f));
    p.drawLine(cx, cy - 2, cx, cy - 14);
    p.setPen(Qt::white);
    p.drawLine(cx, cy + 2, cx, cy + 14);
}

void MapWidget::drawScale(QPainter& p) {
    // Draw approximate 100m scale bar
    // 100m in pixels at current scale:
    // The map width in geo degrees: LON_MAX - LON_MIN = 0.0032 deg lon
    // At lat 18.46: 1 deg lon ≈ 111320 * cos(18.46°) ≈ 105600 m
    // 0.0032 deg ≈ 338 m across the full widget width
    // So scale = width_px / 338 px/m
    double mPerPx = 338.0 / width() / scale_;
    double barPx  = 100.0 / mPerPx;

    int bx = width() - (int)barPx - 15;
    int by = height() - 20;
    p.setPen(Qt::white);
    p.setBrush(Qt::white);
    p.fillRect(bx, by, (int)barPx, 3, Qt::white);
    p.drawLine(bx, by - 3, bx, by + 3);
    p.drawLine(bx + (int)barPx, by - 3, bx + (int)barPx, by + 3);
    p.setFont(QFont("Segoe UI", 7));
    p.setPen(QColor(0xcc, 0xcc, 0xff));
    p.drawText(bx + (int)barPx / 2 - 10, by - 5, "≈ 100 m");
}

void MapWidget::drawCampusLabel(QPainter& p) {
    p.setFont(QFont("Segoe UI", 10, QFont::Bold));
    p.setPen(QColor(0xad, 0xe8, 0xf4, 180));
    p.drawText(10, 20, "VIT Pune — Bibwewadi Campus");
    p.setFont(QFont("Segoe UI", 7));
    p.setPen(QColor(0xff, 0x99, 0x00, 160));
    p.drawText(10, 36, "⚠ Building positions are APPROXIMATE – for demonstration");
}

// ---- Mouse events ----

void MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int id = locationAtPixel(event->pos());
        if (id >= 0) {
            if (blockingMode_) {
                if (blockFrom_ < 0) {
                    blockFrom_ = id;
                    setHighlightedLocation(id);
                } else {
                    emit edgeBlockRequested(blockFrom_, id);
                    blockFrom_ = -1;
                    setHighlightedLocation(-1);
                    blockingMode_ = false;
                }
            } else {
                emit locationClicked(id);
            }
        } else {
            panning_      = true;
            lastMousePos_ = event->pos();
        }
    }
    if (event->button() == Qt::RightButton) {
        panning_      = false;
        blockingMode_ = false;
        blockFrom_    = -1;
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (panning_) {
        QPointF delta = event->pos() - lastMousePos_;
        panOffset_ += delta;
        lastMousePos_ = event->pos();
        recalcNodePositions();
        update();
    } else {
        int id = locationAtPixel(event->pos());
        setHighlightedLocation(id);
    }
}

void MapWidget::wheelEvent(QWheelEvent* event) {
    double factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
    QPointF c = event->position();
    panOffset_ = c + (panOffset_ - c) * factor;
    scale_ *= factor;
    scale_ = std::max(0.4, std::min(scale_, 8.0));
    recalcNodePositions();
    update();
}

// ---- Helpers ----

int MapWidget::locationAtPixel(const QPointF& pt) const {
    for (const auto& np : nodePosCache_) {
        double dx = pt.x() - np.pos.x();
        double dy = pt.y() - np.pos.y();
        if (std::sqrt(dx*dx + dy*dy) <= 16.0) return np.id;
    }
    return -1;
}

QColor MapWidget::categoryColour(VITNav::LocationCategory cat) const {
    switch (cat) {
        case VITNav::LocationCategory::Gate:           return QColor(0xf7, 0x73, 0x06);
        case VITNav::LocationCategory::Administration: return QColor(0x7b, 0x2d, 0x8b);
        case VITNav::LocationCategory::Academic:       return QColor(0x00, 0x77, 0xb6);
        case VITNav::LocationCategory::Department:     return QColor(0x02, 0x96, 0xc7);
        case VITNav::LocationCategory::Library:        return QColor(0x48, 0xcb, 0x49);
        case VITNav::LocationCategory::Facility:       return QColor(0xff, 0x99, 0x00);
        case VITNav::LocationCategory::Laboratory:     return QColor(0x00, 0xb4, 0xd8);
        case VITNav::LocationCategory::Sports:         return QColor(0x06, 0xd6, 0xa0);
        case VITNav::LocationCategory::Parking:        return QColor(0x6c, 0x75, 0x7d);
        case VITNav::LocationCategory::Hostel:         return QColor(0xe0, 0x6c, 0x75);
        case VITNav::LocationCategory::Health:         return QColor(0xef, 0x47, 0x6f);
        case VITNav::LocationCategory::Faculty:        return QColor(0xab, 0x83, 0xa1);
        case VITNav::LocationCategory::Research:       return QColor(0xf4, 0xd0, 0x3f);
        default:                                       return QColor(0x90, 0x90, 0x90);
    }
}

QString MapWidget::categoryIcon(VITNav::LocationCategory cat) const {
    switch (cat) {
        case VITNav::LocationCategory::Gate:           return "G";
        case VITNav::LocationCategory::Administration: return "A";
        case VITNav::LocationCategory::Academic:       return "B";
        case VITNav::LocationCategory::Department:     return "D";
        case VITNav::LocationCategory::Library:        return "L";
        case VITNav::LocationCategory::Facility:       return "F";
        case VITNav::LocationCategory::Laboratory:     return "⚗";
        case VITNav::LocationCategory::Sports:         return "⚽";
        case VITNav::LocationCategory::Parking:        return "P";
        case VITNav::LocationCategory::Hostel:         return "H";
        case VITNav::LocationCategory::Health:         return "+";
        case VITNav::LocationCategory::Faculty:        return "F";
        case VITNav::LocationCategory::Research:       return "R";
        default:                                       return "?";
    }
}
