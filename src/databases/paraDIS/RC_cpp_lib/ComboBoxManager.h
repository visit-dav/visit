/*!
  A class which manages a Qt combo box, causing it to be able to save its contents to a file and to otherwise act intelligently.
  Obviously, this file needs Qt to compile properly.  As such, this is not part of the RC_cpp_lib collection. 
*/
#ifndef COMBOBOX_MANAGER_H
#define COMBOBOX_MANAGER_H

#include <QComboBox>
#include "Prefs.h"
//====================================================================
class ComboBoxManager: public QObject {
  Q_OBJECT
    public:
  ComboBoxManager(QComboBox *box, Preferences *prefs, QString prefKey, QString initialValue, bool deleteEmptyValue=false);
  ~ComboBoxManager();
  void setUp(void); 
  void setUpNonEditable(void );
  void saveToPrefs(void); 

  public slots:
  void itemChanged(int);
  void editFinished(); 
  
 private:
  QComboBox *mComboBox; 
  Preferences *mPrefs; 
  QString mEmptyValue, mPrefKey;
  bool  mDeleteEmptyValue; 
};  


#endif

