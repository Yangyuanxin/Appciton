/**
 ** This file is part of the nPM project.
 ** Copyright (C) Better Life Medical Technology Co., Ltd.
 ** All Rights Reserved.
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 **
 ** Written by ZhongHuan Duan duanzhonghuan@blmed.cn, 2018/7/23
 **/

#include "FactoryImportExportMenuContent.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "Button.h"
#include <QList>
#include <QListWidgetItem>
#include <QDomDocument>
#include <QTextStream>
#include "MessageBox.h"
#include "ConfigManagerMenuWindow.h"
#include <QProcess>
#include "ImportFileSubWidget.h"
#include <QDir>
#include "ListView.h"
#include "ListDataModel.h"
#include "ListViewItemDelegate.h"
#include "LanguageManager.h"
#include "USBManager.h"
#include "ThemeManager.h"
#include <QTimer>

#define CONFIG_MAX_NUM        (6)
#define USB0_DEVICE_NODE      ("/dev/sda2")
#define USB0_PATH_NAME        ("/mnt/usb0")
#define CONFIG_DIR            ("/usr/local/nPM/etc/")
#define CONFIG_FILE_ADULT     ("AdultConfig.Original.xml")
#define CONFIG_FILE_NEO       ("NeoConfig.Original.xml")
#define CONFIG_FILE_PED       ("PedConfig.Original.xml")
#define CONFIG_FILE_SYSTEM    ("System.Original.xml")
#define CONFIG_FILE_MACHINE   ("Machine.xml")
#define LISTVIEW_MAX_VISIABLE_TIME (5)
#define HEIGHT_HINT (themeManger.getAcceptableControlHeight())

class FactoryImportExportMenuContentPrivate
{
public:
    struct FactoryConfigInfo
    {
        QString name;         // name of the this config
        QString fileName;     // xml filename
    };

    FactoryImportExportMenuContentPrivate():
        attrCheckFlag(true),
        configListView(NULL),
        configDataModel(NULL),
        exportBtn(NULL),
        importBtn(NULL),
        importFlag(255),
        infoLab(NULL),
        timer(NULL)
    {
        configs.clear();
        exportFileName.clear();
        importFileName.clear();
        localXml.clear();
        importXml.clear();
    }

    /**
     * @brief loadConfigs  装载数据方法
     */
    void loadConfigs();
    /**
     * @brief updateConfigList  更新配置列表
     */
    void updateConfigList();

    QList<FactoryConfigInfo> configs;
    QStringList      exportFileName;  //  导出的XML文件名称的链表
    QStringList      importFileName;  //  导入的XML文件名称的链表

    QTextStream       textStream;  //  日志接口

    QDomDocument      localXml;  //  本地检查文件

    QDomDocument      importXml;  //  导入检查文件

    bool attrCheckFlag;

    ListView   *configListView;
    ListDataModel *configDataModel;
    Button *exportBtn;
    Button *importBtn;
    int importFlag;
    QLabel *infoLab;  // 记录usb是否存在信息
    QTimer *timer;
};

FactoryImportExportMenuContent::FactoryImportExportMenuContent()
    : MenuContent(trs("ExportImport"),
                  trs("ExportImportDesc")),
      d_ptr(new FactoryImportExportMenuContentPrivate)
{
}

FactoryImportExportMenuContent::~FactoryImportExportMenuContent()
{
    delete d_ptr;
}

void FactoryImportExportMenuContent::readyShow()
{
    d_ptr->loadConfigs();
    d_ptr->updateConfigList();
}

void FactoryImportExportMenuContent::showEvent(QShowEvent *ev)
{
    MenuContent::showEvent(ev);

    onTimeOut();
}

