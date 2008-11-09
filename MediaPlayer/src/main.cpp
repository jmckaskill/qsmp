/******************************************************************************
 * Copyright (C) 2008 James McKaskill <jmckaskill@gmail.com>                  *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "stdafx.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <qsmp/HotkeyWindow.h>
#include <qsmp/Log.h>
#include <qsmp/LuaTcpConsole.h>
#include <qsmp/PlaylistModel.h>
#include <qsmp/Player.h>
#include <qsmp/PlaylistView.h>
#include <qsmp/utilities.h>
#include <QtGui/qapplication.h>
#include <QtCore/qobject.h>
#include <QtGui/qboxlayout.h>
#include <string>
#include <tbb/task_scheduler_init.h>
#include <vector>
#include <windows.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  using namespace qsmp;
  using boost::bind;
  using boost::filesystem::recursive_directory_iterator;
  try
  {
    qInstallMsgHandler(&qsmp::QtMsgHandler);

    DWORD tick = ::GetTickCount();
    QSMP_LOG("Seed") << tick;
    srand(tick);

    tbb::task_scheduler_init init;
    QApplication app(argc, argv);
    app.setApplicationName("SMPMediaPlayer");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Foobar NZ");
    app.setOrganizationDomain("foobar.co.nz");

    std::string path = (argc > 1) ? argv[1] : "";

    std::vector<Media> paths;
    std::copy(recursive_directory_iterator(path),
              recursive_directory_iterator(),
              valueOutputFilterIterator<recursive_directory_iterator::value_type>(
                  testExtension(
                      equals(".mp3",boost::is_iequal())
                    ),
                  std::back_inserter(paths)));

    sort(paths,MetadataType_FileName,SortingOrder_Ascending);

    typedef boost::iterator_range<std::vector<Media>::iterator> Range_t;
    typedef PlaylistModel<Range_t> Model_t;
    typedef boost::iterator_range<PlayerHistory::const_history_iterator> HistoryRange_t;
    typedef PlaylistModel<HistoryRange_t> HistoryModel_t;

    PlayerHistory history;
    Player player(bind(&PlayerHistory::PlayerNext,&history));
    history.SetNextCallback(bind(&chooseRandom<Range_t>,boost::ref(paths)));
    history.SetPlayer(&player);

    //Model_t model(paths);
    boost::shared_ptr<PlaylistModelBase> model = NewPlaylist(bind(construct<HistoryRange_t>(),
                                                                  bind(&PlayerHistory::begin,&history,5),
                                                                  bind(&PlayerHistory::end,&history,15)));
    QObject::connect(&history, SIGNAL(OnHistoryUpdated()), model.get(), SLOT(Reset()));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    layout->addWidget(new PlaylistView(model.get()));
    layout->addWidget(new PlayerControl(&player,&history));

    HotkeyWindow window;
    window.setLayout(layout);
    window.show();

    window.RegisterHotkeys();

    QObject::connect(&window, SIGNAL(OnPrevious()), &history, SLOT(Previous()));
    QObject::connect(&window, SIGNAL(OnNext()), &history, SLOT(Next()));
    QObject::connect(&window, SIGNAL(OnPlayPause()), &player, SLOT(PlayPause()));
    QObject::connect(&window, SIGNAL(OnStop()), &player, SLOT(Stop()));

    //QObject::connect(view,SIGNAL(doubleClicked(QModelIndex)),model,SLOT(onDoubleClicked(QModelIndex)));
    //QObject::connect(model,SIGNAL(itemSelected(QString)),mywindow.control,SLOT(setFilePath(QString)));


    //LuaTcpServer lua;

    return app.exec();
  }
  catch(std::exception& e)
  {
    qFatal("Exception: %s",e.what());
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
