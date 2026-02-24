#ifndef QGRAPHICSVIEWZOOMER_H
#define QGRAPHICSVIEWZOOMER_H

#include <QGraphicsView>
#include <QWidget>
#include <QGesture>
#include <QGestureEvent>
#ifdef Q_OS_IOS
#include <QApplication>
#endif

class QGraphicsViewZoomer : public QObject
{
    Q_OBJECT
public:
    QGraphicsViewZoomer(QGraphicsView* parent, double zoom = 1) : QObject(parent) {
        parent->installEventFilter(this);
        parent->setMouseTracking(true);
        parent->grabGesture(Qt::PinchGesture);
#ifdef Q_OS_IOS
        parent->grabGesture(Qt::TapAndHoldGesture);
#else
        parent->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        parent->setResizeAnchor(QGraphicsView::AnchorViewCenter);
#endif
        setZoom(zoom);
    }
    void setZoom(const double Zoom) {
        zoomfactor = Zoom;
        if (!m_NoMatrix)
        {
            QGraphicsView* w = static_cast<QGraphicsView*>(parent());
#ifndef Q_OS_IOS
            w->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
            w->setResizeAnchor(QGraphicsView::AnchorViewCenter);
#endif
            QTransform matrix = w->transform();
            matrix.reset();
            matrix.scale(zoomfactor,zoomfactor);
            w->setTransform(matrix);
        }
    }
    double getZoom() const {
        return zoomfactor;
    }
    void disableMatrix() {
        m_NoMatrix = true;
    }
    void setMax(double m) {
        m_Max = m;
    }
    void setMin(double m) {
        m_Min = m;
    }
    double max() const {
        return m_Max;
    }
    double min() const {
        return m_Min;
    }
    void scrollXTo(qreal x) {
        QGraphicsView* w = static_cast<QGraphicsView*>(parent());
        qreal y = visibleRect().top();
        w->centerOn(QRectF(w->viewport()->rect()).center() + QPointF(x,y));
    }
    void scrollYTo(qreal y) {
        QGraphicsView* w = static_cast<QGraphicsView*>(parent());
        qreal x = visibleRect().left();
        w->centerOn(QRectF(w->viewport()->rect()).center() + QPointF(x,y));
    }
    qreal maxScrollX() {
        QGraphicsView* w = static_cast<QGraphicsView*>(parent());
        return std::max<qreal>(0,w->sceneRect().width() - w->viewport()->width());
    }
    qreal scrollValueX() {
        return visibleRect().left();
    }
    qreal scrollValueY() {
        return visibleRect().top();
    }
    qreal maxScrollY() {
        QGraphicsView* w = static_cast<QGraphicsView*>(parent());
        return std::max<qreal>(0,w->sceneRect().height() - w->viewport()->height());
    }
    QRectF visibleRect() {
        QGraphicsView* w = static_cast<QGraphicsView*>(parent());
        return w->mapToScene(w->viewport()->rect()).boundingRect();
    }
protected:
    bool eventFilter(QObject * obj, QEvent *event)
    {
        if (event->type() == QEvent::Gesture) return gestureEvent(dynamic_cast<QGestureEvent*>(event));
        return QObject::eventFilter(obj, event);
    }
    bool gestureEvent(QGestureEvent *event)
    {
        if (QGesture *swipe = event->gesture(Qt::SwipeGesture))
        {
            swipeTriggered(dynamic_cast<QSwipeGesture *>(swipe));
        }
        if (QGesture *pan = event->gesture(Qt::PanGesture))
        {
            panTriggered(dynamic_cast<QPanGesture *>(pan));
        }
        if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        {
            pinchTriggered(dynamic_cast<QPinchGesture *>(pinch));
        }
#ifdef Q_OS_IOS
        if (QGesture *longpress = event->gesture(Qt::TapAndHoldGesture))
        {
            qDebug() << "gestureEvent longpress";
            longpressTriggered(dynamic_cast<QTapAndHoldGesture *>(longpress));
        }
#endif
        return true;
    }

    void panTriggered(QPanGesture *gesture)
    {
        Q_UNUSED(gesture);
    }
    void swipeTriggered(QSwipeGesture *gesture)
    {
        Q_UNUSED(gesture);
    }
    void pinchTriggered(QPinchGesture *gesture)
    {
        double oldZoom = zoomfactor;
        if (gesture->state() & Qt::GestureStarted) gesture->setTotalScaleFactor(zoomfactor);
        if (gesture->totalScaleFactor()>m_Max) gesture->setTotalScaleFactor(m_Max);
        if (gesture->totalScaleFactor()<m_Min) gesture->setTotalScaleFactor(m_Min);
        zoomfactor = gesture->totalScaleFactor();
        if (!m_NoMatrix)
        {
            QGraphicsView* w = static_cast<QGraphicsView*>(parent());
#ifdef Q_OS_IOS
            QPointF viewPos = w->mapFromGlobal(gesture->centerPoint());
            QPointF scenePos = w->mapToScene(viewPos.toPoint());
            QTransform matrix = w->transform();
            matrix.reset();
            matrix.translate(scenePos.x(), scenePos.y());
            matrix.scale(zoomfactor,zoomfactor);
            matrix.translate(-scenePos.x(), -scenePos.y());
#else
            w->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            w->setResizeAnchor(QGraphicsView::AnchorViewCenter);
            QTransform matrix = w->transform();
            matrix.reset();
            matrix.scale(zoomfactor,zoomfactor);
#endif
            w->setTransform(matrix);
        }
        emit ZoomChanged(zoomfactor, oldZoom);
    }
#ifdef Q_OS_IOS
    void longpressTriggered(QTapAndHoldGesture *gesture) {
        qDebug() << "longpressTriggered";
        if (gesture->position().toPoint() == QPoint(0,0)) return;
        QPoint pos = ((QGraphicsView*)parent())->mapFromGlobal(gesture->position().toPoint());
        // Skapa ett mousePressEvent för RightButton
        QMouseEvent* pressEvent = new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
        QMouseEvent* releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);

        qDebug() << pressEvent->pos();
        // Skicka händelserna till widgeten
        QApplication::postEvent(((QGraphicsView*)parent())->viewport(), pressEvent);
        QApplication::postEvent(((QGraphicsView*)parent())->viewport(), releaseEvent);
    }
#endif
private:
    double zoomfactor = 1;
    bool m_NoMatrix = false;
    double m_Max = 4;
    double m_Min = 0.1;
signals:
    void ZoomChanged(double zoom, double oldZoom);
};

#endif // QGRAPHICSVIEWZOOMER_H
