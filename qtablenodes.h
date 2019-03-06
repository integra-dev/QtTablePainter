#ifndef QTABLENODES_H
#define QTABLENODES_H

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QItemDelegate>
#include <QPainter>
#include <QPaintEvent>
#include <QColor>
#include <QMouseEvent>
#include <QDropEvent>

#include <vector>
#include <map>

#ifndef M_PI
#define M_PI 3,1415926535897932384626433832795
#endif

#define TABLE_SIZE 250
#define ARROW_WIDTH 10
#define ARROW_HEIGHT 14
#define LINE_WIDTH 2

// global nodes
static std::map<QModelIndex, bool> isAnode;


class QTableNodes : public QTableWidget
{
    Q_OBJECT

private:
    int column_width;
    int row_height;
    bool m_nbMousePressed;
    bool m_leftButtonPressed;
    QLine m_line;
    QPixmap m_nPTargetPixmap;
    QModelIndex drag_index;
    QModelIndex drop_index;

    std :: vector <std::pair<QModelIndex, QModelIndex> > edges;
    std :: map<QModelIndex, std :: vector<QModelIndex> > nodes;
    void print_edges();
    void print_isAnode();

private slots:
    void updateNode(QTableWidgetItem *it);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);

    virtual void paintEvent(QPaintEvent *event);
    bool detectEdgePoints(const QModelIndex& index_from, const QModelIndex& index_to,
                          std::pair<QPointF, QPointF>& points);
    void drawArrow(QPainter &painter, QPointF &from, QPointF &to);
public:
    explicit QTableNodes(QWidget *parent = 0);
    ~QTableNodes() { }

    void fillTable();
    void setHeaderStyle();
};


// class to change cell shapes
class DrawBoxDelegate : public QItemDelegate
{
public:
    DrawBoxDelegate( QObject *parent ) : QItemDelegate( parent ) { }
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
        QItemDelegate::paint( painter, option, index );
        if(index.isValid())
        {
            QString cellText = index.data().toString();
            if(!cellText.isEmpty())
            {
                QPen pen;
                pen.setWidth(2);
                pen.setColor(QColor("#000") );
                painter->setPen(pen);
                isAnode[index] = true;
            }
            else
            {
                QPen pen;
                pen.setWidth(1);
                pen.setColor(QColor("#d8d8d8") );
                painter->setPen(pen);
                isAnode[index] = false;
            }
            painter->drawRect( option.rect.topLeft().x(), option.rect.topLeft().y(),
                               option.rect.width(), option.rect.height());
        }
    }
};

#endif // QTABLENODES_H
