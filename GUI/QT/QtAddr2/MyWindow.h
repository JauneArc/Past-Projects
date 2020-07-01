/* 
 * File:   MyWindow.h
 * Author: ljc878
 *
 * Created on 26 August 2015, 4:56 PM
 */

#ifndef _MYWINDOW_H
#define	_MYWINDOW_H

#include "ui_MyWindow.h"

class MyWindow : public QDialog {
    Q_OBJECT
public:
    MyWindow();
    virtual ~MyWindow();
private:
    Ui::MyWindow widget;
};

#endif	/* _MYWINDOW_H */
