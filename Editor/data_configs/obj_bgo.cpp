/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014-2016 Vitaly Novichkov <admin@wohlnet.ru>
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

#include <QFile>

#include <main_window/global_settings.h>
#include <common_features/graphics_funcs.h>

#include "data_configs.h"

obj_bgo::obj_bgo()
{
    isValid     = false;
    animator_id = 0;
    cur_image   = nullptr;
}

void obj_bgo::copyTo(obj_bgo &bgo)
{
    /* for internal usage */
    bgo.isValid         = isValid;
    bgo.animator_id     = animator_id;
    bgo.cur_image       = cur_image;
    if(cur_image==nullptr)
        bgo.cur_image   = &image;

    bgo.setup = setup;
}

/*!
 * \brief Loads signe BGO configuration
 * \param sbgo Target BGO structure
 * \param section Name of INI-section where look for a BGO
 * \param merge_with Source of already loaded BGO structure to provide default settings per every BGO
 * \param iniFile INI-file where look for a BGO
 * \param setup loaded INI-file descriptor to load from global nested INI-file
 * \return true on success loading, false if error has occouped
 */
bool dataconfigs::loadLevelBGO(obj_bgo &sbgo, QString section, obj_bgo *merge_with, QString iniFile, QSettings *setup)
{
    bool valid=true;
    bool internal=!setup;
    QString errStr;
    if(internal)
    {
        setup=new QSettings(iniFile, QSettings::IniFormat);
        setup->setIniCodec("UTF-8");
    }

    if(!openSection(setup, section))
        return false;

    if(sbgo.setup.parse(setup, bgoPath, default_grid, merge_with ? &merge_with->setup : nullptr, &errStr))
    {
        sbgo.isValid = true;
    }
    else
    {
        addError(errStr);
        sbgo.isValid = false;
    }

    closeSection(setup);
    if(internal)
        delete setup;
    return valid;
}


void dataconfigs::loadLevelBGO()
{
    unsigned int i;

    obj_bgo sbgo;
    unsigned long bgo_total=0;
    bool useDirectory=false;

    QString bgo_ini = getFullIniPath("lvl_bgo.ini");
    if(bgo_ini.isEmpty())
        return;

    QString nestDir = "";

    QSettings setup(bgo_ini, QSettings::IniFormat);
    setup.setIniCodec("UTF-8");

    main_bgo.clear();   //Clear old

    if(!openSection( &setup, "background-main")) return;
        bgo_total = setup.value("total", 0).toUInt();
        total_data +=bgo_total;
        nestDir =   setup.value("config-dir", "").toString();
        if(!nestDir.isEmpty())
        {
            nestDir = config_dir + nestDir;
            useDirectory = true;
        }
    closeSection(&setup);

    emit progressPartNumber(1);
    emit progressMax(int(bgo_total));
    emit progressValue(0);
    emit progressTitle(QObject::tr("Loading BGOs..."));

    ConfStatus::total_bgo = long(bgo_total);

    main_bgo.allocateSlots(int(bgo_total));

    if(ConfStatus::total_bgo==0)
    {
        addError(QString("ERROR LOADING lvl_bgo.ini: number of items not define, or empty config"), PGE_LogLevel::Critical);
        return;
    }

    for(i=1; i<=bgo_total; i++)
    {
        emit progressValue(int(i));
        bool valid=false;

        if(useDirectory)
        {
            valid = loadLevelBGO(sbgo, "background", nullptr, QString("%1/background-%2.ini").arg(nestDir).arg(i));
        }
        else
        {
            valid = loadLevelBGO(sbgo, QString("background-%1").arg(i), 0, "", &setup);
        }

        /***************Load image*******************/
        if(valid)
        {
            QString errStr;
            GraphicsHelps::loadMaskedImage(bgoPath,
               sbgo.setup.image_n, sbgo.setup.mask_n,
               sbgo.image,
               errStr);

            if(!errStr.isEmpty())
            {
                valid=false;
                addError(QString("BGO-%1 %2").arg(i).arg(errStr));
            }
        }
        /***************Load image*end***************/
        sbgo.setup.id = i;
        main_bgo.storeElement(int(i), sbgo, valid);

        if( setup.status() != QSettings::NoError )
        {
            addError(QString("ERROR LOADING lvl_bgo.ini N:%1 (bgo-%2)").arg(setup.status()).arg(i), PGE_LogLevel::Critical);
        }
    }

    if(uint(main_bgo.stored()) < bgo_total)
    {
        addError(QString("Not all BGOs loaded! Total: %1, Loaded: %2").arg(bgo_total).arg(main_bgo.stored()));
    }
}
