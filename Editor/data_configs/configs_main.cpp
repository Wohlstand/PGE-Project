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

#ifdef __linux__
#include <QProcessEnvironment>
#elif _WIN32
#include <QSysInfo>
#endif

#include <common_features/app_path.h>
#include <common_features/version_cmp.h>
#include <main_window/global_settings.h>
#include "../version.h"

#include "data_configs.h"

long ConfStatus::total_blocks=0;
long ConfStatus::total_bgo=0;
long ConfStatus::total_bg=0;
long ConfStatus::total_npc=0;

long ConfStatus::total_wtile=0;
long ConfStatus::total_wpath=0;
long ConfStatus::total_wscene=0;
long ConfStatus::total_wlvl=0;

long ConfStatus::total_music_lvl=0;
long ConfStatus::total_music_wld=0;
long ConfStatus::total_music_spc=0;
long ConfStatus::total_sound=0;

QString ConfStatus::configName="";
QString ConfStatus::configPath="";

QString ConfStatus::defaultTheme="";


dataconfigs::dataconfigs()
{
    default_grid=0;

    engine.screen_w=800;
    engine.screen_h=600;

    engine.wld_viewport_w=668;
    engine.wld_viewport_h=403;
}

dataconfigs::~dataconfigs()
{}

/*
[background-1]
name="Smallest bush"		;background name, default="background-%n"
type="scenery"			;Background type, default="Scenery"
grid=32				; 32 | 16 Default="32"
view=background			; background | foreground, default="background"
image="background-1.gif"	;Image file with level file ; the image mask will be have *m.gif name.
climbing=0			; default = 0
animated = 0			; default = 0 - no
frames = 1			; default = 1
frame-speed=125			; default = 125 ms, etc. 8 frames per sec
*/

void dataconfigs::addError(QString bug, QtMsgType level)
{
    WriteToLog(level, QString("LoadConfig -> ")+bug);
    errorsList<<bug;
}

void dataconfigs::setConfigPath(QString p)
{
    config_dir = p;
}

void dataconfigs::loadBasics()
{
    QString gui_ini = config_dir + "main.ini";
    QSettings guiset(gui_ini, QSettings::IniFormat);
    guiset.setIniCodec("UTF-8");

    int Animations=0;
    guiset.beginGroup("gui");
        splash_logo =               guiset.value("editor-splash", "").toString();
        #ifdef __linux__
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            QString envir = env.value("XDG_CURRENT_DESKTOP", "");
            qApp->setStyle("GTK");
            if(envir=="KDE" || envir=="XFCE")
            {
                ConfStatus::defaultTheme = "Breeze";
            } else {
                ConfStatus::defaultTheme = guiset.value("default-theme", "").toString();
            }
        #elif __APPLE__
            ConfStatus::defaultTheme = "Breeze";
        #elif _WIN32
            if(QSysInfo::WindowsVersion==QSysInfo::WV_WINDOWS10)
                ConfStatus::defaultTheme = "Breeze";
            else
                ConfStatus::defaultTheme = guiset.value("default-theme", "").toString();
        #else
            ConfStatus::defaultTheme =  guiset.value("default-theme", "").toString();
        #endif
        Animations     =            guiset.value("animations", 0).toInt();
    guiset.endGroup();

    guiset.beginGroup("main");
        data_dir = (guiset.value("application-dir", "0").toBool() ?
                        ApplicationPath + "/" : config_dir + "data/" );

        QString url     = guiset.value("home-page", "http://engine.wohlnet.ru/config_packs/").toString();
        QString version = guiset.value("pge-editor-version", "0.0").toString();
        bool ver_notify = guiset.value("enable-version-notify", true).toBool();
        if(ver_notify && (version != VersionCmp::compare(QString("%1").arg(_LATEST_STABLE), version)))
        {
            QMessageBox box;
            box.setWindowTitle( "Legacy configuration package" );
            box.setTextFormat(Qt::RichText);
            #if (QT_VERSION >= 0x050100)
            box.setTextInteractionFlags(Qt::TextBrowserInteraction);
            #endif
            box.setText(tr("You have a legacy configuration package.\n<br>"
                           "Editor will be started, but you may have a some problems with items or settings.\n<br>\n<br>"
                           "Please download and install latest version of a configuration package:\n<br>\n<br>Download: %1\n<br>"
                           "Note: most of config packs are updates togeter with PGE,<br>\n"
                           "therefore you can use same link to get updated version")
                                .arg("<a href=\"%1\">%1</a>").arg(url));
            box.setStandardButtons(QMessageBox::Ok);
            box.setIcon(QMessageBox::Warning);
            box.exec();
        }
    guiset.endGroup();

    if(!splash_logo .isEmpty())
    {
        splash_logo = data_dir + splash_logo;
        if(QPixmap(splash_logo).isNull())
        {
            WriteToLog(QtWarningMsg, QString("Wrong splash image: %1").arg(splash_logo));
            splash_logo = "";//Themes::Image(Themes::splash);
        }

        obj_splash_ani tempAni;

        animations.clear();
        for(;Animations>0;Animations--)
        {
            guiset.beginGroup(QString("splash-animation-%1").arg(Animations));
                QString img =   guiset.value("image", "").toString();
                    if(img.isEmpty()) goto skip;
                    tempAni.img = QPixmap(data_dir + img);
                    if(tempAni.img.isNull()) goto skip;
                tempAni.frames= guiset.value("frames", 1).toInt();
                tempAni.speed = guiset.value("speed", 128).toInt();
                tempAni.x =     guiset.value("x", 0).toInt();
                tempAni.y =     guiset.value("y", 0).toInt();

                animations.push_front(tempAni);
            skip:
            guiset.endGroup();
        }
    }
}