void FactoryImportExportMenuContent::layoutExec()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(10);
    layout->setAlignment(Qt::AlignTop);

    QLabel *label = new QLabel(trs("ExportImportConfig"));
    layout->addWidget(label);

    ListView *listView = new ListView();
    listView->setItemDelegate(new ListViewItemDelegate(listView));
    layout->addWidget(listView);
    ListDataModel *model = new ListDataModel(this);
    d_ptr->configDataModel = model;
    listView->setFixedHeight(LISTVIEW_MAX_VISIABLE_TIME * model->getRowHeightHint()
                             + listView->spacing() * (LISTVIEW_MAX_VISIABLE_TIME * 2));
    listView->setModel(model);
    listView->setRowsSelectMode(true);
    connect(listView, SIGNAL(enterSignal()), this, SLOT(updateBtnStatus()));
    d_ptr->configListView = listView;

    QHBoxLayout *hl = new QHBoxLayout;
    Button *button;

    button = new Button(trs("Export"));
    button->setButtonStyle(Button::ButtonTextOnly);
    hl->addWidget(button);
    connect(button, SIGNAL(released()), this, SLOT(onBtnClick()));
    d_ptr->exportBtn = button;

    button = new Button(trs("Import"));
    button->setButtonStyle(Button::ButtonTextOnly);
    hl->addWidget(button);
    connect(button, SIGNAL(released()), this, SLOT(onBtnClick()));
    d_ptr->importBtn = button;

    layout->addLayout(hl);

    label = new QLabel(trs("WarningNoUSB"));
    label->setFixedHeight(HEIGHT_HINT);
    d_ptr->infoLab = label;
    layout->addWidget(label, 1, Qt::AlignRight);

    layout->addStretch(1);

    QTimer *timer = new QTimer;
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    timer->start();
    d_ptr->timer = timer;
}


void FactoryImportExportMenuContentPrivate::loadConfigs()
{
    QStringList factoryConfigInfoList;
    factoryConfigInfoList.append(CONFIG_FILE_ADULT);
    factoryConfigInfoList.append(CONFIG_FILE_NEO);
    factoryConfigInfoList.append(CONFIG_FILE_PED);
    factoryConfigInfoList.append(CONFIG_FILE_SYSTEM);
    factoryConfigInfoList.append(CONFIG_FILE_MACHINE);

    configs.clear();
    FactoryConfigInfo factoryConfigInfo;
    for (int i = 0; i < factoryConfigInfoList.count(); i++)
    {
        if (QFile::exists(QString("%1%2").arg(CONFIG_DIR).arg(factoryConfigInfoList.at(i))))
        {
            factoryConfigInfo.fileName = factoryConfigInfoList.at(i);
            factoryConfigInfo.name = factoryConfigInfoList.at(i);
            factoryConfigInfo.name.replace(".Original", "");
            factoryConfigInfo.name.replace(".xml", "OfFactory");
            configs.append(factoryConfigInfo);
        }
    }
}

void FactoryImportExportMenuContentPrivate::updateConfigList()
{
    QStringList configNameList;
    for (int i = 0; i < configs.count(); i++)
    {
        QString s = configs.at(i).name.left(configs.at(i).name.indexOf(QRegExp("[A-Z]"), 1));
        configNameList.append(QString("%1(%2)").arg(trs("FactoryConfig")).arg(trs(s)));
    }
    configDataModel->setStringList(configNameList);

    int curSelectedRow = configListView->curCheckedRow();
    bool isEnable;
    if (curSelectedRow == -1)
    {
        isEnable = false;
    }
    else
    {
        isEnable = true;
    }
    exportBtn->setEnabled(isEnable);

    if (configDataModel->getRowCount() >= CONFIG_MAX_NUM)
    {
        isEnable = false;
    }
    else
    {
        isEnable = true;
    }
    importBtn->setEnabled(isEnable);
}

