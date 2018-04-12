#pragma once

#include "IThread.h"
#include "ErrorLog.h"

#include "IConfig.h"
#include "SoftKeyManager.h"
#include "KeyActionManager.h"
#include "NormalModeKeyAction.h"
#include "WindowManager.h"
#include "WaveWidgetSelectMenu.h"
#include "MenuManager.h"
#include "PublicMenuManager.h"
#include "SupervisorMenuManager.h"
#include "KeyBoardPanel.h"
#include "NumberPanel.h"
#include "SetWidget.h"

#include "DataStorageDirManager.h"

#include "SoundManager.h"
#include "NetworkManager.h"
#include "MailManager.h"
#include "SystemTick.h"
#include "SystemMenu.h"
#include "SupervisorEntrance.h"
#include "SystemStatusBarWidget.h"
#include "SystemModeBarWidget.h"
#include "SystemManager.h"
#include "SystemBoardProvider.h"
#include "SystemAlarm.h"
#include "LightManager.h"
#include "BatteryMessage.h"
#include "BatteryAlarm.h"
#include "PowerManager.h"
#include "PromptInfoBarWidget.h"

#include "PatientManager.h"
#include "PatientInfoWidget.h"
#include "PatientBarWidget.h"
#include "PatientStatusBarWidget.h"

#include "TimeDate.h"
#include "TimeManager.h"
#include "DateTimeWidget.h"
#include "ElapseTimeWidget.h"

#include "WaveformCache.h"
#include "TrendCache.h"
#include "ParamDataStorageManager.h"
#include "TrendDataStorageManager.h"
#include "TrendDataWidget.h"
#include "TrendRescueRangePrint.h"
#include "RescueDataExportWidget.h"
#include "RescueDataDeleteWidget.h"

#include "SummaryStorageManager.h"
#include "SummaryRescueDataWidget.h"
#include "SummaryRescueRangePrint.h"

#include "EventStorageManager.h"

#include "PRT72Provider.h"
#include "PrintExec.h"
#include "PrintManager.h"
#include "PrintContinuousLayout.h"
#include "PrintTrendDataLayout.h"
#include "PrintTriggerSummaryLayout.h"
#include "PrintViewSummaryLayout.h"
#include "PrintOtherTriggerLayout.h"
#include "PrintSupervisorCfgLayout.h"
#include "PrintAlarm.h"

#include "Alarm.h"
#include "AlarmPhyInfoBarWidget.h"
#include "AlarmTechInfoBarWidget.h"
#include "AlarmMuteBarWidget.h"
#include "AlarmLimitMenu.h"
#include "AlarmStateMachine.h"

#include "ParamManager.h"
#include "DemoProvider.h"
#include "TN3Provider.h"
#include "SuntechProvider.h"
#include "BLMNIBPProvider.h"
#include "TS3Provider.h"
#include "S5Provider.h"
#include "BLMSPO2Provider.h"
#include "MasimoProvider.h"
#include "TT3Provider.h"
#include "T5Provider.h"
#include "BLMCO2Provider.h"
#include "TE3Provider.h"

#include "NIBPParam.h"
#include "NIBPAlarm.h"
#include "NIBPTrendWidget.h"
#include "NIBPDataTrendWidget.h"
#include "WaveTrendWidget.h"
#include "NIBPMenu.h"

#include "CO2Param.h"
#include "CO2Alarm.h"
#include "CO2WaveWidget.h"
#include "CO2TrendWidget.h"
#include "CO2BRTrendWidget.h"
#include "CO2Menu.h"

#include "WitleafProvider.h"
#include "IBPAlarm.h"
#include "IBPParam.h"
#include "IBPWaveWidget.h"
#include "IBPTrendWidget.h"
#include "IBPMenu.h"

#include "COParam.h"
#include "COAlarm.h"
#include "COMenu.h"
#include "COTrendWidget.h"
#include "COParam.h"

#include "PhaseinProvider.h"
#include "AGAlarm.h"
#include "AGParam.h"
#include "AGWaveWidget.h"
#include "AGTrendWidget.h"
#include "AGMenu.h"

#include "SPO2Param.h"
#include "SPO2Alarm.h"
#include "SPO2WaveWidget.h"
#include "SPO2TrendWidget.h"
#include "SPO2Menu.h"
#include "ShortTrendManager.h"

#include "TEMPParam.h"
#include "TEMPAlarm.h"
#include "TEMPTrendWidget.h"

#include "RESPDupParam.h"
#include "RESPDupAlarm.h"
#include "RESPParam.h"
#include "RESPAlarm.h"
#include "RESPWaveWidget.h"
#include "RESPTrendWidget.h"
#include "RESPMenu.h"

#include "ECGDupAlarm.h"
#include "ECGDupParam.h"
#include "ECGParam.h"
#include "ECGAlarm.h"
#include "ECGWaveWidget.h"
#include "ECGTrendWidget.h"
#include "ECGPVCSTrendWidget.h"
#include "ECGSTTrendWidget.h"
#include "ECGMenu.h"

#include "E5Provider.h"

#include "WiFiProfileMenu.h"
#include "EmailAddressLayout.h"

#include "RawDataCollection.h"
#include "RawDataCollectionTxt.h"

#include "SupervisorConfigManager.h"
#include "SupervisorGeneralSetMenu.h"
#include "SupervisorPortMenu.h"
#include "SupervisorNIBPMenu.h"
#include "SupervisorDisplayMenu.h"
#include "SupervisorPrintMenu.h"
#include "Supervisor12LMenu.h"
#include "SupervisorTimeMenu.h"
#include "SupervisorCodeMarker.h"
#include "SupervisorLocalMenu.h"
#include "SupervisorAlarmLimitMenu.h"
#include "SupervisorAlarmMenu.h"
#include "SupervisorWifiMenu.h"
#include "SupervisorMailMenu.h"

#include "USBManager.h"

#include "ActivityLogManager.h"

#include "CodeMarkerWidget.h"

#include "ECG12LDataAcquire.h"
#include "ECG12LDataMessNote.h"
#include "ECG12LSnapShotReview.h"
#include "ECG12LDataPrint.h"
#include "ECG12LDataStorage.h"
#include "ECG12LeadManager.h"
#include "PrintECG12LeadLayout.h"

#include "PMessageBox.h"

#include "OxyCRGWidget.h"
#include "OxyCRGHRWidget.h"
#include "OxyCRGRESPWidget.h"
#include "OxyCRGRRWidget.h"
#include "OxyCRGSPO2Widget.h"

// 构造所有的对象。
extern void newObjects(void);

// 删除对象。
extern void deleteObjects(void);
