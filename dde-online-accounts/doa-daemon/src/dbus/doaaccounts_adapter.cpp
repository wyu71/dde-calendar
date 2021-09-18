/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp com.dde.onlineaccount.accounts.xml -a doaaccounts_adapter -c DOAAccountsadapter
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "doaaccounts_adapter.h"
#include "dbus_consts.h"
#include "utils.h"
#include "aesencryption.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class DOAAccountsadapter
 */

DOAAccountsadapter::DOAAccountsadapter(QObject *parent)
    : QObject(parent)
    , m_checkAccountCalendarTimer(new QTimer(this))
{
    //定时验证当前帐户状态
    connect(m_checkAccountCalendarTimer, &QTimer::timeout, this, &DOAAccountsadapter::CheckAccountState);
}

DOAAccountsadapter::~DOAAccountsadapter()
{
    // destructor
    qWarning() << "~DOAAccountsadapter";
    if (m_doaProvider != nullptr) {
        delete m_doaProvider;
        m_doaProvider = nullptr;
    }
}

/**
 * @brief DOAAccountsadapter::CheckAccountState
 * 定时判断当前状态的状态
 */
void DOAAccountsadapter::CheckAccountState()
{
    //登录验证
    DOAProvider::LoginState ret = m_doaProvider->login();

    if (ret != DOAProvider::SUCCESS) {
        emit this->sign_accountState("AccountStat", QString("%1:%2").arg(DOAProvider::TIMEOUT).arg(QMetaEnum::fromType<DOAProvider::LoginState>().valueToKey(ret)));
    }

    if (ret != m_doaProvider->getAccountStat()) {
        m_doaProvider->setAccountStat(ret);
        //更新数据库
        emit this->sign_changeProperty("Status", m_doaProvider);
    }
}

bool DOAAccountsadapter::calendarDisabled() const
{
    // get the value of property CalendarDisabled
    return m_doaProvider->getCalendarDisabled();
}

/**
 * @brief DOAAccountsadapter::setCalendarDisabled
 * @param value true:禁用同步状态 false：启用同步状态
 * 设置日历同步功能
 */
void DOAAccountsadapter::setCalendarDisabled(bool value)
{
    if (value) {
        //禁用同步
        m_checkAccountCalendarTimer->stop();
    } else {
        //启用日历同步 暂定2分钟循环判断帐户状态
        m_checkAccountCalendarTimer->start(120000);
    }

    if (m_doaProvider->getCalendarDisabled() != value) {
        m_doaProvider->setCalendarDisabled(value);
        //更新数据库
        emit this->sign_changeProperty("CalendarDisable", m_doaProvider);
    }
}

QString DOAAccountsadapter::id() const
{
    // get the value of property Id
    return m_doaProvider->getAccountID();
}

/**
 * @brief DOAAccountsadapter::userName
 * @return
 * 获取用户名
 */
QString DOAAccountsadapter::userName() const
{
    return m_doaProvider->getDisplayName();
}

void DOAAccountsadapter::setUserName(QString &userName)
{
    m_doaProvider->setDisplayName(userName);
    //更新数据库
    emit this->sign_changeProperty("UserName", m_doaProvider);
}

QString DOAAccountsadapter::providerName() const
{
    // get the value of property ProviderName 帐户后缀
    return QMetaEnum::fromType<DOAProvider::AccountType>().valueToKey(m_doaProvider->getProviderName());
}

QString DOAAccountsadapter::providerType() const
{
    // get the value of property ProviderType
    return QLatin1String(QMetaEnum::fromType<DOAProvider::ProtocolType>().valueToKey(m_doaProvider->getProtocol()));
}

/**
 * @brief DOAAccountsadapter::Remove
 * 删除帐户
 */
bool DOAAccountsadapter::Remove()
{
    m_checkAccountCalendarTimer->stop();
    disconnect(m_checkAccountCalendarTimer, &QTimer::timeout, this, &DOAAccountsadapter::CheckAccountState);
    m_checkAccountCalendarTimer->deleteLater();

    emit this->sign_remove(this);
    return true;
}

void DOAAccountsadapter::loginCancle()
{
    m_doaProvider->loginCancel();
}
