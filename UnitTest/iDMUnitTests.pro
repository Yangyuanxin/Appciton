####################################################################################################
# 使用说明:
#  1. 此工程文件(iDMTests.pro)归集所有测试用例;
#  2. 源码树中的每个独立模块(例如XXX)都包括一个独立的测试用例目录,
#     以Test前缀命名(例如TestXXX), 包含独立的.pro文件(例如TestXXX.pro);
####################################################################################################

TEMPLATE = subdirs

SUBDIRS += \
    hemodymic
