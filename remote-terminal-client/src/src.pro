# This application is under GNU GPLv3. Please read the COPYING.txt file for further terms and conditions of the license.
# Copyright © 2017 Matthew James 
# "Remote Terminal" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
# "Remote Terminal" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
# You should have received a copy of the GNU General Public License along with "Remote Terminal". If not, see http://www.gnu.org/licenses/.

#-------------------------------------------------
#
# Project created by QtCreator 2016-02-13T09:17:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RTC
TEMPLATE = app


SOURCES += main.cpp\
        frmmain.cpp \
    client.cpp \
    crypto.cpp \
    frmabout.cpp

HEADERS  += frmmain.h \
    client.h \
    crypto.h \
    frmabout.h

FORMS    += frmmain.ui \
    frmabout.ui
    
    
DESTDIR = ../bin
MOC_DIR = ../build/moc
RCC_DIR = ../build/rcc
UI_DIR = ../build/ui
unix:OBJECTS_DIR = ../build/o/unix
win32:OBJECTS_DIR = ../build/o/win32
macx:OBJECTS_DIR = ../build/o/mac

LIBS += -lcrypto++
