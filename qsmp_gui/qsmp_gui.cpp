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
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <ctime>
#include <iostream>
#include <qsmp_gui/Cache.h>
#include <qsmp_gui/CacheModel.h>
#include <qsmp_gui/HotkeyWindow.h>
#include <qsmp_gui/LuaTcpConsole.h>
#include <qsmp_gui/PlaylistModel.h>
#include <qsmp_gui/Player.h>
#include <qsmp_gui/PlaylistView.h>
#include <qsmp_gui/Process.h>
#include <qsmp_gui/utilities.h>
#include <qsmp_gui/ViewSelector.h>
#include <qsmp_lib/Log.h>
#include <QtCore/qobject.h>
#include <QtGui/qapplication.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qsplitter.h>
#include <string>
#include <vector>
#include <windows.h>


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void QsmpQtMsgHandler(QtMsgType type, const char* buf)
{
  using namespace qsmp;
  static LogContext context("Qt");
  switch(type)
  {
  case QtDebugMsg:
    LOG(context) << buf;
    break;
  case QtWarningMsg:
    WARNING(context) << buf;
    break;
  case QtCriticalMsg:
  case QtFatalMsg:
    FATAL(context) << buf;
    break;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  using namespace qsmp;
  using boost::bind;
  using boost::filesystem::recursive_directory_iterator;
  namespace io = boost::iostreams;
    qInstallMsgHandler(&QsmpQtMsgHandler);

    time_t current_time = std::time(NULL);
    LOG("Seed") << current_time;
    srand(current_time);

    QApplication app(argc, argv);
    app.setApplicationName("SMPMediaPlayer");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Foobar NZ");
    app.setOrganizationDomain("foobar.co.nz");

    Cache::lease();

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

    PlayerHistory history;
    Player player(bind(&PlayerHistory::GetPlayerNext,&history));
    history.SetNextCallback(bind(&chooseRandom<Range_t>,boost::ref(paths)));
    history.SetPlayer(&player);

    QSplitter* view_splitter = new QSplitter;
    QLayout*  view_layout = new QHBoxLayout;
    QWidget*  dummy_view_widget = new QWidget;
    QLayout*  dummy_view_layout = new QVBoxLayout;
    ViewTree* views = new ViewTree(dummy_view_layout);

    view_layout->addWidget(view_splitter);
    view_splitter->addWidget(views);
    view_splitter->addWidget(dummy_view_widget);
    dummy_view_widget->setLayout(dummy_view_layout);

    views->model()->AddNewView("History", new LayoutWidget<QVBoxLayout>(new HistoryView(&history),
                                                                        new PlayerControl(&player, &history)));
    views->model()->AddNewView("Cache", new CacheView("7a1e8b5b31087018f993cfd39e104d33344fe86b"));

    HotkeyWindow window;
    window.setLayout(view_layout);
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
