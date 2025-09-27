#pragma once

#include "CompileConfiguration.h"


#if CC_COMPILE_MODE_DEBUG

#include <QLabel>

#define DL_SHOW(str) {auto __label = new QLabel; __label->setText(str); __label->show();}
#define DL_SHOWNUM(num) {auto __label = new QLabel; __label->setText(QString::number(num)); __label->show();}
#define DL_SHOWPTR(ptr)  {auto __label = new QLabel; __label->setText(QString::number((long long)ptr)); __label->show();}

// DEV: DL_LOG

#else
#define DL_SHOW(str) 
#define DL_SHOWNUM(num) 

#endif


