 #ifndef QSMP_HOTKEYWINDOW_H_
#define QSMP_HOTKEYWINDOW_H_

#include "common.h"

QSMP_BEGIN

class HotkeyWindow : public QWidget
{
  Q_OBJECT
public:
  HotkeyWindow();
  virtual ~HotkeyWindow();

  bool RegisterHotkeys();
  bool UnregisterHotkeys();

protected:
  bool winEvent(MSG* message, long* result);

Q_SIGNALS:
  void OnPrevious();
  void OnNext();
  void OnPlayPause();
  void OnStop();

private:
  bool registered_hotkeys_;
};

QSMP_END

#endif /*QSMP_HOTKEYWINDOW_H_*/