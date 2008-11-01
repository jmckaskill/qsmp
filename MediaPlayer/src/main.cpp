#include <iostream>
#include "LuaTcpConsole.h"
#include "PlaylistModel.h"
#include "PlaylistModel.inl"
#include "Player.h"
#include "PlaylistView.h"
#include "utilities.h"


int main(int argc, char **argv)
{
  using namespace qsmp;
  try
  {
    tbb::task_scheduler_init init;
    QApplication app(argc, argv);
    app.setApplicationName("SMPMediaPlayer");
    app.setApplicationVersion("0.0.1");
    app.setOrganizationName("Foobar NZ");
    app.setOrganizationDomain("foobar.co.nz");

    std::string path = (argc > 1) ? argv[1] : "";

    std::vector<Entry> paths;
    std::copy(recursive_directory_iterator(path),
              recursive_directory_iterator(),
              valueOutputFilterIterator<recursive_directory_iterator::value_type>(
                  testExtension(
                      equals(".mp3",boost::is_iequal())
                    ),
                  std::back_inserter(paths)));

    sort(paths,MetadataType_FileName,SortingOrder_Ascending);

    typedef boost::iterator_range<std::vector<Entry>::iterator> Range_t;
    typedef PlaylistModel<Range_t> Model_t;

    PlayerHistory history;
    Player player(bind(&PlayerHistory::PlayerNext,&history));
    history.SetNextCallback(bind(&chooseRandom<Range_t>,ref(paths)));
    history.SetPlayer(&player);

    Model_t model(paths);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    layout->addWidget(new PlaylistView(&model));
    layout->addWidget(new PlayerControl(&player,&history));

    QWidget window;
    window.setLayout(layout);
    window.show();

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
