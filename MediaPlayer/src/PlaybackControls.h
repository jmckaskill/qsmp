#ifndef PLAYBACKCONTROLS_H
#define PLAYBACKCONTROLS_H

#include "common.h"

#define SETTINGS_FILE "settings.ini"

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif

#include "MainWin.h"
class MainWin;

class PlaybackControls : public QObject
{
	Q_OBJECT
public:
	PlaybackControls(MainWin *mainwindow, QString file = "");
	virtual ~PlaybackControls();

	virtual QString getFilePath();
public slots:
	virtual void PlayPause();
	virtual void Stop();
	virtual void editFilePath();
	virtual void browseFilePath();
	virtual void setFilePath(QString file);
	virtual void setSeek();
	virtual void setSeek(int value);
	virtual void updateSeek();
	virtual void seekReturned();
	virtual void setVolume();
	virtual void setVolume(int value);
	virtual void getVolume();
	virtual QString getArtist(QString file);
	virtual QPixmap getPicture(QString file);
	virtual QString getAlbum(QString file);
	virtual QString getTitle(QString file);
	virtual QString getComment(QString file);
signals:
	virtual void toResize();
	virtual void pathChanged(QString newFile);
	virtual void volumeChanged(int volume);
private:
	virtual void LoadSettingFile();
	virtual void SaveSettingFile();
	virtual HRESULT LoadFile();
	MainWin *mpParentWindow;
	bool needsReset;

	QString mFile;
	int mVolume;
	QString mPath;

	Phonon::MediaObject mMediaObject;
	Phonon::AudioOutput mAudioOutput;
	Phonon::Path mAudioOutputPath;
	Phonon::Path mVideoOutputPath;
	Phonon::VideoWidget mVideoWidget;
};

#endif