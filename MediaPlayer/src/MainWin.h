#ifndef MAINWIN_H
#define MAINWIN_H

#include <QObject>
#include <QTimer>
#include <MMShellHook.h>

#include "ui_untitled.h"

#include "PlaybackControls.h"
class PlaybackControls;

class MainWin : public QMainWindow, public Ui_MainWindow
{
	Q_OBJECT
public:
	MainWin();
	virtual ~MainWin();
	bool winEvent(MSG *message, long *result);
	PlaybackControls* control;
	QTimer* timer;
//signals:
//	virtual void playPause();
//	virtual void stop();
};

#endif