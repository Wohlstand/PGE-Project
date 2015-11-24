#!/bin/bash
cd $PWD
echo "#===============================================================================================" > _paths.sh
echo "#=================PLEASE SET UP THE ABLSOLUTE PATHS TO QMAKE AND TO LRELEASE====================" >> _paths.sh
echo "#===============================================================================================" >> _paths.sh
#    QMAKE_SPEC - the shortname of the host mkspec that is resolved and stored in the QMAKESPEC variable during a host build
#echo "QMAKESPEC=/home/travis/Qt/Qt-5.4_static/mkspecs/linux-g++" >> _paths.sh
#/home/travis/Qt/Qt-5.4_static/bin/qmake -set QMAKESPEC /home/travis/Qt/Qt-5.4_static/mkspecs/linux-g++
#    QT_INSTALL_PREFIX - default prefix for all paths
#echo "QT_INSTALL_PREFIX=/home/travis/Qt/Qt-5.4_static/" >> _paths.sh
#/home/travis/Qt/Qt-5.4_static/bin/qmake -set QT_INSTALL_PREFIX /home/travis/Qt/Qt-5.4_static/
#    QT_INSTALL_DATA - location of general architecture-independent Qt data
#echo "QT_INSTALL_DATA=/home/travis/Qt/Qt-5.4_static/" >> _paths.sh
#/home/travis/Qt/Qt-5.4_static/bin/qmake -set QT_INSTALL_DATA /home/travis/Qt/Qt-5.4_static/
#    QT_INSTALL_BINS - location of Qt binaries (tools and applications)
#echo "QT_INSTALL_BINS=/home/travis/Qt/Qt-5.4_static/bin" >> _paths.sh
#/home/travis/Qt/Qt-5.4_static/bin/qmake -set QT_INSTALL_BINS /home/travis/Qt/Qt-5.4_static/bin
#echo "QT_HOST_BINS=/home/travis/Qt/Qt-5.4_static/bin" >> _paths.sh
#/home/travis/Qt/Qt-5.4_static/bin/qmake -set QT_HOST_BINS /home/travis/Qt/Qt-5.4_static/bin
#echo "QT_HOST_DATA=/home/travis/Qt/Qt-5.4_static" >> _paths.sh
#/home/travis/Qt/Qt-5.4_static/bin/qmake -set QT_HOST_DATA /home/travis/Qt/Qt-5.4_static
#    QT_INSTALL_CONFIGURATION - location for Qt settings. Not applicable on Windows
#echo "QT_INSTALL_CONFIGURATION=/home/travis/Qt/Qt-5.4_static/bin" >> _paths.sh
#cp $PWD/_Libs/qt.conf /home/travis/Qt/Qt-5.4_static/bin
#/home/travis/Qt/Qt-5.4_static/bin/qmake -query
#echo "export QMAKESPEC QT_INSTALL_CONFIGURATION QT_INSTALL_PREFIX" >> _paths.sh
echo "QMake=\"qmake\"; #" >> _paths.sh
echo "" >> _paths.sh
echo "LRelease=\"lrelease\"; #" >> _paths.sh
echo "#===============================================================================================" >> _paths.sh
echo "#===============================================================================================" >> _paths.sh
echo "#===============================================================================================" >> _paths.sh
echo "" >> _paths.sh
chmod u+x _paths.sh
