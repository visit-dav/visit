/*
** $RCSfile: debugutil-qt.cpp,v $
** $Name:  $
**
** ASCI Visualization Project 
**
** Lawrence Livermore National Laboratory
** Information Management and Graphics Group
** P.O. Box 808, Mail Stop L-561
** Livermore, CA 94551-0808
**
** For information about this project see:
**     http://www.llnl.gov/sccd/lc/img/
**
**      or contact: asciviz@llnl.gov
**
** For copyright and disclaimer information see:
**      $(ASCIVIS_ROOT)/copyright_notice_1.txt
**
**     or man llnl_copyright
**
** $Id: debugutil-qt.cpp,v 1.1 2009/04/02 00:39:23 rcook Exp $
**
*/
/*
**
**  Abstract:
**
**  Author:
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <stdarg.h>
#include "debugutil-qt.h"
#include <time.h>
#include <sys/time.h>
#include "QFile"
#include "QDataStream"
#include "QDir"
#include <sys/stat.h>

using namespace std; 

static  int    sVerbose = 0;
static    int    sCheck = 0;
static bool sUseDateString = false; 
static QString sPrependString; 
static bool sUseFilesAndLines = false; 

static QFile sDebugFile; 
static QDataStream sDebugFileStream;
static int sDebugFileVerbose = 0; 

static QTcpSocket *sDebugSocket = NULL; 
QDataStream sDebugSocketStream; 
static int sDebugSocketVerbose = 0; 

 
char *datestring(void) {
  static char timebuf[32]; 
  time_t now;

  now = time(NULL);
  strftime(timebuf, 31, "%d/%m/%y %H:%M:%S", (struct tm *)localtime(&now));
  return timebuf;
}

void real_dbfprintf(FILE *stream, int level, QString filename, int lineno, QString msg){
  if (sUseFilesAndLines) {
    msg = QString("[ %1: %2 ]: %3").arg(filename).arg(lineno).arg(msg);
  }
  if (sUseDateString) {
    msg = QString("(%1) ").arg(datestring()) + msg;
  }
  msg = sPrependString + msg; 
  if(sVerbose >= level){
    fprintf(stream, msg.toStdString().c_str()); 
  }    
  if (sDebugFileVerbose >= level && sDebugFile.isWritable()) {
    sDebugFileStream << msg.toAscii(); 
  } 
  if (sDebugSocketVerbose >= level && sDebugSocket && sDebugSocket->isWritable()) {
    sDebugSocketStream << msg; 
  }
  return;
  
}


static int check_verbose(void)
{
  if (!sCheck) {
    if (getenv("DEBUG_VERBOSE")) {
      sVerbose = atoi(getenv("DEBUG_VERBOSE"));
    }
    sCheck = 1;
  }
  return(sVerbose);
}

int dbg_isverbose(void)
{
  return(check_verbose());
}


void dbg_setverbose(int verbose)
{
  sVerbose = verbose;
  sCheck = 1;
}

void dbg_useDateString(bool use) {
  sUseDateString = use; 
}


void dbg_prependString(QString text) {
  sPrependString = text;
  return; 
}

void dbg_useFilesAndLines(bool use) {
  sUseFilesAndLines = use; 
}

// every message at or below the given level will go over the file
bool dbg_setlogfile(QString name, int level) {
  if (name.contains("/")) {
    // extract directory and create it:
    QString dirname = name.left(name.lastIndexOf('/')); 
    QDir dir(dirname); 
    if (!dir.exists()) {
      if (mkdir(dirname.toStdString().c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        return false;  
      }
    }
  }
  sDebugFileVerbose = level; 
  sDebugFile.setFileName(name); 
  sDebugFile.open(QIODevice::ReadWrite | QIODevice::Truncate); 
  if (!sDebugFile.isWritable()) {
    return false; 
  }
  sDebugFileStream.setDevice(&sDebugFile); 
  return true; 
}

// every message at or below the given level will go over the socket
void dbg_setSocket(QTcpSocket *socket, int level) {
  sDebugSocketVerbose = level; 
  sDebugSocket = socket; 
  sDebugSocketStream.setDevice(sDebugSocket); 
  return; 
}

