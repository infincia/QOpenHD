#include "fcmavlinkmissionitemsmodel.h"
#include "qdebug.h"


FCMavlinkMissionItemsModel::FCMavlinkMissionItemsModel(QObject *parent)
    :  QAbstractListModel(parent)
{
    connect(this, &FCMavlinkMissionItemsModel::signalAddElement, this, &FCMavlinkMissionItemsModel::do_not_call_me_add_element);
    /*double lat=0;
    for(int i=0;i<10;i++){
        add_element({lat,0});
        lat += 2;
    }*/
    for(int i=0;i<100;i++){
        //test_add();
    }
    /*m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, this, &FCMavlinkMissionItemsModel::timercb);
    m_timer->start(1000);*/
}

FCMavlinkMissionItemsModel& FCMavlinkMissionItemsModel::instance()
{
    static FCMavlinkMissionItemsModel instance;
    return instance;
}

void FCMavlinkMissionItemsModel::add_element(FCMavlinkMissionItemsModel::Element el)
{
    emit signalAddElement(el.latitude,el.longitude,el.mission_index);
}

void FCMavlinkMissionItemsModel::hack_add_el_if_nonexisting(double lat, double lon,int mission_index)
{
    auto tmp=std::make_pair(lat,lon);
    if(m_map.find(tmp)==m_map.end()){
        add_element({lat,lon,mission_index});
        m_map[tmp]=nullptr;
    }
    qDebug()<<"N mission items:"<<m_map.size();
}

void FCMavlinkMissionItemsModel::test_add()
{
    add_element({last,last});
    last +=1;
}

int FCMavlinkMissionItemsModel::rowCount( const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.count();
}

QVariant FCMavlinkMissionItemsModel::data(const QModelIndex &index, int role) const
{
    qDebug()<<"FCMavlinkMissionItemsModel::data at "<<index<<" Role:"<<role;
    if ( !index.isValid() ){
        qDebug()<<"invalid index";
        return QVariant();
    }

    const FCMavlinkMissionItemsModel::Element &data = m_data.at(index.row());
    if ( role == LatitudeRole )
        return data.latitude;
    else if ( role == LongitudeRole )
        return data.longitude;
    else if (role==MissionIndexRole){
        return data.mission_index;
    }
    else
        return QVariant();
}

QHash<int, QByteArray> FCMavlinkMissionItemsModel::roleNames() const
{
    static QHash<int, QByteArray> mapping {
        {LatitudeRole, "latitude"},
        {LongitudeRole, "longitude"},
        {MissionIndexRole, "mission_index"}
    };
    return mapping;
}

void FCMavlinkMissionItemsModel:: do_not_call_me_add_element(double lat,double lon,int mission_index){
    FCMavlinkMissionItemsModel::Element element{};
    element.latitude=lat;
    element.longitude=lon;
    element.mission_index=mission_index;
    addData(element);
}

void FCMavlinkMissionItemsModel::removeData(int row)
{
    if (row < 0 || row >= m_data.count())
        return;

    //qDebug()<<"Removing "<<row;
    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    endRemoveRows();
}

void FCMavlinkMissionItemsModel::addData(FCMavlinkMissionItemsModel::Element data)
{
    //qDebug()<<"FCMavlinkMissionItemsModel::addData"<<data.latitude<<","<<data.longitude;
    //qDebug()<<"A Row count:"<<rowCount();
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_data.push_back(data);
    endInsertRows();
    //qDebug()<<"B Row count:"<<rowCount();
}

void FCMavlinkMissionItemsModel::timercb()
{
    test_add();
}

