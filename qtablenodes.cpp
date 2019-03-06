#include "qtablenodes.h"
#include <QBrush>
#include <QPen>

#include <QDebug>

QTableNodes::QTableNodes(QWidget* parent) :
    QTableWidget(parent)
{
    column_width = 100;
    row_height = 35;
    m_nbMousePressed = false;
    m_leftButtonPressed = false;

    connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(updateNode(QTableWidgetItem*)));
    setMouseTracking(true);
}


void QTableNodes::fillTable()
{
    for(int i = 0; i < TABLE_SIZE; i++)
    {
        int row = rowCount();
        insertRow(row);
        setRowHeight(row, row_height);

        int col = columnCount();
        insertColumn(col);
        setColumnWidth(col, column_width);
        update();
        isAnode[indexFromItem(item(row,col))] = false;
    }

    // changing style for horizontal and vertical headers
    setHeaderStyle();

    // setting grid lines width
    setItemDelegate( new DrawBoxDelegate(this) );
}


void QTableNodes::setHeaderStyle()
{
    setStyleSheet(
        "QHeaderView::section{"
            "border-top:0px solid #b7b7b7;"
            "border-left:0px solid #b7b7b7;"
            "border-right:1px solid #b7b7b7;"
            "border-bottom: 1px solid #b7b7b7;"
            "background-color: #d8d8d8;"
            "padding:4px;"
            "text-align:center;"
        "}"
        "QTableCornerButton::section{"
            "border-top:1px solid #b7b7b7;"
            "border-left:1px solid #b7b7b7;"
            "border-right:1px solid #b7b7b7;"
            "border-bottom: 1px solid #b7b7b7;"
            "background-color: #d8d8d8;"
        "}"
        );
}


void QTableNodes::updateNode(QTableWidgetItem* it)
{
    it->setTextAlignment(Qt::AlignCenter);
    setItemDelegate( new DrawBoxDelegate(this) );   // update all cell items

    isAnode[indexFromItem(it)] = it->text().isEmpty() ? false : true;
    for(auto const& isNd : isAnode)
    {
        for(size_t i = 0; i < edges.size(); i++)
        {
            if(edges[i].first == edges[i].second && isNd.second)    // remove loops (edge=point)
            {
                edges.erase(edges.begin() + i);
            }
            else if(edges[i].first == isNd.first && (!isAnode[edges[i].first] || !isAnode[edges[i].second]) )
            {
                edges.erase(edges.begin() + i);
            }
            else if(edges[i].second == isNd.first && (!isAnode[edges[i].first] || !isAnode[edges[i].second]) )
            {
                edges.erase(edges.begin() + i);
            }
        }
    }
}


void QTableNodes::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QTableWidget::mousePressEvent(event);
        m_leftButtonPressed = true;
    }
    if(event->button() == Qt::RightButton && !m_nbMousePressed)
    {
        m_nbMousePressed = true;
        viewport()->setFocus();
        m_line.setP1(event->pos());

        QTableWidgetItem* it = itemAt(m_line.p1());
        if(it == nullptr)  return;
        if(!it->text().isEmpty())
        {
            edges.push_back(std::make_pair(indexAt(m_line.p1()),  // initially line is a point
                                           indexAt(m_line.p1())
                                           )
                            );
        }
    }
}


void QTableNodes::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() && Qt::LeftButton)
    {
        if(!m_nbMousePressed)
            QTableWidget::mouseMoveEvent(event);
    }
    if(event->buttons() && Qt::RightButton)
    {
        if(m_nbMousePressed)
        {
            m_line.setP2(event->pos());
            if(itemAt(m_line.p2()) == nullptr)  return;
            if(!edges.empty() && itemAt(m_line.p2())->text().length() > 0)
            {
                edges.back().second = indexAt(m_line.p2());     // updating second vertex
            }
        }
    }
    setItemDelegate( new DrawBoxDelegate(this) );
}


void QTableNodes::mouseReleaseEvent(QMouseEvent *event)
{
    QTableWidget::mouseReleaseEvent(event);

    if(!m_leftButtonPressed)
    {
        m_line.setP2(event->pos());
        if (event->type() == QEvent::MouseButtonRelease && event->buttons() && Qt::RightButton)
        {
            edges.back().second = this->indexAt(m_line.p2());
        }
    }
    m_nbMousePressed = false;
    m_leftButtonPressed = false;

    // removing duplicates
    std::sort(edges.begin(), edges.end() );
    const auto& it = std::unique (edges.begin(), edges.end());   // unique_edge_1 ... unique_edge_n   ?  ?
                                                                 //                                   ^
    edges.resize( std::distance(edges.begin(),it) );
    setItemDelegate( new DrawBoxDelegate(this) );
}


void QTableNodes::dragEnterEvent(QDragEnterEvent *event)
{
    drag_index = indexAt(event->pos());
    event->acceptProposedAction();
}


void QTableNodes::dropEvent(QDropEvent *event)
{
    QTableWidget::dropEvent(event);
    drop_index = indexAt(event->pos());

    // if key exists in isAnode then update nodes
    if ( isAnode.find(drop_index) != isAnode.end() )
    {
        isAnode[drag_index] = false;
        isAnode[drop_index] = true;
    }

    if(m_leftButtonPressed)
    {
        // trying to update edges
        QModelIndex new_drop_index = drop_index;    //indexAt(event->pos());      // current index to update arrows

        for(size_t i = 0; i < edges.size(); i++)      // tracking all items to update edges
        {
            if(edges[i].first == drag_index)
            {
                edges[i].first = new_drop_index;
            }
            else if(edges[i].second == drag_index)
            {
                edges[i].second = new_drop_index;
            }
        }

        // removing not actual edges
        for(auto const& isNd : isAnode)
        {
            for(size_t i = 0; i < edges.size(); i++)
            {
                if(edges[i].first == edges[i].second && isNd.second)    // remove loops (edge=point)
                {
                    edges.erase(edges.begin() + i);
                }
                else if(edges[i].first == isNd.first && (!isAnode[edges[i].first] || !isAnode[edges[i].second]))
                {
                    edges.erase(edges.begin() + i);
                }
            }
        }
    }
}


