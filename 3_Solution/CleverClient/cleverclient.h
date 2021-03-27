#pragma once

#include <QtWidgets/QWidget>
#include "ui_cleverclient.h"

class CleverClient : public QWidget
{
    Q_OBJECT

public:
    CleverClient(QWidget *parent = Q_NULLPTR);

private:
    Ui::CleverClientClass ui;
};
