#include "PlaybackControls.h"
#include <math.h>
#include <QFileDialog>
#include <QPainter>

PlaybackControls::PlaybackControls(MainWin *mainwindow)
{
	needsReset = false;
	mpParentWindow = mainwindow;
	mpGraph = NULL;
	mGraphValid = false;
	LoadSettingFile();
}

PlaybackControls::PlaybackControls(MainWin *mainwindow, QString file)
{
	needsReset = false;
	mpParentWindow = mainwindow;
	mFile = file;
	LoadSettingFile();
	InitialiseFilterGraph();	
}

PlaybackControls::~PlaybackControls()
{
	Stop();
	SaveSettingFile();
}

void PlaybackControls::LoadSettingFile()
{
	FILE *settingsFile = NULL;
	char path[256];
	char drive[10], dir[255], fname[255], ext[255];
	GetModuleFileNameA(NULL, path, 256);
	_splitpath(path, &drive[0], &dir[0], &fname[0], &ext[0]);
	sprintf_s(path, "%s%s\0", drive, dir);
	mPath = path;
	QString filename = mPath + SETTINGS_FILE;
	fopen_s(&settingsFile, filename.toStdString().c_str(), "r");
	if (settingsFile)
	{
		if (fscanf_s(settingsFile, "VOLUME=%d", &mVolume))
			mpParentWindow->volumeBar->setValue(mVolume); //$$better checks here
		fclose(settingsFile);
	}
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

HRESULT PlaybackControls::InitialiseFilterGraph()
{	
	mGraphValid = false;
	// Create the filter graph manager and query for interfaces.
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&mpGraph);
    if (FAILED(hr))
    {
		mpGraph = NULL;
		std::cout << "ERROR - Could not create the Filter Graph Manager." << std::endl;
    }
	return hr;
}

HRESULT PlaybackControls::LoadFile()
{
	QPixmap picture;
	picture = getPicture(mFile);
	if (!picture.isNull())
	{
		mpParentWindow->id3Art->setPixmap(picture.scaledToHeight(128));
	}

	HRESULT hr = 0;
	if (!mpGraph)
	{
		if (FAILED(hr = InitialiseFilterGraph()))
			return hr;
	}

	if (!mGraphValid)
	{
		hr = mpGraph->RenderFile(mFile.toStdWString().c_str(), NULL);
		if (SUCCEEDED(hr))
		{
			QString metadata = getArtist(mFile);
			mpParentWindow->artistLabel->setText("Artist : " + metadata);
			metadata = getAlbum(mFile);
			mpParentWindow->albumLabel->setText("Album : " + metadata);
			metadata = getTitle(mFile);
			mpParentWindow->titleLabel->setText("Title : " + metadata);
			mGraphValid = true;
			setVolume(mVolume);
			emit volumeChanged(mVolume);
		}
		return hr;
	}
	return S_OK;
}

void PlaybackControls::EmptyGraph()
{
	IEnumFilters *pEnum = NULL;
	if (mpGraph)
	{
		HRESULT hr = mpGraph->EnumFilters(&pEnum);
		if (SUCCEEDED(hr))
		{
			IBaseFilter *pFilter = NULL;
			while (S_OK == pEnum->Next(1, &pFilter, NULL))
			{
				mpGraph->RemoveFilter(pFilter);
				pEnum->Reset();
				pFilter->Release();
			}
			pEnum->Release();
		}
	}
	mGraphValid = false;
}

void PlaybackControls::Play()
{
	IMediaControl *pControl = NULL;
		
	if (SUCCEEDED(LoadFile()))
	{
		mpGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
		pControl->Run();
		pControl->Release();
	}
}

void PlaybackControls::PlayPause()
{
	IMediaControl *pControl = NULL;
	FILTER_STATE fs;
	HRESULT hr;
	
	if (mpGraph)
	{
		hr = mpGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
		pControl->GetState(50, (OAFilterState*)&fs);

		if (fs == State_Running)
			pControl->Pause();
		else
			pControl->Run();

		pControl->Release();
	}
}

void PlaybackControls::Stop()
{
	mpParentWindow->id3Art->clear();
	mpParentWindow->artistLabel->clear();
	mpParentWindow->albumLabel->clear();
	mpParentWindow->titleLabel->clear();
		
	IMediaControl *pControl = NULL;
	HRESULT hr;
	
	if (mpGraph)
	{
		hr = mpGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
		pControl->Stop();
		EmptyGraph();
		pControl->Release();
		mpGraph->Release();
		mpGraph = NULL;
	}
	mGraphValid = false;
	needsReset = true;
}

void PlaybackControls::setFilePath(QString file)
{
	Stop();
	mFile = file;
	emit pathChanged(mFile);
}

QString PlaybackControls::getFilePath()
{
	return mFile;
}

void PlaybackControls::changeFilePath()
{
	Stop();
	mFile = mpParentWindow->pathBox->displayText();
	Play();
	emit pathChanged(mFile);
}

void PlaybackControls::browseFilePath()
{
	QString tempFile = QFileDialog::getOpenFileName(mpParentWindow/*, tr("Open Music File"), "", tr("Media Files (*.mp3 *.avi *.wma)")*/);
	if (tempFile != "")
	{
		mFile = tempFile;
		Stop();
		Play();
		emit pathChanged(mFile);
	}
}

void PlaybackControls::setSeek()
{
	if (mpParentWindow->seekBox->displayText() != "")
	{
		int pos = mpParentWindow->seekBox->displayText().toInt();
		setSeek(pos);
	}
}

void PlaybackControls::setSeek(int value)
{
	IMediaPosition *pPosition;
	if (mpGraph)
	{
		HRESULT hr = mpGraph->QueryInterface(IID_IMediaPosition, (void **)&pPosition);
		if (SUCCEEDED(hr))
		{
			pPosition->put_CurrentPosition(value);
		}
		pPosition->Release();
	}
}

void PlaybackControls::getSeek()
{
	if (needsReset == true)
	{
		mpParentWindow->resize(351, 161);
		QSize size = mpParentWindow->size();
		int height = size.height();
		int width = size.width();
		if (width < 360)
		{
			needsReset = false;
		}
	}
	reset = true;
	REFTIME time;
	REFTIME duration;
	QString timeString;
	QString durationString;
	IMediaPosition *pPosition;
	if (mpGraph)
	{
		HRESULT hr = mpGraph->QueryInterface(IID_IMediaPosition, (void **)&pPosition);
		if (SUCCEEDED(hr))
		{
			pPosition->get_CurrentPosition(&time);
			pPosition->get_Duration(&duration);
			timeString.setNum(floor(time));
			durationString.setNum(floor(duration));
			QString output = timeString;
			output += " : ";
			output += durationString;
			mpParentWindow->seekLabel->setText(output);
			mpParentWindow->seekBar->setRange(0, duration);
			if (!mpParentWindow->seekBar->isSliderDown())
			{
				mpParentWindow->seekBar->setValue(time);
			}
		}
		pPosition->Release();
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
	IBasicAudio *pBasicAudio;
	mVolume = volume;
	if (mpGraph)
	{
		HRESULT hr = mpGraph->QueryInterface(IID_IBasicAudio, (void **)&pBasicAudio);
		if (SUCCEEDED(hr))
		{
			if (volume == -10000)
				volume = -25000;
			pBasicAudio->put_Volume(volume/2.5);
		}
		pBasicAudio->Release();
	}
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
