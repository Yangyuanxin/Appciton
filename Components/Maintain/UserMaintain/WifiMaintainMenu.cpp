#include "WifiMaintainMenu.h"
#include "FontManager.h"
#include "IComboList.h"
#include "NetworkManager.h"
#include "Debug.h"
#include "IListWidget.h"
#include "IConfig.h"
#include "ILabel.h"
#include "LabelButton.h"
#include <QFocusEvent>
#include "WiFiProfileEditor.h"
#include "IMessageBox.h"
#include "ColorManager.h"
#include "SupervisorMenuManager.h"

#define PROFILE_LIST_ITEM_H 30
#define PROFILE_LIST_ITEM_W 200
#define PROFILE_MAX_NUM 5

class WifiMaintainMenuPrivate
{
public:
    Q_DECLARE_PUBLIC(WifiMaintainMenu)

    WifiMaintainMenuPrivate(WifiMaintainMenu * const q_ptr)
        :q_ptr(q_ptr), switchCombo(NULL), profileList(NULL),
        addBtn(NULL), editBtn(NULL), delBtn(NULL),
        lastSelectItem(NULL){}

    void onSwitch(int val);
    void onProfileItemClick();
    void updateProfileList();
    void onListExit(bool backtab);
    void onBtnClick();
    void loadProfiles();
    void saveProfiles();

    WifiMaintainMenu * const q_ptr;
    IComboList *switchCombo ;
    IListWidget *profileList;
    LButtonEx *addBtn;
    LButtonEx *editBtn;
    LButtonEx *delBtn;
    QListWidgetItem *lastSelectItem;
    QVector<WiFiProfileInfo> profiles;
};

/***************************************************************************************************
 * onSwitch: private slot, handle the switch combo button click event
 **************************************************************************************************/
void WifiMaintainMenuPrivate::onSwitch(int val)
{
    systemConfig.setNumValue("WiFi|EnableWifi", val);
    systemConfig.save();
}

/***************************************************************************************************
 * onProfileItemClick : private slot, handle the profile list item click signal
 **************************************************************************************************/
void WifiMaintainMenuPrivate::onProfileItemClick()
{
    QListWidgetItem *item = profileList->currentItem();
    if(lastSelectItem)
    {
        lastSelectItem->setIcon(QIcon());
    }

    if(item != lastSelectItem)
    {
        item->setIcon(QIcon("/usr/local/nPM/icons/select.png"));
        lastSelectItem = item;
    }
    else
    {
        lastSelectItem = NULL;
    }

    if(lastSelectItem)
    {
        delBtn->setEnabled(true);
        editBtn->setEnabled(true);
    }
    else  {
        delBtn->setEnabled(false);
        editBtn->setEnabled(false);
    }
}

/***************************************************************************************************
 * updateProfileList : Update the profile List item from config
 **************************************************************************************************/
