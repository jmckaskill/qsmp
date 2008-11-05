#include "common.h"
#include <iostream>
#include "LuaTcpConsole.h"
#include "PlaylistModel.h"
#include "PlaylistModel.inl"
#include "Player.h"
#include "PlaylistView.h"
#include "utilities.h"
#include "HotkeyWindow.h"


int main(int argc, char **argv)
{
  using namespace qsmp;
  try
  {
    srand(::GetTickCount());

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
    history.SetNextCallback(bind(&chooseRandom<Range_t>,ref(paths)));
    history.SetPlayer(&player);

    //Model_t model(paths);
    shared_ptr<PlaylistModelBase> model = NewPlaylist(bind(construct<HistoryRange_t>(),
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
