/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2015 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#undef main
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#undef main
#include <QFileInfo>
#include <QDir>

#include <common_features/logger.h>
#include <common_features/proxystyle.h>
#include <common_features/app_path.h>
#include <common_features/installer.h>
#include <common_features/themes.h>
#include <common_features/crashhandler.h>
#include <SingleApplication/singleapplication.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    CrashHandler::initCrashHandlers();

    QApplication::addLibraryPath( QFileInfo(argv[0]).dir().path() );
    QApplication *a = new QApplication(argc, argv);

    SingleApplication *as = new SingleApplication(argc, argv);
    if(!as->shouldContinue())
    {
        QTextStream(stdout) << "Editor already runned!\n";
        return 0;
    }

    a->setStyle(new PGE_ProxyStyle);

    QFont fnt = a->font();
    fnt.setPointSize(8);
    a->setFont(fnt);

    //Init system paths
    AppPathManager::initAppPath();

    foreach(QString arg, a->arguments())
    {
        if(arg=="--install")
        {
            AppPathManager::install();
            AppPathManager::initAppPath();

            Installer::moveFromAppToUser();
            Installer::associateFiles();

            QApplication::quit();
            QApplication::exit();
            delete a;
            delete as;
            return 0;
        }
    }

    //Init themes engine
    Themes::init();

    //Init log writer
    LoadLogSettings();

    WriteToLog(QtDebugMsg, "--> Application started <--");

    //Init SDL Audio subsystem
    if(SDL_Init(SDL_INIT_AUDIO)<0)
    {
        WriteToLog(QtWarningMsg, QString("Error of loading SDL: %1").arg(SDL_GetError()));
    }

    if(Mix_Init(MIX_INIT_FLAC|MIX_INIT_MOD|MIX_INIT_MP3|MIX_INIT_OGG)<0)
    {
        WriteToLog(QtWarningMsg, QString("Error of loading SDL Mixer: %1").arg(Mix_GetError()));
    }

    int ret=0;
    //Init Main Window class
    MainWindow *w = new MainWindow;
    if(!w->continueLoad)
    {
        delete w;
        goto QuitFromEditor;
    }

    a->connect( a, SIGNAL(lastWindowClosed()), a, SLOT( quit() ) );
    a->connect( w, SIGNAL( closeEditor()), a, SLOT( quit() ) );
    a->connect( w, SIGNAL( closeEditor()), a, SLOT( closeAllWindows() ) );

    w->show();
    w->setWindowState(w->windowState()|Qt::WindowActive);
    w->raise();
    QApplication::setActiveWindow(w);

    //Open files which opened by command line
    w->openFilesByArgs(a->arguments());

    //Set acception of external file openings
    w->connect(as, SIGNAL(openFile(QString)), w, SLOT(OpenFile(QString)));

#ifdef Q_OS_WIN
    w->initWindowsThumbnail();
#endif

    //Run main loop
    ret=a->exec();

QuitFromEditor:
    QApplication::quit();
    QApplication::exit();
    delete a;
    delete as;

    SDL_Quit();
    return ret;
}