bool FactoryImportExportMenuContent::compareTagAttribute(QDomElement importTag, QDomElement localTag)
{
#define PARAMETER_PREFIX_NUM0   (4)  /*参数前缀的长度*/
#define PARAMETER_PREFIX_NUM1   (2)  /*参数前缀的长度*/
#define PARAMETER_PREFIX0       ("Enable")  /*参数前缀*/
#define PARAMETER_PREFIX1       ("Min")  /*参数前缀*/
    float numArrary[] = {1.0, 2.0};
    QString attrString[] = {"Enable", "Prio"};
    QDomNamedNodeMap localDomNameNodeMap = localTag.attributes();

    if (localDomNameNodeMap.count() == PARAMETER_PREFIX_NUM0) /*导入标签无属性值*/
    {
        if (localDomNameNodeMap.namedItem("cAttr0").nodeValue().toFloat() == 0) /*数字属性 需要比较上下限*/
        {
            if (importTag.text().toFloat() < localDomNameNodeMap.namedItem("cMin").nodeValue().toFloat() || \
                    importTag.text().toFloat() > localDomNameNodeMap.namedItem("cMax").nodeValue().toFloat())
            {
                //  信息加入日志
                d_ptr->textStream << QString("Inserted xml files NodeName(%1)`s number value parser failed !!!\r\n").arg(
                                      localTag.nodeName());
                float floatMin = localDomNameNodeMap.namedItem("cMin").nodeValue().toFloat();
                float floatMax = localDomNameNodeMap.namedItem("cMax").nodeValue().toFloat();
                d_ptr->textStream << QString("(%1 %2 %3)\r\n").arg(importTag.text().toFloat()).arg(floatMin).arg(floatMax);
                return false;
            }
        }
        else/*字符串属性 直接比较是否相等*/
        {
            if (importTag.nodeValue() != localTag.nodeValue())
            {
                //  信息加入日志
                d_ptr->textStream << QString("Inserted xml files NodeName(%1)`s string value parser failed !!!\r\n").arg(
                                      localTag.nodeName());
                return false;
            }
        }
    }
    else if (localDomNameNodeMap.count() == PARAMETER_PREFIX_NUM1) /*导入标签有属性值的*/
    {
        int localAttrbuiteSize = localDomNameNodeMap.count();
        for (int i = 0; i < localAttrbuiteSize; i++)
        {
            if (localDomNameNodeMap.item(i).nodeName() == PARAMETER_PREFIX0) /*Enable、Prio属性*/
            {
                if (importTag.attributes().namedItem(attrString[i]).nodeValue().toFloat() < 0 || \
                        importTag.attributes().namedItem(attrString[i]).nodeValue().toFloat() > numArrary[i])
                {
                    //  信息加入日志
                    d_ptr->textStream << QString("Inserted xml files NodeName(%1)`s attribute1 parser failed !!!\r\n").arg(
                                          localTag.nodeName());
                    return false;
                }
            }
            else if (localDomNameNodeMap.item(i).nodeName() == PARAMETER_PREFIX1) /*Min、Max属性*/
            {
                if (importTag.text().toFloat() < localDomNameNodeMap.namedItem("Min").nodeValue().toFloat() || \
                        importTag.text().toFloat() > localDomNameNodeMap.namedItem("Max").nodeValue().toFloat())
                {
                    //  信息加入日志
                    d_ptr->textStream << QString("Inserted xml files NodeName(%1)`s attribute2 parser failed !!!\r\n").arg(
                                          localTag.nodeName());
                    float fl = localDomNameNodeMap.namedItem("Min").nodeValue().toFloat();
                    float floa = localDomNameNodeMap.namedItem("Max").nodeValue().toFloat();
                    d_ptr->textStream << QString("(%1 %2 %3)\r\n").arg(importTag.text().toFloat()).arg(fl).arg(floa);
                    return false;
                }
            }
        }
    }
    return true;
}


bool FactoryImportExportMenuContent::checkXMLContent(QList<QDomElement> &importTagList, QDomElement &importTag)
{
    if (importTag.isNull())
    {
        QStringList nameList;

        for (int i = 0; i < importTagList.count(); i++)
        {
            nameList.append(importTagList.at(i).nodeName());
        }

        QDomElement localTag = tagFindElement(nameList);
        if (!localTag.isNull())
        {
            bool attrFlag = compareTagAttribute(importTagList.at(importTagList.count() - 1), localTag);

            if (attrFlag == false)
            {
                d_ptr->attrCheckFlag = false;
                return d_ptr->attrCheckFlag;
            }
        }

        if (importTagList.empty())
        {
            d_ptr->attrCheckFlag = true;
            return d_ptr->attrCheckFlag;
        }
        importTag = importTagList.last().nextSiblingElement();//下一个同类子节点
        importTagList.removeLast();//移除最后一个子节点
        if (importTagList.empty())
        {
            d_ptr->attrCheckFlag = true;
            return d_ptr->attrCheckFlag;
        }

        if (importTag.isNull())
        {
            importTag = importTagList.last();
            importTag = importTag.nextSiblingElement();//下一个同类子节点
            importTagList.removeLast();//移除最后一个子节点
        }
        checkXMLContent(importTagList, importTag);
    }
    else
    {
        importTagList.append(importTag);
        importTag = importTag.firstChildElement();
        checkXMLContent(importTagList, importTag);
    }

    return d_ptr->attrCheckFlag;
}

void FactoryImportExportMenuContent::onBtnClick()
{
    Button *btn = qobject_cast<Button *>(sender());

    if (btn == d_ptr->exportBtn)
    {
        bool isOk =  exportFileToUSB();
        QString paramTitle(trs("Export"));
        QString paraText;
        if (isOk)
        {
            paraText = trs("ExportFileCompleted");
            MessageBox messageBox(paramTitle, paraText, false);
            messageBox.exec();
        }
    }
    else if (btn == d_ptr->importBtn)
    {
        bool isOk = insertFileFromUSB();
        QString paramTitle(trs("Import"));
        QString paraText;
        if (isOk && d_ptr->importFlag != 0)
        {
            paraText = trs("ImportFileCompleted");
            MessageBox messageBox(paramTitle, paraText, false);
            messageBox.exec();
        }
        d_ptr->importFlag = 0;
    }
}

