#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <qdialog.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QColorDialog>
#include <QMouseEvent>

class Label : public QLabel
{
    Q_OBJECT
public:
  explicit Label(QWidget *parent = 0);
  
protected:
  void mousePressEvent(QMouseEvent*);
  
signals:
  void clicked();
};

class ColorWidget : public QWidget
{
    Q_OBJECT
public:
  explicit ColorWidget(int index, QColor color, QWidget *parent = 0);
  QColor color();
  
protected:
  void setColor(QColor color);
  
private:
  QColor m_color;
  Label *m_label;
  
private slots:
  void getColor();
};

class ColorDialog : public QDialog
{
    Q_OBJECT
public:
  explicit ColorDialog(QList<QColor> colors, QWidget *parent = 0);
  
  QList<QColor> getListColor();
  
private:
  QVBoxLayout *colorLayout;
  
  QList<ColorWidget*> m_colorWidgets;
  QList<QColor> m_colors;
  
};

#endif // COLORDIALOG_H
