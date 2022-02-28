#include "craftproxymodel.h"
#include "craftmodel.h"

CraftProxyModel::CraftProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{

}

static bool filterAcceptColumn(const QString filter, int row, CraftModel::Column col, QAbstractItemModel* model, const QModelIndex &parent)
{
    if(!filter.isEmpty()){
        QString tmpFilter = filter;
        QModelIndex idx = model->index(row, col, parent);
        bool negative = filter.at(0) == '!';
        if(negative){
            tmpFilter = tmpFilter.right(1);
        }
        auto contains = (model->data(idx).toString().contains(tmpFilter, Qt::CaseInsensitive));
        if((!negative && !contains) || (negative && contains)){
            return false;
        }
    }
    return true;
}

bool CraftProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    if (!filterAcceptColumn(m_callsignFilter, row, CraftModel::CM_CALLSIGN, sourceModel(), parent )){
        return false;
    }
    if (!filterAcceptColumn(m_hexFilter, row, CraftModel::CM_HEX, sourceModel(), parent )){
        return false;
    }
    if (!filterAcceptColumn(m_codeFilter, row, CraftModel::CM_CODE, sourceModel(), parent )){
        return false;
    }
    if (!filterAcceptColumn(m_typeFilter, row, CraftModel::CM_TYPE, sourceModel(), parent )){
        return false;
    }
    if (!filterAcceptColumn(m_flagFilter, row, CraftModel::CM_FLAG, sourceModel(), parent )){
        return false;
    }
    if (!filterAcceptColumn(m_registrationFilter, row, CraftModel::CM_REG, sourceModel(), parent )){
        return false;
    }
    if (!filterAcceptColumn(m_squawkFilter, row, CraftModel::CM_SQUAWK, sourceModel(), parent )){
        return false;
    }

    if(!m_altMinFilter.isEmpty() || !m_altMaxFilter.isEmpty()){
        QModelIndex idx = sourceModel()->index(row, CraftModel::CM_ALT, parent);
        bool ok = false;
        auto alt = sourceModel()->data(idx).toDouble(&ok);
        if(ok){
            if(!m_altMinFilter.isEmpty()){
                auto min = m_altMinFilter.toDouble(&ok);
                if (ok && alt < min){
                    return false;
                }
            }
            if(!m_altMaxFilter.isEmpty()){
                auto max = m_altMaxFilter.toDouble(&ok);
                if (ok && alt > max){
                    return false;
                }
            }
        }
    }

    if(!m_speedMinFilter.isEmpty() || !m_speedMaxFilter.isEmpty()){
        QModelIndex idx = sourceModel()->index(row, CraftModel::CM_SPEED, parent);
        bool ok = false;
        auto speed = sourceModel()->data(idx).toDouble(&ok);
        if(ok){
            if(!m_speedMinFilter.isEmpty()){
                auto min = m_speedMinFilter.toDouble(&ok);
                if (ok && speed < min){
                    return false;
                }
            }
            if(!m_speedMaxFilter.isEmpty()){
                auto max = m_speedMaxFilter.toDouble(&ok);
                if (ok && speed > max){
                    return false;
                }
            }
        }
    }

    if(!m_distanceFilter.isEmpty()){
        QModelIndex idx = sourceModel()->index(row, CraftModel::CM_DIST, parent);
        bool ok = false;
        auto dist = sourceModel()->data(idx).toDouble(&ok);
        if(ok){
            auto db = m_distanceFilter.toDouble(&ok);
            if (ok && dist > db){
                return false;
            }
        }
    }

    return true;
}

void CraftProxyModel::setCallsignFilter(QString filter)
{
    if (m_callsignFilter != filter){
        m_callsignFilter = filter;
        invalidateFilter();
    }
}

void CraftProxyModel::setHexFilter(QString filter)
{
    if (m_hexFilter != filter){
        m_hexFilter = filter;
        invalidateFilter();
    }
}

void CraftProxyModel::setCodeFilter(QString filter)
{
    if (m_codeFilter != filter){
        m_codeFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setTypeFilter(QString filter)
{
    if (m_typeFilter != filter){
        m_typeFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setFlagFilter(QString filter)
{
    if (m_flagFilter != filter){
        m_flagFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setAltMinFilter(QString filter)
{
    if (m_altMinFilter != filter){
        m_altMinFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setAltMaxFilter(QString filter)
{
    if (m_altMaxFilter != filter){
        m_altMaxFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setSpeedMinFilter(QString filter)
{
    if (m_speedMinFilter != filter){
        m_speedMinFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setSpeedMaxFilter(QString filter)
{
    if (m_speedMaxFilter != filter){
        m_speedMaxFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setRegistrationFilter(QString filter)
{
    if (m_registrationFilter != filter){
        m_registrationFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setSquawkFilter(QString filter)
{
    if (m_squawkFilter != filter){
        m_squawkFilter = filter;
        invalidateFilter();
    }
}
void CraftProxyModel::setDistanceFilter(QString filter)
{
    if (m_distanceFilter != filter){
        m_distanceFilter = filter;
        invalidateFilter();
    }
}

void CraftProxyModel::resetFilter()
{
    m_callsignFilter = "";
    m_hexFilter = "";
    m_codeFilter = "";
    m_typeFilter = "";
    m_flagFilter = "";
    m_registrationFilter = "";
    m_squawkFilter = "";
    m_distanceFilter = "";

    m_altMinFilter = "";
    m_altMaxFilter = "";
    m_speedMinFilter = "";
    m_speedMaxFilter = "";
    invalidateFilter();
}
