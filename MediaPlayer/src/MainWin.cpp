#include "MainWin.h"

MainWin::MainWin()
{
	setupUi(this);
	id3Art->clear();
	artistLabel->clear();
	albumLabel->clear();
	titleLabel->clear();
	resize(363, 187);

	WId id = internalWinId();
	SetMMShellHook(id);
	control = new PlaybackControls(this);

	timer = new QTimer;

	/*play*/QObject::connect(playButton, SIGNAL(clicked()), control, SLOT(PlayPause()));
	/*stop*/QObject::connect(stopButton, SIGNAL(clicked()), control, SLOT(Stop()));
	/*show path*/QObject::connect(control, SIGNAL(pathChanged(QString)), pathBox, SLOT(setText(QString)));
	/*change path*/QObject::connect(pathBox, SIGNAL(returnPressed()), control, SLOT(editFilePath()));	
	/*path dialog*/QObject::connect(changeButton, SIGNAL(clicked()), control, SLOT(browseFilePath()));
	
	QObject::connect(seekBox, SIGNAL(returnPressed()), control, SLOT(setSeek()));
	QObject::connect(seekBar, SIGNAL(sliderMoved(int)), control, SLOT(setSeek(int)));
	QObject::connect(timer, SIGNAL(timeout()), control, SLOT(updateSeek()));
	QObject::connect(volumeBar, SIGNAL(sliderMoved(int)), control, SLOT(setVolume(int)));
	QObject::connect(control, SIGNAL(volumeChanged(int)), volumeBar, SLOT(setValue(int)));
	
	timer->start(100);
}

MainWin::~MainWin()
{
	delete timer;
	delete control;
	UnSetMMShellHook(internalWinId());
}

bool MainWin::winEvent(MSG *message, long * /*result*/)
{
	if(message->message == WM_APPCOMMAND)
	{
	  switch(GET_APPCOMMAND_LPARAM(message->lParam))
	  {
	  case APPCOMMAND_MEDIA_NEXTTRACK:
		// do something, which skips the track
		  show();
		return 1;
	  case APPCOMMAND_MEDIA_PREVIOUSTRACK:
		// do something...
		hide();
		return 1;
	  case APPCOMMAND_MEDIA_STOP:
		// do something, which stops playing
		  //emit stop();
		  control->Stop();
		return 1;
	  case APPCOMMAND_MEDIA_PLAY_PAUSE:
		// toggle between play and pause
		  control->PlayPause();
		//emit playPause();
		return 1;
	  }
	}
	return 0;
}