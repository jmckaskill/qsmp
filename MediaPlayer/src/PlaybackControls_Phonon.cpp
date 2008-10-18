#include "PlaybackControls.h"
#include <math.h>
#include <QFileDialog>

#include <phonon/backendcapabilities.h>

PlaybackControls::PlaybackControls(MainWin *mainwindow, QString file) : 
	mAudioOutput(Phonon::MusicCategory)
{
	mpParentWindow = mainwindow;
	mFile = file;
	mPath = QCoreApplication::applicationDirPath() + QDir::separator();
	mVolume = 100;
	needsReset = false;
	LoadSettingFile();
	mAudioOutputPath = Phonon::createPath(&mMediaObject, &mAudioOutput);
//	mVideoOutputPath = Phonon::createPath(&mMediaObject, &mVideoWidget);
}

PlaybackControls::~PlaybackControls()
{
	Stop();
	SaveSettingFile();
}

void PlaybackControls::LoadSettingFile()
{
	FILE *settingsFile = NULL;
	QString filename = mPath + SETTINGS_FILE;
	fopen_s(&settingsFile, filename.toStdString().c_str(), "r");
	if (settingsFile)
	{
		if (fscanf_s(settingsFile, "VOLUME=%d", &mVolume))
		{
			if ((mVolume > 100) || (mVolume < 0))
			{
				mVolume = 100;
			}
		}
		fclose(settingsFile);
	}
	mpParentWindow->volumeBar->setValue(mVolume); //$$better checks here
}

void PlaybackControls::SaveSettingFile()
{
	FILE *settingsFile = NULL;
	QString filename = mPath + SETTINGS_FILE;
	fopen_s(&settingsFile, filename.toStdString().c_str(), "w");
	if (settingsFile)
	{
		fprintf(settingsFile, "VOLUME=%d\n", mVolume);
		fclose(settingsFile);
	}
}

HRESULT PlaybackControls::LoadFile()
{
	mMediaObject.stop();
//	Phonon::State state = mMediaObject.state();
	QPixmap picture;
	picture = getPicture(mFile);
	if (!picture.isNull())
	{
		mpParentWindow->id3Art->setPixmap(picture.scaledToHeight(128));
	}
	QString metadata = getArtist(mFile);
	mpParentWindow->artistLabel->setText("Artist : " + metadata);
	metadata = getAlbum(mFile);
	mpParentWindow->albumLabel->setText("Album : " + metadata);
	metadata = getTitle(mFile);
	mpParentWindow->titleLabel->setText("Title : " + metadata);

	mMediaObject.setCurrentSource(mFile);
	QStringList list = mMediaObject.metaData(Phonon::TitleMetaData);
//	bool test = list.isEmpty();
	mAudioOutput.setVolume(mVolume / 100.0);
	emit volumeChanged(mVolume);
	return 0;
}

//void PlaybackControls::Play()
//{
//	Phonon::State state = mMediaObject.state();
//	switch (state)
//	{
//	case Phonon::PlayingState:
//		mMediaObject.pause();
//		break;
//	case Phonon::PausedState:
//		mMediaObject.play();
//		break;
//	default:
//		LoadFile();
//		mMediaObject.play();
//		break;
//	}
//}

void PlaybackControls::PlayPause()
{
	Phonon::State state = mMediaObject.state();
	switch (state)
	{
	case Phonon::PlayingState:
		mMediaObject.pause();
		break;
	case Phonon::PausedState:
		mMediaObject.play();
		break;
	default:
		LoadFile();
		mMediaObject.play();
//		mpParentWindow->layout()->addWidget(&mVideoWidget);
		break;
	}
}

void PlaybackControls::Stop()
{
	mMediaObject.stop();

	mpParentWindow->id3Art->clear();
	mpParentWindow->artistLabel->clear();
	mpParentWindow->albumLabel->clear();
	mpParentWindow->titleLabel->clear();
	needsReset = true;
}

void PlaybackControls::setFilePath(QString file)
{
	Stop();
	mFile = file;
  LoadFile();
  mMediaObject.play();
	emit pathChanged(mFile);
}

QString PlaybackControls::getFilePath()
{
	return mFile;
}

void PlaybackControls::editFilePath()
{
	QString tempFile = mpParentWindow->pathBox->displayText();
	if (tempFile != "")
	{
		Stop();
		mFile = tempFile;
		LoadFile();
		mMediaObject.play();
		emit pathChanged(mFile);
	}
}