void FactoryImportExportMenuContent::updateBtnStatus()
{

    int curSelectedRow = d_ptr->configListView->curCheckedRow();
    bool isEnable;
    if (curSelectedRow == -1
            || !usbManager.isUSBExist())
    {
        isEnable = false;
    }
    else
    {
        isEnable = true;
    }
    d_ptr->exportBtn->setEnabled(isEnable);
}

void FactoryImportExportMenuContent::onTimeOut()
{
    return;
    bool isEnable;
    if (usbManager.isUSBExist())
    {
        isEnable = true;
        d_ptr->infoLab->hide();
    }
    else
    {
        isEnable = false;
        d_ptr->infoLab->show();
    }
    updateBtnStatus();
    d_ptr->importBtn->setEnabled(isEnable);
}

QDomElement FactoryImportExportMenuContent::tagFindElement(const QStringList &list)
{
    QDomElement tag;

    if (d_ptr->localXml.isNull())
    {
        return tag;
    }

    if (list.size() < 1)
    {
        return tag;
    }

    tag = d_ptr->localXml.firstChildElement();

    for (int i = 0; i < list.size(); i++)
    {
        if (tag.isNull())
        {
            return tag;
        }

        tag = tag.firstChildElement(list.at(i));
    }
    return tag;
}

bool FactoryImportExportMenuContent::insertFileFromUSB()
{
    //  建立日志文件
    QFile filelogg(QString("%1%2").arg(CONFIG_DIR).arg("logg.txt"));
    filelogg.open(QIODevice::WriteOnly | QIODevice::Append);
    d_ptr->textStream.setDevice(&filelogg);/*打开日志IO文件*/

    if (QFile::exists(USB0_PATH_NAME) == false)
    {
        return false;
    }

    // find the xml files
    QDir dir(QString("%1%2").arg(USB0_PATH_NAME).arg("/"));
    QStringList nameFilters;
    nameFilters.append(CONFIG_FILE_ADULT);
    nameFilters.append(CONFIG_FILE_NEO);
    nameFilters.append(CONFIG_FILE_PED);
    nameFilters.append(CONFIG_FILE_SYSTEM);
    nameFilters.append(CONFIG_FILE_MACHINE);
    QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
    if (files.isEmpty())
    {
        MessageBox messageBox(trs("Import"), trs("PleaseAddImportFiles"), false);
        messageBox.exec();
        return false;
    }

    QStringList configNameList = files;
    QStringList nameList;
    for (int i = 0; i < configNameList.count(); i++)
    {
        configNameList[i].replace(".Original", "");
        configNameList[i].replace(".xml", "OfFactory");
        QString s = configNameList.at(i).left(configNameList.at(i).indexOf(QRegExp("[A-Z]"), 1));
        nameList.append(QString("%1(%2)").arg(trs("FactoryConfig")).arg(trs(s)));
    }

    ImportSubWidget myImportSubWidget(nameList, USB0_PATH_NAME);
    bool status = myImportSubWidget.exec();
    d_ptr->importFileName.clear();
    if (status == true) //  OK
    {
        QMap<int, bool> rowsMap = myImportSubWidget.readRowsMap();
        for (int i = 0; i < files.count(); i++)
        {
            if (rowsMap[i])
            {
                d_ptr->importFileName.append(files.at(i));
            }
        }
        d_ptr->importFlag = 255;
    }
    else
    {
        d_ptr->importFlag = 0;
    }

    // load local data
    QFile fileLocal(QString("%1usercheck.xml").arg(CONFIG_DIR));
    if (!fileLocal.open(QFile::ReadOnly | QFile::Text))
    {
        MessageBox message(trs("Import"), trs("ImportFileFailed"));
        message.exec();
        return false;
    }
    if (!d_ptr->localXml.setContent(&fileLocal))
    {
        MessageBox message(trs("Import"), trs("ImportFileFailed"));
        message.exec();
        return false;
    }
    fileLocal.close();

    // load configs again
    d_ptr->loadConfigs();


    for (int i = 0; i < d_ptr->importFileName.count(); i++)
    {
        QFile fileImport(QString("%1/%2").
                         arg(USB0_PATH_NAME).
                         arg(d_ptr->importFileName.at(i)));
        if (!fileImport.open(QFile::ReadOnly | QFile::Text))
        {
            MessageBox message(trs("Import"), trs("ImportFileFailed"));
            message.exec();
            return false;
        }
        if (!d_ptr->importXml.setContent(&fileImport))
        {
            MessageBox message(trs("Import"), trs("ImportFileFailed"));
            message.exec();
            return false;
        }
        fileImport.close();

            // 依靠匹配到同名文件才导入文件，所以暂时关闭原始文件内部节点校验
//        QList<QDomElement> importTagList;
//        QDomElement  importTag = d_ptr->importXml.documentElement();

//        // check the import file
//        bool checkXmlFlag = checkXMLContent(importTagList, importTag);

//        if (!checkXmlFlag)
//        {
//            MessageBox message(trs("Import"), trs("ImportFileFailed"));
//            message.exec();
//            return false;
//        }

        // find the same file
        bool isExist;
        if (QFile::exists(QString("%1%2").
                          arg(CONFIG_DIR).
                          arg(d_ptr->importFileName.at(i))))
        {
            isExist = true;
        }
        else
        {
            isExist = false;
        }
        int importFlag = 1;
        if (isExist)
        {
            MessageBox message(trs("Import"),
                               trs(QString("%1\r\n%2?").
                                   arg(d_ptr->importFileName.at(i)).
                                   arg(trs("IfSelectTheSameNameFile"))),
                               QStringList() << trs("NotRepeated")
                               << trs("Repeated"));
            importFlag = message.exec();
        }

        // cover
        bool isCopyOk = true;
        if (importFlag)
        {
            QString fileName = QString("%1/%2").
                               arg(USB0_PATH_NAME).
                               arg(d_ptr->importFileName.at(i));
            QString newFileName = QString("%1%2").
                                  arg(CONFIG_DIR).
                                  arg(d_ptr->importFileName.at(i));
            QFile::remove(newFileName);
            isCopyOk = QFile::copy(fileName, newFileName);
        }
        if (!isCopyOk)
        {
            MessageBox message(trs("Import"),
                               trs(QString("%1\r\n%2").
                                   arg(d_ptr->exportFileName.at(i)).
                                   arg(trs("ImportFailed"))),
                               QStringList() << trs("Yes"));
            message.exec();
            return false;
        }
    }
    d_ptr->updateConfigList();

    filelogg.close();
    return true;
}