void QTableNodes::drawArrow(QPainter& painter, QPointF& from, QPointF& to)
{
    painter.drawLine(from, to);  // drawing line

    // drawing arrow head
    QPolygon arrow;

    painter.setBrush(QBrush(QColor(0, 0, 255, 255)));   // comment to disable triangle filling

    arrow << QPoint(0, - ARROW_WIDTH/2)
          << QPoint(ARROW_HEIGHT, 0)
          << QPoint(0, ARROW_WIDTH/2);

    double dx = to.x() - from.x();
    double dy = to.y() - from.y();
    double angle = atan2(dy, dx);

    QPointF tmp_point(to.x() - ARROW_HEIGHT * cos(angle),
                      to.y() - ARROW_HEIGHT * sin(angle));
    double lineLen = sqrt(dx*dx + dy*dy);

    if(lineLen >= ARROW_HEIGHT){
        painter.drawLine(from, tmp_point);
    }
    painter.save();
    painter.translate(tmp_point);
    painter.rotate(180 / M_PI * angle);
    painter.drawConvexPolygon(arrow);
    painter.restore();
}


void QTableNodes::paintEvent(QPaintEvent *event)
{
    QTableWidget::paintEvent(event);
    QPainter painter(this->viewport());
    painter.setPen(QPen(QBrush(QColor(0, 0, 255, 255)), LINE_WIDTH));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    for(auto i : edges)
    {
        std :: pair<QPointF, QPointF> points;
        bool points_exist = detectEdgePoints(i.first, i.second, points);
        if(i.first == i.second || !points_exist)
            continue;

        QPointF from = points.first, to = points.second;
        drawArrow(painter, from, to);
    }
}


// debug purposes only
void QTableNodes:: print_edges()
{
    qDebug() << "edges:";
     for(auto e : edges)
     {
         qDebug() << e.first << " -> " << e.second;
     }
}


// debug purposes only
void QTableNodes:: print_isAnode()
{
    qDebug() << "isAnode:";
     for(const auto &i : isAnode)
     {
         qDebug() << i.first << " -> " << i.second;
     }
}


bool QTableNodes::detectEdgePoints(const QModelIndex& index_from, const QModelIndex& index_to,
                                   std :: pair<QPointF, QPointF>& points)
{
    QTableWidgetItem* item_from = itemFromIndex(index_from);
    QTableWidgetItem* item_to = itemFromIndex(index_to);

    if(!item_from || !item_to)
        return false;

    int row_1 = item_from->row(), col_1 = item_from->column(),
        row_2 = item_to->row(), col_2 = item_to->column();

    int width_1 = visualRect(index_from).width(),
        height_1 = visualRect(index_from).height(),
        width_2 = visualRect(index_to).width(),
        height_2 = visualRect(index_to).height();

    // direct looking of all 8 possible  positions

    // left -> bottom
    if( (row_1 > row_2) && (col_1 > col_2) )
    {
        points.first = visualRect(index_from).center() - QPointF(width_1 / 2, 0);
        points.second = visualRect(index_to).center() + QPointF(0, height_2 / 2);
    }
    // right -> bottom
    else if( (row_1 > row_2) && (col_1 < col_2) )
    {
        points.first = visualRect(index_from).center() + QPointF(width_1 / 2, 0);
        points.second = visualRect(index_to).center() + QPointF(0, height_2 / 2);
    }
    // left -> top
    else if( (row_1 < row_2) && (col_1 > col_2) )
    {
        points.first = visualRect(index_from).center() - QPointF(width_1 / 2, 0);
        points.second = visualRect(index_to).center() - QPointF(0, height_2 / 2);
    }
    //right -> top
    else if( (row_1 < row_2) && (col_1 < col_2) )
    {
        points.first = visualRect(index_from).center() + QPointF(width_1 / 2, 0);
        points.second = visualRect(index_to).center() - QPointF(0, height_2 / 2);
    }
    // left -> right
    else if( (row_1 == row_2) && (col_1 > col_2) )
    {
        points.first = visualRect(index_from).center() - QPointF(width_1 / 2, 0);
        points.second = visualRect(index_to).center() + QPointF(width_2 / 2, 0);
    }
    // right -> left
    else if( (row_1 == row_2) && (col_1 < col_2) )
    {
        points.first = visualRect(index_from).center() + QPointF(width_1 / 2, 0);
        points.second = visualRect(index_to).center() - QPointF(width_2 / 2, 0);
    }
    // bottom -> top
    else if( (row_1 > row_2) && (col_1 == col_2) )
    {
        points.first = visualRect(index_from).center() - QPointF(0, height_1 / 2);
        points.second = visualRect(index_to).center() + QPointF(0, height_2 / 2);
    }
    // top -> bottom
    else if( (row_1 < row_2) && (col_1 == col_2) )
    {
        points.first = visualRect(index_from).center() + QPointF(0, height_1 / 2);
        points.second = visualRect(index_to).center() - QPointF(0, height_2 / 2);
    }
    else
    {
        points.first = visualRect(index_from).center();
        points.second = visualRect(index_to).center();
    }
    return true;
}