void WifiMaintainMenuPrivate::updateProfileList()
{
    //remove old item
    while(profileList->count())
    {
        QListWidgetItem *item = profileList->takeItem(0);
        delete item;
    }

    for(int i = 0; i < profiles.count(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(profiles.at(i).profileName, profileList);
        item->setSizeHint(QSize(PROFILE_LIST_ITEM_W, PROFILE_LIST_ITEM_H));
    }

    int count = profileList->count();

    if(count)
    {
        profileList->setFocusPolicy(Qt::StrongFocus);
    }
    else
    {
        profileList->setFocusPolicy(Qt::NoFocus);
    }

    if(count >= PROFILE_MAX_NUM)
    {
        addBtn->setEnabled(false);
    }
    else
    {
        addBtn->setEnabled(true);
    }
}

/***************************************************************************************************
 * onListExit : private slot, handle IListWidget exitList event
 **************************************************************************************************/
void WifiMaintainMenuPrivate::onListExit(bool backTab)
{
    Q_Q(WifiMaintainMenu);
    if(backTab)
    {
        q->focusPreviousChild();
    }
    else
    {
        q->focusNextChild();
    }
}

/***************************************************************************************************
 * caseInsensitiveLessThan, use to compare profile name
 **************************************************************************************************/
static bool caseInsensitiveLessThan(const WiFiProfileInfo &p1, const WiFiProfileInfo &p2)
{
    return p1.profileName.toLower() < p2.profileName.toLower();
}

/***************************************************************************************************
 * onBtnClick : handle add, edit, delete button click signal
 **************************************************************************************************/
void WifiMaintainMenuPrivate::onBtnClick()
{
    Q_Q(WifiMaintainMenu);
    QObject *sender = q->sender();

    if(sender == addBtn)
    {
        WiFiProfileEditor editor;
        while(editor.exec())
        {
            QVector<WiFiProfileInfo>::ConstIterator iter;
            WiFiProfileInfo editProfile = editor.getProfileInfo();
            bool duplicate = false;
            for(iter = profiles.constBegin(); iter !=profiles.constEnd(); iter ++)
            {
                if(iter->profileName == editProfile.profileName)
                {
                    duplicate = true;
                    break;
                }
            }

            if(duplicate)
            {
                QString title = trs("ProfileConflict");
                IMessageBox msgBox(title, QString("%1 %2 %3").arg(trs("Profile")).arg(editProfile.profileName).arg(trs("AlreadyExist")), false);
                msgBox.exec();
                continue;
            }
            else
            {
                profiles.append(editor.getProfileInfo());
                qSort(profiles.begin(), profiles.end(), caseInsensitiveLessThan);
                updateProfileList();
                lastSelectItem = NULL;
                saveProfiles();
                break;
            }
        }
    }
    else if (sender == editBtn)
    {
        if(lastSelectItem)
        {
            int index = profileList->row(lastSelectItem);
            WiFiProfileEditor editor(profiles[index]);
            while (editor.exec()) {
                bool duplicate = false;
                QString profileName = editor.getProfileInfo().profileName;
                for(int i = 0;  i < profiles.size(); i++)
                {
                    if(i != index && profileName == profiles[i].profileName)
                    {
                        duplicate = true;
                        break;
                    }
                }

                if(duplicate)
                {
                    QString title = trs("ProfileConflict");
                    IMessageBox msgBox(title, QString("%1 %2 %3").arg(trs("Profile")).arg(profileName).arg(trs("AlreadyExist")), false);
                    msgBox.exec();
                    continue;
                }

                profiles[index] = editor.getProfileInfo();
                qSort(profiles.begin(), profiles.end(), caseInsensitiveLessThan);
                updateProfileList();
                lastSelectItem = NULL;
                saveProfiles();

                for(int i = 0; i< profiles.size(); i++)
                {
                    if(profiles[i].profileName == profileName)
                    {
                        profileList->setCurrentRow(i);
                        QMetaObject::invokeMethod(profileList, "realRelease", Qt::QueuedConnection);
                        break;
                    }
                }

                break;

            }
        }
    }
    else if (sender == delBtn) {
        if(lastSelectItem)
        {
            IMessageBox messageBox(trs("Prompt"), trs("DeleteSelectedWifiProfile"));
            if(messageBox.exec() == 0)
            {
                return;
            }

            int index = profileList->row(lastSelectItem);
            profiles.remove(index);
            qSort(profiles.begin(), profiles.end(), caseInsensitiveLessThan);
            updateProfileList();
            saveProfiles();
            lastSelectItem = NULL;
            delBtn->setEnabled(false);
            editBtn->setEnabled(false);
        }
    }
    else
    {
        qDebug()<<Q_FUNC_INFO<<"Unknown singal sender!";
    }
}

/***************************************************************************************************
 * loadProfiles, load the profiles from the config file
 **************************************************************************************************/
void WifiMaintainMenuPrivate::loadProfiles()
{
    QString tmpStr;
    int tmpValue=0;
    int count;
    bool ok;
    if(!systemConfig.getStrAttr("WiFi|Profiles", "Count", tmpStr))
    {
        return;
    }

    count = tmpStr.toInt(&ok);

    if(!ok)
    {
        return;
    }

    profiles.clear();
    for(int i = 0; i < count; i++)
    {
        QString prefix = QString("WiFi|Profiles|Profile%1|").arg(i);
        WiFiProfileInfo profile;
        systemConfig.getStrValue(prefix + "ProfileName", profile.profileName);
        systemConfig.getStrValue(prefix + "SSID", profile.ssid);
        systemConfig.getNumValue(prefix + "AuthType", tmpValue);
        profile.authType = (WiFiProfileInfo::AuthenticationType) tmpValue;
        systemConfig.getStrValue(prefix + "SecurityKey", profile.securityKey);
        systemConfig.getNumValue(prefix + "IsStatic", tmpValue);
        profile.isStatic = tmpValue;
        if(profile.isStatic)
        {
            systemConfig.getStrValue(prefix + "StaticIP", profile.staticIp);
            systemConfig.getStrValue(prefix + "DefaultGateway", profile.defaultGateway);
            systemConfig.getStrValue(prefix + "SubnetMask", profile.subnetMask);
            systemConfig.getStrValue(prefix + "PreferedDNS", profile.preferedDNS);
            systemConfig.getStrValue(prefix + "AlternateDNS", profile.alternateDNS);
        }

        profiles.append(profile);
    }
    qSort(profiles.begin(), profiles.end(), caseInsensitiveLessThan);
}

/***************************************************************************************************
 * saveProfiles : save the profiles, it will delete all the old profiles and add the new profiles
 **************************************************************************************************/
void WifiMaintainMenuPrivate::saveProfiles()
{
    bool ok;
    QString tmpStr;
    if(!systemConfig.getStrAttr("WiFi|Profiles", "Count", tmpStr))
    {
        return;
    }

    int count = tmpStr.toInt(&ok);

    if(!ok)
    {
        return;
    }

    //remove old profile
    for(int i = 0; i < count; i++)
    {
        QString prefix = QString("WiFi|Profiles|Profile%1").arg(i);
        systemConfig.removeNode(prefix);
    }

    //add new profile
    for(int j = 0; j < profiles.count(); j++)
    {
        QString prefix("WiFi|Profiles");
        QString nodeName = QString("Profile%1").arg(j);
        systemConfig.addNode(prefix, nodeName);
        prefix = prefix + "|" + nodeName;
        systemConfig.addNode(prefix, "ProfileName", profiles.at(j).profileName);
        systemConfig.addNode(prefix, "SSID", profiles.at(j).ssid);
        systemConfig.addNode(prefix, "AuthType", QString::number((int)profiles.at(j).authType));
        systemConfig.addNode(prefix, "SecurityKey", profiles.at(j).securityKey);
        systemConfig.addNode(prefix, "IsStatic", QString::number(profiles.at(j).isStatic));
        if(profiles.at(j).isStatic)
        {
            systemConfig.addNode(prefix, "StaticIP", profiles.at(j).staticIp);
            systemConfig.addNode(prefix, "DefaultGateway", profiles.at(j).defaultGateway);
            systemConfig.addNode(prefix, "SubnetMask", profiles.at(j).subnetMask);
            systemConfig.addNode(prefix, "PreferedDNS", profiles.at(j).preferedDNS);
            systemConfig.addNode(prefix, "AlternateDNS", profiles.at(j).alternateDNS);
        }
    }

    systemConfig.setStrAttr("WiFi|Profiles", "Count", QString::number(profiles.count()));
    int currentSelect = -1; //default select nothing if wifi profile changes
    systemConfig.setNumAttr("WiFi|Profiles", "CurrentSelect", currentSelect);

    systemConfig.save();
}

/***************************************************************************************************
 * Singleton mode, get a instance
 **************************************************************************************************/
WifiMaintainMenu &WifiMaintainMenu::construction()
{
    static WifiMaintainMenu *_instance = NULL;
    if(_instance == NULL)
    {
        _instance =  new WifiMaintainMenu;
    }
    return *_instance;
}

/***************************************************************************************************
 * focusFirstItem : set the first widget to get focus
 **************************************************************************************************/
void WifiMaintainMenu::focusFirstItem()
{
    //has set the focus proxy, just set this widget foucs will make the proxy get focus
    this->setFocus();

}

/***************************************************************************************************
 * eventFilter : handle the profile list focus in event, the operation of the list will be more natrual.
 *              If we don't handle the focus in event, the focus item will be the last item of the list
 *              when the focus reason is tab, or the first item when the focus reason is back tab.
 *
 *              Handle the profile list hide event, clear the select item when the widget is hidden.
 **************************************************************************************************/
bool WifiMaintainMenu::eventFilter(QObject *obj, QEvent *ev)
{
    Q_D(WifiMaintainMenu);
    if(obj == d->profileList)
    {
        if (ev->type() == QEvent::FocusIn)
        {
            QFocusEvent *e = static_cast<QFocusEvent *>(ev);
            if(e->reason() == Qt::TabFocusReason)
            {
                d->profileList->setCurrentRow(0);
            }
            else if (e->reason() == Qt::BacktabFocusReason)
            {
                d->profileList->setCurrentRow(d->profileList->count() - 1);
            }
        }

        if (ev->type() == QEvent::Hide)
        {
            if(d->lastSelectItem)
            {
                d->lastSelectItem->setIcon(QIcon());
                d->delBtn->setEnabled(false);
                d->editBtn->setEnabled(false);
                d->lastSelectItem = NULL;
            }
        }
    }
    return false;
}

/***************************************************************************************************
 * layoutExec : layout issue
 **************************************************************************************************/
void WifiMaintainMenu::layoutExec()
{
    Q_D(WifiMaintainMenu);
    int submenuW = supervisorMenuManager.getSubmenuWidth();
    int submenuH = supervisorMenuManager.getSubmenuHeight();
    setMenuSize(submenuW, submenuH);

    int fontSize = fontManager.getFontSize(1);

    int itemW = submenuW - ICOMBOLIST_SPACE;
    int btnWidth = itemW / 2;
    int labelWidth = itemW - btnWidth;

    //switch combo list
    d->switchCombo = new IComboList(trs("WiFiMenu"));
    d->switchCombo->setParent(this);
    this->setFocusProxy(d->switchCombo);
    d->switchCombo->setFont(fontManager.textFont(fontSize));
    d->switchCombo->addItem(trs("Off"));
    d->switchCombo->addItem(trs("On"));
    connect(d->switchCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onSwitch(int)));
    d->switchCombo->label->setFixedSize(labelWidth, ITEM_H);
    d->switchCombo->combolist->setFixedSize(btnWidth, ITEM_H);
    mainLayout->addWidget(d->switchCombo);


    //configure label
    QLabel *label = new QLabel();
    label->setFont(fontManager.textFont(fontSize));
    QMargins margin = label->contentsMargins();
    margin.setTop(10);
    margin.setLeft(15);
    margin.setBottom(10);
    label->setContentsMargins(margin);
    label->setText(trs("ConfiguredAccessPointProfiles"));
    mainLayout->addWidget(label);

    //profile list
    d->profileList =  new IListWidget();
    d->profileList->setFont(fontManager.textFont(fontSize));
    d->profileList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->profileList->setSelectionMode(QAbstractItemView::SingleSelection);
    d->profileList->setFrameStyle(QFrame::Plain);
    d->profileList->setSpacing(2);
    d->profileList->setUniformItemSizes(true);
    d->profileList->setIconSize(QSize(16,16));

    QString profileListStyleSheet = QString("QListWidget { margin-left: 15px; border:1px solid #808080; border-radius: 2px; background-color: transparent; outline: none; }\n "
    "QListWidget::item {padding: 5px; border-radius: 2px; border: none; background-color: %1;}\n"
    "QListWidget::item:focus {background-color: %2;}").arg("white").arg(colorManager.getHighlight().name());

    d->profileList->setStyleSheet(profileListStyleSheet);
    connect(d->profileList, SIGNAL(exitList(bool)), this, SLOT(onListExit(bool)));
    connect(d->profileList, SIGNAL(realRelease()), this, SLOT(onProfileItemClick()));
    d->profileList->installEventFilter(this);
    d->profileList->setFixedHeight(174); //size for 5 items

    mainLayout->addWidget(d->profileList);

    //buttons
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(QMargins(15, 10, 0, 40));
    hlayout->setSpacing(10);
    d->addBtn = new LButtonEx();
    d->addBtn->setText(trs("Add"));
    d->addBtn->setFont(fontManager.textFont(fontSize));
    hlayout->addWidget(d->addBtn);
    connect(d->addBtn, SIGNAL(realReleased()), this, SLOT(onBtnClick()));
    d->editBtn = new LButtonEx();
    d->editBtn->setText(trs("Edit"));
    d->editBtn->setFont(fontManager.textFont(fontSize));
    d->editBtn->setEnabled(false);
    hlayout->addWidget(d->editBtn);
    connect(d->editBtn, SIGNAL(realReleased()), this, SLOT(onBtnClick()));
    d->delBtn =  new LButtonEx();
    d->delBtn->setText((trs("Delete")));
    d->delBtn->setFont(fontManager.textFont(fontSize));
    d->delBtn->setEnabled(false);
    hlayout->addWidget(d->delBtn);
    connect(d->delBtn, SIGNAL(realReleased()), this, SLOT(onBtnClick()));

    mainLayout->addLayout(hlayout);
}

/***************************************************************************************************
 * call before display, load configurations here
 **************************************************************************************************/
void WifiMaintainMenu::readyShow()
{
    Q_D(WifiMaintainMenu);
    bool enableWifi = false;
    systemConfig.getNumValue("WiFi|EnableWifi", enableWifi);
    d->switchCombo->setCurrentIndex(enableWifi);
    d->loadProfiles();
    d->updateProfileList();
}

/***************************************************************************************************
 * constructor
 **************************************************************************************************/
WifiMaintainMenu::WifiMaintainMenu()
    :SubMenu(trs("WiFiMenu")),d_ptr(new WifiMaintainMenuPrivate(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setDesc(trs("WiFiMenuDesc"));
    startLayout();
}

#include "moc_WifiMaintainMenu.cpp"