bool dataconfigs::loadconfigs()
{
    //unsigned long i;//, prgs=0;

    total_data=0;
    default_grid=0;
    errorsList.clear();

    //dirs
    if((!QDir(config_dir).exists())||(QFileInfo(config_dir).isFile()))
    {
        WriteToLog(QtCriticalMsg, QString("CONFIG DIR NOT FOUND AT: %1").arg(config_dir));
        return false;
    }

    emit progressPartsTotal(12);
    emit progressPartNumber(0);

    QString main_ini = config_dir + "main.ini";
    QSettings mainset(main_ini, QSettings::IniFormat);
    mainset.setIniCodec("UTF-8");

    QString customAppPath = ApplicationPath;

    mainset.beginGroup("main");
        customAppPath = mainset.value("application-path", ApplicationPath).toString();
        customAppPath.replace('\\', '/');
        data_dir = (mainset.value("application-dir", false).toBool() ?
                        customAppPath + "/" : config_dir + "data/" );

        ConfStatus::configName = mainset.value("config_name", QDir(config_dir).dirName()).toString();

        dirs.worlds = data_dir + mainset.value("worlds", "worlds").toString() + "/";

        dirs.music = data_dir +  mainset.value("music", "data/music").toString() + "/";
        dirs.sounds = data_dir + mainset.value("sound", "data/sound").toString() + "/";

        dirs.glevel = data_dir + mainset.value("graphics-level", "data/graphics/level").toString() + "/";
        dirs.gworld= data_dir +  mainset.value("graphics-worldmap", "data/graphics/worldmap").toString() + "/";
        dirs.gplayble = data_dir+mainset.value("graphics-characters", "data/graphics/characters").toString() + "/";

        dirs.gcustom = data_dir+ mainset.value("custom-data", "data-custom").toString() + "/";
    mainset.endGroup();

    ConfStatus::configPath = config_dir;

    mainset.beginGroup("graphics");
        default_grid = mainset.value("default-grid", 32).toInt();
    mainset.endGroup();

    if( mainset.status() != QSettings::NoError )
    {
        WriteToLog(QtCriticalMsg, QString("ERROR LOADING main.ini N:%1").arg(mainset.status()));
    }


    characters.clear();

    emit progressPartNumber(0);

    mainset.beginGroup("characters");
        int characters_q = mainset.value("characters", 0).toInt();
        for(int i=1; i<= characters_q; i++)
        {
            obj_playable_character pchar;
            pchar.id = i;
            pchar.name = mainset.value(QString("character%1-name").arg(i), QString("Character #%1").arg(i)).toString();
            characters.push_back(pchar);
        }
    mainset.endGroup();


    //Basic settings of engine
    QString engine_ini = config_dir + "engine.ini";
    if(QFile::exists(engine_ini)) //Load if exist, is not required
    {
        QSettings engineset(engine_ini, QSettings::IniFormat);
        engineset.setIniCodec("UTF-8");

        engineset.beginGroup("common");
        engine.screen_w = engineset.value("screen-width", engine.screen_w).toInt();
        engine.screen_h = engineset.value("screen-height", engine.screen_h).toInt();
        engineset.endGroup();

        engineset.beginGroup("world-map");
        engine.wld_viewport_w = engineset.value("viewport-width", engine.wld_viewport_w).toInt();
        engine.wld_viewport_h = engineset.value("viewport-height", engine.wld_viewport_h).toInt();
        engineset.endGroup();
    }


    ////////////////////////////////Preparing////////////////////////////////////////
    bgoPath =   dirs.glevel +  "background/";
    BGPath =    dirs.glevel +  "background2/";
    blockPath = dirs.glevel +  "block/";
    npcPath =   dirs.glevel +  "npc/";

    tilePath =  dirs.gworld +  "tile/";
    scenePath = dirs.gworld +  "scene/";
    pathPath =  dirs.gworld +  "path/";
    wlvlPath =  dirs.gworld +  "level/";

    //////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////Level items////////////////////////////////////////////
    loadLevelBackgrounds();
    loadLevelBGO();
    loadLevelBlocks();
    loadLevelNPC();
    ///////////////////////////////////////Level items////////////////////////////////////////////

    ///////////////////////////////////////World map items////////////////////////////////////////
    loadWorldTiles();
    loadWorldScene();
    loadWorldPaths();
    loadWorldLevels();
    ///////////////////////////////////////World map items////////////////////////////////////////


    //progress.setLabelText("Loading Music Data");
    ///////////////////////////////////////Music////////////////////////////////////////////
    loadMusic();
    ///////////////////////////////////////Music////////////////////////////////////////////

    ///////////////////////////////////////Sound////////////////////////////////////////////
    loadSound();
    ///////////////////////////////////////Sound////////////////////////////////////////////


    ///////////////////////////////////////Tilesets////////////////////////////////////////////
    loadTilesets();
    ///////////////////////////////////////Tilesets////////////////////////////////////////////

    ///////////////////////////////////////Rotation rules table////////////////////////////////////////////
    loadRotationTable();
    ///////////////////////////////////////Rotation rules table////////////////////////////////////////////

    emit progressMax(100);
    emit progressValue(100);
    emit progressTitle(QObject::tr("Finishing loading..."));

    /*if((!progress.wasCanceled())&&(!nobar))
        progress.close();*/

    WriteToLog(QtDebugMsg, QString("-------------------------"));
    WriteToLog(QtDebugMsg, QString("Config status 1"));
    WriteToLog(QtDebugMsg, QString("-------------------------"));
    WriteToLog(QtDebugMsg, QString("Loaded blocks          %1/%2").arg(main_block.stored()).arg(ConfStatus::total_blocks));
    WriteToLog(QtDebugMsg, QString("Loaded BGOs            %1/%2").arg(main_bgo.stored()).arg(ConfStatus::total_bgo));
    WriteToLog(QtDebugMsg, QString("Loaded NPCs            %1/%2").arg(main_npc.stored()).arg(ConfStatus::total_npc));
    WriteToLog(QtDebugMsg, QString("Loaded Backgrounds     %1/%2").arg(main_bg.stored()).arg(ConfStatus::total_bg));
    WriteToLog(QtDebugMsg, QString("Loaded Tiles           %1/%2").arg(main_wtiles.size()).arg(ConfStatus::total_wtile));
    WriteToLog(QtDebugMsg, QString("Loaded Sceneries       %1/%2").arg(main_wscene.size()).arg(ConfStatus::total_wscene));
    WriteToLog(QtDebugMsg, QString("Loaded Path images     %1/%2").arg(main_wpaths.size()).arg(ConfStatus::total_wpath));
    WriteToLog(QtDebugMsg, QString("Loaded Level images    %1/%2").arg(main_wlevels.size()).arg(ConfStatus::total_wlvl));
    WriteToLog(QtDebugMsg, QString("Loaded Level music     %1/%2").arg(main_music_lvl.size()).arg(ConfStatus::total_music_lvl));
    WriteToLog(QtDebugMsg, QString("Loaded Special music   %1/%2").arg(main_music_spc.size()).arg(ConfStatus::total_music_spc));
    WriteToLog(QtDebugMsg, QString("Loaded World music     %1/%2").arg(main_music_wld.size()).arg(ConfStatus::total_music_wld));
    WriteToLog(QtDebugMsg, QString("Loaded Sounds          %1/%2").arg(main_sound.size()).arg(ConfStatus::total_sound));

    return true;
}

bool dataconfigs::check()
{
    return
            (
    (main_bgo.stored()<=0)||
    (main_bg.stored()<=0)||
    (main_block.stored()<=0)||
    (main_npc.stored()<=0)||
    (main_wtiles.size()<=0)||
    (main_wscene.size()<=0)||
    (main_wpaths.size()<=0)||
    (main_wlevels.size()<=0)||
    (main_music_lvl.size()<=0)||
    (main_music_wld.size()<=0)||
    (main_music_spc.size()<=0)||
    (main_sound.size()<=0)
            );
}