bool FactoryImportExportMenuContent::exportFileToUSB()
{
    if (QFile::exists(USB0_PATH_NAME) == false)
    {
        return false;
    }

    // get selected row file path names
    d_ptr->exportFileName.clear();
    int rowCount = d_ptr->configDataModel->getRowCount();
    QMap<int, bool> selectedMap = d_ptr->configListView->getRowsSelectMap();
    for (int i = 0; i < rowCount; i++)
    {
        if (selectedMap[i])
        {
            d_ptr->exportFileName.append(d_ptr->configs.at(i).fileName);
        }
    }

    // export data
    for (int i = 0; i < d_ptr->exportFileName.count(); i++)
    {
        bool isExist;
        if (QFile::exists(QString("%1/%2").
                          arg(USB0_PATH_NAME).
                          arg(d_ptr->exportFileName.at(i))))
        {
            isExist = true;
        }
        else
        {
            isExist = false;
        }

        // find the same name
        int exportFlag = 1;
        if (isExist)
        {
            MessageBox message(trs("Export"),
                               trs(QString("%1\r\n%2?").
                                   arg(d_ptr->exportFileName.at(i)).
                                   arg(trs("IfSelectTheSameNameFile"))),
                               QStringList() << trs("NotRepeated")
                               << trs("Repeated"));
            exportFlag = message.exec();
        }

        // cover
        bool isCopyOk = true;
        if (exportFlag)
        {
            QString fileName = QString("%1%2").
                               arg(CONFIG_DIR).
                               arg(d_ptr->exportFileName.at(i));
            QString newFileName = QString("%1/%2").
                                  arg(USB0_PATH_NAME).
                                  arg(d_ptr->exportFileName.at(i));
            QFile::remove(newFileName);
            isCopyOk = QFile::copy(fileName, newFileName);
        }
        if (!isCopyOk)
        {
            MessageBox message(trs("Export"),
                               trs(QString("%1\r\n%2").
                                   arg(d_ptr->exportFileName.at(i)).
                                   arg(trs("ExportFailed"))),
                               QStringList() << trs("Yes"));
            message.exec();
            return false;
        }
    }

    return true;
}
























