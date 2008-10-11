#include <iostream>
#include "PlaybackControls.h"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	MainWin mywindow;
	mywindow.show();

	return app.exec();
}
