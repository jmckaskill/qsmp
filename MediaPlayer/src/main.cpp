#include <iostream>
#include "PlaybackControls.h"
#include "LuaTcpConsole.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
  app.setApplicationName("SMPMediaPlayer");
  app.setApplicationVersion("0.0.1");
  app.setOrganizationName("Foobar NZ");
  app.setOrganizationDomain("foobar.co.nz");

	MainWin mywindow;
	mywindow.show();

  LuaTcpServer lua;

	return app.exec();
}