void PlaybackControls::browseFilePath()
{
	QString tempFile = QFileDialog::getOpenFileName(mpParentWindow/*, tr("Open Music File"), "", tr("Media Files (*.mp3 *.avi *.wma)")*/);
	if (tempFile != "")
	{
		Stop();
		mFile = tempFile;
		LoadFile();
		mMediaObject.play();
		emit pathChanged(mFile);
	}
}

void PlaybackControls::setSeek()
{
	QString input = mpParentWindow->seekBox->displayText();
	int val = input.toInt();
	setSeek(val*1000);
}

void PlaybackControls::setSeek(int value)
{
	mMediaObject.seek(value);
}

void PlaybackControls::updateSeek()
{
	if (needsReset == true)
	{
		mpParentWindow->resize(363, 187);
		QSize size = mpParentWindow->size();
//		int height = size.height();
		int width = size.width();
		if (width < 370)
		{
			needsReset = false;
		}
	}
	
	qint64 timestamp = mMediaObject.currentTime();

	QString timeString, durationString;
	timeString.setNum((timestamp / 1000) / 60);
	QString output = timeString + ":";
	int seconds = (timestamp / 1000) % 60;
	timeString.setNum(seconds);
	if (seconds < 10)
	{
		timeString = "0" + timeString;
	}
	output += timeString;

	output += " / ";
	
	durationString.setNum((mMediaObject.totalTime() / 1000) / 60);
	output += durationString + ":";
	seconds = (mMediaObject.totalTime() / 1000) % 60;
	durationString.setNum(seconds);
	if (seconds < 10)
	{
		durationString = "0" + durationString;
	}
	output += durationString;

	mpParentWindow->seekLabel->setText(output);
	
	mpParentWindow->seekBar->setRange(0, mMediaObject.totalTime());
	if (!mpParentWindow->seekBar->isSliderDown())
	{
		mpParentWindow->seekBar->setValue(timestamp);
	}
}

void PlaybackControls::seekReturned()
{
	setSeek(mpParentWindow->seekBar->value());
}

void PlaybackControls::setVolume()
{
	int vol = mpParentWindow->volumeBar->value();
	setVolume(vol);
}

void PlaybackControls::setVolume(int volume)
{
	mVolume = volume;
	mAudioOutput.setVolume(volume/100.0);
}

void PlaybackControls::getVolume()
{
	mpParentWindow->volumeBar->setValue(mVolume);
}

QString PlaybackControls::getArtist(QString file)
{
	QString field;
	ID3_Tag mTag(file.toStdString().c_str());
	ID3_Frame* myFrame = mTag.Find(ID3FID_LEADARTIST);
	if (myFrame)
	{
		ID3_Field* myField = myFrame->GetField(ID3FN_TEXT);
		if (myField)
		{
			field = myField->GetRawText();
		}
	}
	return field;
}

QPixmap PlaybackControls::getPicture(QString file)
{
	QPixmap field;
	ID3_Tag mTag(file.toStdString().c_str());
	ID3_Frame* myFrame = mTag.Find(ID3FID_PICTURE);
	if (myFrame)
	{
		ID3_Field* myField = myFrame->GetField(ID3FN_DATA);
		if (myField)
		{
			const uchar *test = myField->GetRawBinary();
			field.loadFromData(test, myField->Size());
		}
	}
	return field;
}

QString PlaybackControls::getAlbum(QString file)
{
	QString field;
	ID3_Tag mTag(file.toStdString().c_str());
	ID3_Frame* myFrame = mTag.Find(ID3FID_ALBUM);
	if (myFrame)
	{
		ID3_Field* myField = myFrame->GetField(ID3FN_TEXT);
		if (myField)
		{
			field = myField->GetRawText();
		}
	}
	return field;
}

QString PlaybackControls::getTitle(QString file)
{	
	QString field;
	ID3_Tag mTag(file.toStdString().c_str());
	ID3_Frame* myFrame = mTag.Find(ID3FID_TITLE);
	if (myFrame)
	{
		ID3_Field* myField = myFrame->GetField(ID3FN_TEXT);
		if (myField)
		{
			field = myField->GetRawText();
		}
	}
	return field;
}

QString PlaybackControls::getComment(QString file)
{
	QString field;
	ID3_Tag mTag(file.toStdString().c_str());
	ID3_Frame* myFrame = mTag.Find(ID3FID_COMMENT);
	if (myFrame)
	{
		ID3_Field* myField = myFrame->GetField(ID3FN_TEXT);
		if (myField)
		{
			field = myField->GetRawText();
		}
	}
	return field;
}
