#include "ComboBoxManager.h"
#include <QLineEdit>
#include "debugutil-qt.h"
//============================================================
ComboBoxManager::ComboBoxManager(QComboBox *box, Preferences *prefs, QString prefKey, QString emptyValue, bool deleteEmptyValue):
  mComboBox(box), mPrefs(prefs), mEmptyValue(emptyValue), 
  mPrefKey(prefKey), mDeleteEmptyValue(deleteEmptyValue) {
  setUp(); 

 // these needs to be after setUp() to avoid recursion
  if (mComboBox->isEditable()) {
    connect(mComboBox->lineEdit(), SIGNAL(editingFinished()),
        this, SLOT(editFinished()));
  }
  connect(mComboBox, SIGNAL(currentIndexChanged(int)), 
          this, SLOT(itemChanged(int))); 

  return; 
}

//============================================================
ComboBoxManager::~ComboBoxManager(){
  return;
}


//============================================================
void ComboBoxManager::setUpNonEditable( ) {
  dbprintf(5, "setUpNonEditable()");
  if (mPrefs->hasKey((mPrefKey+"_initial").toStdString())) {
    dbprintf(5, QString("found key %1\n").arg(mPrefKey+"_initial")); 
    QString value = mPrefs->GetValue((mPrefKey+"_initial").toStdString()).c_str();
    int init = mComboBox->findText(value, Qt::MatchExactly); 
    if (init != -1) {
      dbprintf(5, QString("key exists in menu\n")); 
      mComboBox->setCurrentIndex(init); 
    } else {
      dbprintf(5, QString("key does not exist in menu, so ignoring\n")); 
    }
  }
  return; 
}
 
//============================================================
void ComboBoxManager::setUp(void ) {
  dbprintf(5, QString("setupComboBox(%1)\n").arg(mPrefKey)); 
  if (!mComboBox->isEditable()) {
    setUpNonEditable(); 
    return; 
  }
  QString data, initialItem;
  if (mPrefs->hasKey(mPrefKey.toStdString())) {
    data = mPrefs->GetValue(mPrefKey.toStdString()).c_str();
    dbprintf(5, QString("Prefs have key\n")); 
  } else {
    dbprintf(5, QString("Prefs no not have key\n")); 
    if (!mComboBox->count()) {
      if (mPrefs->hasKey((mPrefKey+"_initial").toStdString())) {
        data = mPrefs->GetValue((mPrefKey+"_initial").toStdString()).c_str();
     } else {
        data = "Click to edit";
      }
    }
  }
  dbprintf(5, QString("setupComboBox: data is \"%1\" and initialItem is \"%2\"\n").arg(data).arg(initialItem)); 
  
  QStringList items =
    data.split("<comboBoxItem>", QString::SkipEmptyParts); 
  
  //box->clear();
  int itemNum = 0; // skip the first, which is the boxName
  while (itemNum < items.size()) {
    QString item = items[itemNum].trimmed();
    dbprintf(5, QString("Parsing item \"%1\"...").arg(item)); 
    if (item != "" && mComboBox->findText(item, Qt::MatchExactly) == -1) {
      dbprintf(5, QString("adding item\n")); 
      mComboBox->addItem(item);     
    } else { 
      dbprintf(5, "skipping blank or redundant item\n"); 
    }
    ++itemNum; 
  }
  dbprintf(5, QString("Loaded %1 items to combo box \n").arg(itemNum));
  
  if (mPrefs->hasKey((mPrefKey+"_initial").toStdString())) {
    initialItem = mPrefs->GetValue((mPrefKey+"_initial").toStdString()).c_str();
  } else {
    initialItem = mComboBox->itemText(0);
  }
  dbprintf(5, QString("initialItem is now \"%1\"\n").arg(initialItem));
  if (initialItem != "") {
    int init = mComboBox->findText(initialItem, Qt::MatchExactly); 
    if (init != -1) {
      dbprintf(5, "initialItem was found\n"); 
      mComboBox->setCurrentIndex(init); 
    } else {
      dbprintf(5, "initialItem not found\n"); 
      mComboBox->addItem(initialItem); 
      mComboBox->setCurrentIndex(mComboBox->findText(initialItem, Qt::MatchExactly)); 
    }
  }
  dbprintf(5, "done with comboBox\n"); 
  return ; 
}

//============================================================
void ComboBoxManager::saveToPrefs(void) {
  QString value, text; 
  int i = 0; 
  while (i < mComboBox->count()) {
    text = mComboBox->itemText(i).trimmed(); 
    if (text != "") {
      value += QString("<comboBoxItem> %1 ").arg(text);
    }
    ++i; 
  }
  mPrefs->SetValue(mPrefKey.toStdString(), value.toStdString());
  mPrefs->SetValue((mPrefKey+"_initial").toStdString(), mComboBox->currentText().trimmed().toStdString()); 
  return; 
} 

//============================================================
void ComboBoxManager::itemChanged(int item) {
  dbprintf(5, QString("itemChanged, key is \"%1\", item is %2, current text is \"%3\"\n").arg(mPrefKey).arg(item).arg(mComboBox->currentText())); 
  if (mComboBox->isEditable()) {
    editFinished(); 
  }
  return; 
}

//============================================================
void ComboBoxManager::editFinished() {
  dbprintf(5, QString("editFinished, key is \"%1\", current text is \"%2\"\n").arg(mPrefKey).arg(mComboBox->currentText())); 
  QString text = mComboBox->currentText();
  if (text == "") {
    dbprintf(5, "empty text\n"); 
    if (mComboBox->currentIndex() != -1 && mDeleteEmptyValue) {
      dbprintf(5, "removing empty current item\n"); 
      mComboBox->removeItem(mComboBox->currentIndex());
    }
    if (mComboBox->count() == 0) {
      dbprintf(5, QString("Empty list, placing empty value \"%1\" in list\n").arg(mEmptyValue)); 
      mComboBox->addItem(mEmptyValue); 
      mComboBox->setCurrentIndex(0);
    }
    return;
  }
  if (text != "" &&  mComboBox->findText(text, Qt::MatchExactly) == -1) {
    dbprintf(5, QString("Found and adding text \"%1\"\n").arg(text)); 
    mComboBox->addItem(text);
  }
  if (mDeleteEmptyValue) {
    int bad = mComboBox->findText(mEmptyValue, Qt::MatchExactly);
    if (mComboBox->count() > 1 && bad != -1) {
      dbprintf(5, QString("Removing empty value item \"%1\" at index \"%2\"\n").arg(mEmptyValue).arg(mComboBox->itemText(bad))); 
      mComboBox->removeItem(bad);
    }
  }
  int setindex=mComboBox->findText(text, Qt::MatchExactly); 
  dbprintf(5, QString("setting index to %1 (%2)\n").arg(setindex).arg(text)); 
  mComboBox->setCurrentIndex(setindex); 
 
 return; 
}

